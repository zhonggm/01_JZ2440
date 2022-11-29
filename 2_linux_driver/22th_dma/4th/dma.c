
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
#include <linux/dma-mapping.h>


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

static volatile struct s3c_dma_regs *dma_regs;/* ��ֹ�Ż� */

static DECLARE_WAIT_QUEUE_HEAD(dma_waitq);/* ����һ�������������� */

/* �ж��¼���־, �жϷ����������1��ioctl������0 */
static volatile int ev_dma = 0;

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
			/* ������֮��Ƚ�һ�� */
			if(memcmp(src, dst, BUF_SIZE) == 0){
				printk("MEM_CPY_NO_DMA OK\n");				
			}else{
				printk("MEM_CPY_NO_DMA ERROR\n");				
			}
			break;
		}
		case MEM_CPY_DMA:
		{
			ev_dma = 0;
			/* ��Դ��Ŀ�ģ����ȸ���DMA */
			dma_regs->disrc  = src_phys;/* Դ�������ַ */
			dma_regs->disrcc = (0<<1) | (0<<0);/* Դ��AHB����(�ڴ�)�ϣ�Դ��ַ�Ǳ仯�����ǵ����� */
			dma_regs->didst  = dst_phys;/* Ŀ�ĵ������ַ */
			dma_regs->didstc = (0<<2) | (0<<1) | (0<<0);/* ��TCΪ0ʱ�����жϣ�Ŀ����AHB����(�ڴ�)�ϣ�Ŀ�ĵ�ַ�Ǳ仯�����ǵ����� */
			dma_regs->dcon   = (1<<29) | (0<<28) | (0<<23) | (0<<20) | (BUF_SIZE<<0);/* ʹ���жϣ��������䣬 ����������� ���ֽ�Ϊ��λ���䣬�ܴ����С */

			/* ����DMA */
			dma_regs->dmasktrig = (1<<1) | (1<<0);/* ����DMA��������� */

			/* ���֪��DMAʲôʱ�����? ���жϡ�
			 * ��������һ���жϣ�����Ҫע��һ���жϡ�
			 */
			//�������������
			wait_event_interruptible(dma_waitq, ev_dma); 

			/* ������֮��Ƚ�һ�� */
			if(memcmp(src, dst, BUF_SIZE) == 0){
				printk("MEM_CPY_DMA OK\n");				
			}else{
				printk("MEM_CPY_DMA ERROR\n");				
			}
			
			break;
		}
	}

	return 0;
}

static struct file_operations dma_fops = {
	.owner = THIS_MODULE,
	.ioctl = s3c_dma_ioctl,
};

static irqreturn_t s3c_dma_irq(int irq, void *devid)
{
	/* ��DMA���֮����  */
	ev_dma = 1;
	wake_up_interruptible(&dma_waitq);/* �������ߵĽ��� */
	
	return IRQ_HANDLED;
}


static int s3c_dma_init(void)
{
	if(request_irq(IRQ_DMA3, s3c_dma_irq, 0, "s3c_dma", 1)){/* ������ز�����0�Ļ���ʾʧ�� */
		printk("can't request_irq for DMA\n");
		return -EBUSY;
	}

	
	/************************************
	����src, dst��Ӧ�Ļ������� 
	������kmalloc����Ϊ����������ڴ棬�������ַ��һ���������ģ���ʹ��DMA������ڴ�����������ġ�
	***************************************/
	src = dma_alloc_writecombine(NULL, BUF_SIZE, &src_phys, GFP_KERNEL);
	if(src == NULL){

		
		printk("can't alloc buffer for src\n");
		free_irq(IRQ_DMA3, 1);
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
	dma_regs = ioremap(DMA3_BASE_ADDR, sizeof(struct s3c_dma_regs));

	return 0;
}


static void s3c_dma_exit(void)
{
	iounmap(dma_regs);
	class_device_destroy(cls, MKDEV(major, 0));
	class_destroy(cls);
	unregister_chrdev(major, "s3c_dma");
	dma_free_writecombine(NULL, BUF_SIZE, dst, dst_phys);
	dma_free_writecombine(NULL, BUF_SIZE, src, src_phys);
	free_irq(IRQ_DMA3, 1);
}

module_init(s3c_dma_init);
module_exit(s3c_dma_exit);
MODULE_LICENSE("GPL");


