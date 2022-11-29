
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

static char *src;/* 源的虚拟地址 */
static u32 src_phys;/* 源的物理地址 */

static char *dst;/* 目的的虚拟地址 */
static u32 dst_phys;/* 目的的物理地址 */

#define  BUF_SIZE (512 * 1024)/* 512 KB*/

//为了让它自动地创建设备节点，还得创建一个类
static struct class *cls;

static struct s3c_dma_regs *dma_regs;

int s3c_dma_ioctl (struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	int i;

	//把源地址和目的地址内存数据分别设置为0xAA和0x55。
	memset(src, 0xAA, BUF_SIZE); 
	memset(dst, 0x55, BUF_SIZE);

	
	switch(cmd)
	{
		case MEM_CPY_NO_DMA:
		{
			for(i = 0; i < BUF_SIZE; i++){
				dst[i] = src[i];
			}
			/* 拷贝完之后比较下 */
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
	分配src, dst对应的缓冲区， 
	不能用kmalloc，因为用它分配的内存，其物理地址不一定是连续的，而使用DMA处理的内存必须是连续的。
	***************************************/
	src = dma_alloc_writecombine(NULL, BUF_SIZE, &src_phys, GFP_KERNEL);
	if(src == NULL){
		printk("can't alloc buffer for src\n");
		return -ENOMEM;
	}
	
	dst = dma_alloc_writecombine(NULL, BUF_SIZE, &dst_phys, GFP_KERNEL);
	if(dst == NULL){
		/* 如果分配了源成功，但分配目的不成功就释放掉源，以免造成内存泄露(内存分配没有释放并且又没人用它) */
		dma_free_writecombine(NULL, BUF_SIZE, src, src_phys);
		printk("can't alloc buffer for dst\n");
		return -ENOMEM;
	}
	//都分配成功后再注册字符设备
	major = register_chrdev(0, "s3c_dma", &dma_fops);

	/*为了自动创建设备节点*/
	cls = class_create(THIS_MODULE, "s3c_dma");
	//在这个类下面创建一个设备,这样mdev就会根据这些信息创建设备节点/dev/dma
	class_device_create(cls, NULL, MKDEV(major, 0), NULL, "dma");

	/*
	1. 先映射第0个通道，不行再换。

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














