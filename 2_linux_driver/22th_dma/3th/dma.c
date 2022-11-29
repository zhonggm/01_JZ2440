
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>
#include <linux/poll.h>

#define DMA0_BASE_ADDR  0x4B000000
#define DMA1_BASE_ADDR  0x4B000040
#define DMA2_BASE_ADDR  0x4B000080
#define DMA3_BASE_ADDR  0x4B0000C0

struct s3c_dma_regs {
	unsigned long disrc;
	unsigned long disrcc;
	unsigned long didst;
	unsigned long didstc;
	unsigned long dcon;
	unsigned long dstat;
	unsigned long dcsrc;
	unsigned long dcdst;
	unsigned long dmasktrig;
};


static int major = 0;

#define MEM_CPY_NO_DMA  0
#define MEM_CPY_DMA     1

static char *src;/* Դ�������ַ */
static u32 src_phys;/* Դ�������ַ */

static char *dst;/* Ŀ�ĵ������ַ */
static u32 dst_phys;/* Ŀ�ĵ������ַ */

#define  BUF_SIZE (512 * 1024)/* 512 KB*/

//Ϊ�������Զ��ش����豸�ڵ㣬���ô���һ����
static struct class *cls;

static struct s3c_dma_regs *dma_regs;

int s3c_dma_ioctl (struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	int i;

	//��Դ��ַ��Ŀ�ĵ�ַ�ڴ����ݷֱ�����Ϊ0xAA��0x55��
	memset(src, 0xAA, BUF_SIZE); 
	memset(dst, 0x55, BUF_SIZE);

	
	switch(cmd)
	{
		case MEM_CPY_NO_DMA:
		{
			for(i = 0; i < BUF_SIZE; i++){
				dst[i] = src[i];
			}
			/* ������֮��Ƚ��� */
			if(memcmp(src, dst, BUF_SIZE) == 0){
				printk("MEM_CPY_NO_DMA OK\n");				
			}else{
				printk("MEM_CPY_NO_DMA ERROR\n");				
			}
			break;
		}
		case MEM_CPY_DMA:
		{
			
			break;
		}
	}

	return 0;
}

static struct file_operations dma_fops = {
	.owner = THIS_MODULE,
	.ioctl = s3c_dma_ioctl,
	
}

static int s3c_dma_init(void)
{
	/************************************
	����src, dst��Ӧ�Ļ������� 
	������kmalloc����Ϊ����������ڴ棬�������ַ��һ���������ģ���ʹ��DMA������ڴ�����������ġ�
	***************************************/
	src = dma_alloc_writecombine(NULL, BUF_SIZE, &src_phys, GFP_KERNEL);
	if(src == NULL){
		printk("can't alloc buffer for src\n");
		return -ENOMEM;
	}
	
	dst = dma_alloc_writecombine(NULL, BUF_SIZE, &dst_phys, GFP_KERNEL);
	if(dst == NULL){
		/* ���������Դ�ɹ���������Ŀ�Ĳ��ɹ����ͷŵ�Դ����������ڴ�й¶(�ڴ����û���ͷŲ�����û������) */
		dma_free_writecombine(NULL, BUF_SIZE, src, src_phys);
		printk("can't alloc buffer for dst\n");
		return -ENOMEM;
	}
	//������ɹ�����ע���ַ��豸
	major = register_chrdev(0, "s3c_dma", &dma_fops);

	/*Ϊ���Զ������豸�ڵ�*/
	cls = class_create(THIS_MODULE, "s3c_dma");
	//����������洴��һ���豸,����mdev�ͻ������Щ��Ϣ�����豸�ڵ�/dev/dma
	class_device_create(cls, NULL, MKDEV(major, 0), NULL, "dma");

	/*
	1. ��ӳ���0��ͨ���������ٻ���

	*/
	dma_regs = ioremap(DMA0_BASE_ADDR, sizeof(struct s3c_dma_regs));

	return 0;
}


static int s3c_dma_exit(void)
{
	class_device_destroy(cls, MKDEV(major, 0));
	class_destroy(cls);
	unregister_chrdev(major, "s3c_dma");
	dma_free_writecombine(NULL, BUF_SIZE, dst, dst_phys);
	dma_free_writecombine(NULL, BUF_SIZE, src, src_phys);
}

module_init(s3c_dma_init);
module_exit(s3c_dma_exit);
MODULE_LICENSE("GPL");














