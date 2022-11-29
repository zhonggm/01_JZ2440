/*
参考 linux-2.6.22.6\arch\arm\mach-s3c2410\s3c2410.c
参考 linux-2.6.22.6\drivers\mtd\nand\at91_nand.c
*/
#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/clk.h>
	
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>
	
#include <asm/io.h>
	
//NAND flash 控制寄存器结构体类型定义
struct s3c_nand_regs {
	unsigned long nfconf  ;
	unsigned long nfcont  ;
	unsigned long nfcmd   ;
	unsigned long nfaddr  ;
	unsigned long nfdata  ;
	unsigned long nfeccd0 ;
	unsigned long nfeccd1 ;
	unsigned long nfeccd  ;
	unsigned long nfstat  ;
	unsigned long nfestat0;
	unsigned long nfestat1;
	unsigned long nfmecc0 ;
	unsigned long nfmecc1 ;
	unsigned long nfsecc  ;
	unsigned long nfsblk  ;
	unsigned long nfeblk  ;
};
 

//定义一个nand_chip结构体指针
static struct nand_chip *s3c_nand;

//定义一个mtd_info结构体指针
static struct mtd_info *s3c_mtd;

struct s3c_nand_regs *s3c_nand_regs;

static struct mtd_partition s3c_nand_parts[] = { 
	[0] = { 
		.name = "bootloader", //分区名字 
		.size = 0x00040000, //分区大小 
		.offset = 0, //分区偏移值 
	}, 
	[1] = { 
		.name = "params", 
		.offset = MTDPART_OFS_APPEND, //APPEND表示这个分区紧跟上一个分区 
		.size = 0x00020000, 
	}, 
	[2] = { 
		.name = "kernel", 
		.offset = MTDPART_OFS_APPEND, 
		.size = 0x00200000, 
	}, 
	[3] = { 
		.name = "root", 
		.offset = MTDPART_OFS_APPEND, 
		.size = MTDPART_SIZ_FULL, //剩下的所有空间大小 
	} 
};


/*因为默认的nand_select_chip当chipnr为0时什么都没做，而要把
  nand的控制寄存器的NFCONT的bit1设为0才能选中nand flash，所以
  我们重新定义下nand_select_chip函数。
*/
static void s3c2440_select_chip(struct mtd_info *mtd, int chipnr)
{
	if(chipnr == -1){
		/* 取消选中:NFCONT[1]设为1 */
		s3c_nand_regs->nfcont |= (1<<1);
	}else{
		/* 选    中:NFCONT[1]设为0 */
		s3c_nand_regs->nfcont &= ~(1<<1);
	}
}


static void s3c2440_cmd_ctrl(struct mtd_info *mtd, int dat, unsigned int ctrl)
{
	if(ctrl & NAND_CLE){
		/* 发命令 : NFCMMD = dat */
		s3c_nand_regs->nfcmd = dat;
		
	}else{
		/* 发地址 : NFADDR = dat */
		s3c_nand_regs->nfaddr = dat;
	}
}

//返回nand flash的当前状态
static int s3c2440_dev_ready(struct mtd_info *mtd)
{
	//"NFSTATE的bit0"
	return (s3c_nand_regs->nfstat & (1<<0));
}

//入口函数
static int s3c_nand_init(void)
{
	struct clk *clk;
	
	/* 1. 分配一个nand_ship结构体 */
	s3c_nand = kzalloc(sizeof(struct nand_chip), GFP_KERNEL);
	
	//地址映射
	s3c_nand_regs = ioremap(0x4E000000, sizeof(struct s3c_nand_regs));
	
	/* 2. 设置nand_chip */
	/* 设置nand_chip是给nand_scan函数使用的，如果不知道怎么设置，先看nand_scan怎么使用 
	 * 它应该提供: 选中，发命令，发地址，发数据，读数据，判断状态的功能
	 */
	s3c_nand->select_chip = s3c2440_select_chip;//默认函数不适用，用我们自定义的
	s3c_nand->cmd_ctrl    = s3c2440_cmd_ctrl;
	s3c_nand->IO_ADDR_R   = &s3c_nand_regs->nfdata;//"NFDATA的虚拟地址";
	s3c_nand->IO_ADDR_W   = &s3c_nand_regs->nfdata;//"NFDATA的虚拟地址";
	s3c_nand->dev_ready   = s3c2440_dev_ready;
	s3c_nand->ecc.mode = NAND_ECC_SOFT;	/* enable ECC */

	/* 3. 硬件相关的设置: 根据NAND FLASH的手册设置时间参数 */
	/* 在这些硬件设置前需要使能NAND FLASH控制器的时钟 */
	clk = clk_get(NULL, "nand");
	clk_enable(clk);              /* CLKCON'bit[4] */
	
	/* HCLK=100MHz
	 * TACLS:  发出CLE/ALE之后多长时间才发出nWE信号, 从NAND手册可知CLE/ALE与nWE可以同时发出,所以TACLS=0
	 * TWRPH0: nWE的脉冲宽度, fHCLK *( TWRPH0 + 1 ), 从NAND手册可知它要>=12ns, 所以TWRPH0 >=1 
	 * TWRPH1: nWE变为高电平后多长时间CLE/ALE才能变为低电平, 从NAND手册可知它要(TWRPH1+1)*fHCLK>=5ns, 所以TWRPH1>=0
	 */
	#define TACLS    0
	#define TWRPH0   1
	#define TWRPH1   0
	s3c_nand_regs->nfconf = (TACLS<<12) | (TWRPH0<<8) | (TWRPH1<<4);

	/* NFCONT: 
	 * BIT1-设为1, 取消片选 
	 * BIT0-设为1, 使能NAND FLASH控制器
	 */
	s3c_nand_regs->nfcont = (1<<1) | (1<<0);
	
	/* 4. 使用: nand_scan */
	s3c_mtd = kzalloc(sizeof(struct mtd_info), GFP_KERNEL);//分配一个mtd_info结构体
	s3c_mtd->priv = s3c_nand;
	s3c_mtd->owner = THIS_MODULE;
	
	nand_scan(s3c_mtd, 1);/* 扫描识别NAND FLASH，构造mtd_info结构体，这个结构体里有读写擦除函数 */
	
	/* 5. add_mtd_partitions */
//	add_mtd_partitions(s3c_mtd, s3c_nand_parts, 4);//有4项
	mtd_device_register(s3c_mtd, s3c_nand_parts, 4);//有4项
	
	return 0;
}

//出口函数
static void s3c_nand_exit(void)
{
//	del_mtd_partitions(s3c_mtd);
	mtd_device_unregister(s3c_mtd);
	kfree(s3c_mtd);
	iounmap(s3c_nand_regs);
	kfree(s3c_nand);
}

module_init(s3c_nand_init);
module_exit(s3c_nand_exit);
MODULE_LICENSE("GPL");


