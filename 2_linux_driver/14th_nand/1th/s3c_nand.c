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
	
#include <asm/arch/regs-nand.h>
#include <asm/arch/nand.h>

//����һ��nand_chip�ṹ��ָ��
static struct nand_chip *s3c_nand;

//����һ��mtd_info�ṹ��ָ��
static struct mtd_info *s3c_mtd;

/*��ΪĬ�ϵ�nand_select_chip��chipnrΪ0ʱʲô��û������Ҫ��
  nand�Ŀ��ƼĴ�����NFCONT��bit1��Ϊ0����ѡ��nand flash������
  �������¶�����nand_select_chip������
*/
static void s3c2440_select_chip(struct mtd_info *mtd, int chipnr)
{
	if(chipnr == -1){
		/* ȡ��ѡ��:NFCONT[1]��Ϊ1 */
		
	}else{
		/* ѡ��:NFCONT[1]��Ϊ0 */
		
	}

}


static void s3c2440_cmd_ctrl(struct mtd_info *mtd, int dat, unsigned int ctrl)
{
	if(ctrl & NAND_CLE){
		/* ������ : NFCMMD = dat */
		
		
	}else{
		/* ����ַ : NFADDR = dat */

	}

}

static int s3c2440_dev_ready(struct mtd_info *mtd)
{
	return "NFSTATE��bit0";
}

//��ں���
static int s3c_nand_init(void)
{
	/* 1. ����һ��nand_ship�ṹ�� */
	s3c_nand = kzalloc(sizeof(struct nand_chip), GFP_KERNEL);


	/* 2. ����nand_chip */
	/* ����nand_chip�Ǹ�nand_scan����ʹ�õģ������֪����ô���ã��ȿ�nand_scan��ôʹ�� 
	 * ��Ӧ���ṩ: ѡ�У����������ַ�������ݣ������ݣ��ж�״̬�Ĺ���
	 */
	s3c_nand->select_chip = s3c2440_select_chip;//Ĭ�Ϻ��������ã��������Զ����
	s3c_nand->cmd_ctrl    = s3c2440_cmd_ctrl;
	s3c_nand->IO_ADDR_R   = "NFDATA�������ַ";
	s3c_nand->IO_ADDR_W   = "NFDATA�������ַ";
	s3c_nand->dev_ready   = s3c2440_dev_ready;

	/* 3. Ӳ����ص����� */



	/* 4. ʹ��: nand_scan */
	s3c_mtd = kzalloc(sizeof(struct mtd_info), GFP_KERNEL);//����һ��mtd_info�ṹ��
	s3c_mtd->priv = s3c_nand;
	s3c_mtd->owner = THIS_MODULE;

	nand_scan(s3c_mtd, 1);/* ɨ��ʶ��NAND FLASH������mtd_info�ṹ�壬����ṹ�����ж�д�������� */


	
	/* 5. add_mtd_partitions */



	return 0;
}

//���ں���
static void s3c_nand_exit(void)
{

}


module_init(s3c_nand_init);
module_exit(s3c_nand_exit);
MODULE_LICENSE("GPL");


