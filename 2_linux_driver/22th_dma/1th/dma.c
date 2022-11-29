
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

int s3c_dma_ioctl (struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	switch(cmd)
	{
		case MEM_CPY_NO_DMA:
			
			break;
		case MEM_CPY_DMA:
			
			break;
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














