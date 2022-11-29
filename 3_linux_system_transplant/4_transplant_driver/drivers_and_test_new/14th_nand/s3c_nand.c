/*
�ο� linux-2.6.22.6\arch\arm\mach-s3c2410\s3c2410.c
�ο� linux-2.6.22.6\drivers\mtd\nand\at91_nand.c
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
	
//NAND flash ���ƼĴ����ṹ�����Ͷ���
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
 

//����һ��nand_chip�ṹ��ָ��
static struct nand_chip *s3c_nand;

//����һ��mtd_info�ṹ��ָ��
static struct mtd_info *s3c_mtd;

struct s3c_nand_regs *s3c_nand_regs;

static struct mtd_partition s3c_nand_parts[] = { 
	[0] = { 
		.name = "bootloader", //�������� 
		.size = 0x00040000, //������С 
		.offset = 0, //����ƫ��ֵ 
	}, 
	[1] = { 
		.name = "params", 
		.offset = MTDPART_OFS_APPEND, //APPEND��ʾ�������������һ������ 
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
		.size = MTDPART_SIZ_FULL, //ʣ�µ����пռ��С 
	} 
};


/*��ΪĬ�ϵ�nand_select_chip��chipnrΪ0ʱʲô��û������Ҫ��
  nand�Ŀ��ƼĴ�����NFCONT��bit1��Ϊ0����ѡ��nand flash������
  �������¶�����nand_select_chip������
*/
static void s3c2440_select_chip(struct mtd_info *mtd, int chipnr)
{
	if(chipnr == -1){
		/* ȡ��ѡ��:NFCONT[1]��Ϊ1 */
		s3c_nand_regs->nfcont |= (1<<1);
	}else{
		/* ѡ    ��:NFCONT[1]��Ϊ0 */
		s3c_nand_regs->nfcont &= ~(1<<1);
	}
}


static void s3c2440_cmd_ctrl(struct mtd_info *mtd, int dat, unsigned int ctrl)
{
	if(ctrl & NAND_CLE){
		/* ������ : NFCMMD = dat */
		s3c_nand_regs->nfcmd = dat;
		
	}else{
		/* ����ַ : NFADDR = dat */
		s3c_nand_regs->nfaddr = dat;
	}
}

//����nand flash�ĵ�ǰ״̬
static int s3c2440_dev_ready(struct mtd_info *mtd)
{
	//"NFSTATE��bit0"
	return (s3c_nand_regs->nfstat & (1<<0));
}

//��ں���
static int s3c_nand_init(void)
{
	struct clk *clk;
	
	/* 1. ����һ��nand_ship�ṹ�� */
	s3c_nand = kzalloc(sizeof(struct nand_chip), GFP_KERNEL);
	
	//��ַӳ��
	s3c_nand_regs = ioremap(0x4E000000, sizeof(struct s3c_nand_regs));
	
	/* 2. ����nand_chip */
	/* ����nand_chip�Ǹ�nand_scan����ʹ�õģ������֪����ô���ã��ȿ�nand_scan��ôʹ�� 
	 * ��Ӧ���ṩ: ѡ�У����������ַ�������ݣ������ݣ��ж�״̬�Ĺ���
	 */
	s3c_nand->select_chip = s3c2440_select_chip;//Ĭ�Ϻ��������ã��������Զ����
	s3c_nand->cmd_ctrl    = s3c2440_cmd_ctrl;
	s3c_nand->IO_ADDR_R   = &s3c_nand_regs->nfdata;//"NFDATA�������ַ";
	s3c_nand->IO_ADDR_W   = &s3c_nand_regs->nfdata;//"NFDATA�������ַ";
	s3c_nand->dev_ready   = s3c2440_dev_ready;
	s3c_nand->ecc.mode = NAND_ECC_SOFT;	/* enable ECC */

	/* 3. Ӳ����ص�����: ����NAND FLASH���ֲ�����ʱ����� */
	/* ����ЩӲ������ǰ��Ҫʹ��NAND FLASH��������ʱ�� */
	clk = clk_get(NULL, "nand");
	clk_enable(clk);              /* CLKCON'bit[4] */
	
	/* HCLK=100MHz
	 * TACLS:  ����CLE/ALE֮��೤ʱ��ŷ���nWE�ź�, ��NAND�ֲ��֪CLE/ALE��nWE����ͬʱ����,����TACLS=0
	 * TWRPH0: nWE��������, fHCLK *( TWRPH0 + 1 ), ��NAND�ֲ��֪��Ҫ>=12ns, ����TWRPH0 >=1 
	 * TWRPH1: nWE��Ϊ�ߵ�ƽ��೤ʱ��CLE/ALE���ܱ�Ϊ�͵�ƽ, ��NAND�ֲ��֪��Ҫ(TWRPH1+1)*fHCLK>=5ns, ����TWRPH1>=0
	 */
	#define TACLS    0
	#define TWRPH0   1
	#define TWRPH1   0
	s3c_nand_regs->nfconf = (TACLS<<12) | (TWRPH0<<8) | (TWRPH1<<4);

	/* NFCONT: 
	 * BIT1-��Ϊ1, ȡ��Ƭѡ 
	 * BIT0-��Ϊ1, ʹ��NAND FLASH������
	 */
	s3c_nand_regs->nfcont = (1<<1) | (1<<0);
	
	/* 4. ʹ��: nand_scan */
	s3c_mtd = kzalloc(sizeof(struct mtd_info), GFP_KERNEL);//����һ��mtd_info�ṹ��
	s3c_mtd->priv = s3c_nand;
	s3c_mtd->owner = THIS_MODULE;
	
	nand_scan(s3c_mtd, 1);/* ɨ��ʶ��NAND FLASH������mtd_info�ṹ�壬����ṹ�����ж�д�������� */
	
	/* 5. add_mtd_partitions */
//	add_mtd_partitions(s3c_mtd, s3c_nand_parts, 4);//��4��
	mtd_device_register(s3c_mtd, s3c_nand_parts, 4);//��4��
	
	return 0;
}

//���ں���
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


