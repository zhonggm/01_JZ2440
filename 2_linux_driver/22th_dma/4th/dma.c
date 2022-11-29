
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

static char *src;/* 源的虚拟地址 */
static u32 src_phys;/* 源的物理地址 */

static char *dst;/* 目的的虚拟地址 */
static u32 dst_phys;/* 目的的物理地址 */

#define  BUF_SIZE (512 * 1024)/* 512 KB*/

//为了让它自动地创建设备节点，还得创建一个类
static struct class *cls;

static volatile struct s3c_dma_regs *dma_regs;/* 防止优化 */

static DECLARE_WAIT_QUEUE_HEAD(dma_waitq);/* 定义一个队列用于休眠 */

/* 中断事件标志, 中断服务程序将它置1，ioctl将它清0 */
static volatile int ev_dma = 0;

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
			/* 拷贝完之后比较一下 */
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
			/* 把源，目的，长度告诉DMA */
			dma_regs->disrc  = src_phys;/* 源的物理地址 */
			dma_regs->disrcc = (0<<1) | (0<<0);/* 源在AHB总线(内存)上，源地址是变化的且是递增的 */
			dma_regs->didst  = dst_phys;/* 目的的物理地址 */
			dma_regs->didstc = (0<<2) | (0<<1) | (0<<0);/* 当TC为0时发生中断，目的在AHB总线(内存)上，目的地址是变化的且是递增的 */
			dma_regs->dcon   = (1<<29) | (0<<28) | (0<<23) | (0<<20) | (BUF_SIZE<<0);/* 使能中断，单个传输， 用软件触发， 以字节为单位传输，总传输大小 */

			/* 启动DMA */
			dma_regs->dmasktrig = (1<<1) | (1<<0);/* 启动DMA，软件触发 */

			/* 如何知道DMA什么时候完成? 用中断。
			 * 传输完有一个中断，所以要注册一个中断。
			 */
			//启动后进入休眠
			wait_event_interruptible(dma_waitq, ev_dma); 

			/* 拷贝完之后比较一下 */
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
	/* 当DMA完成之后唤醒  */
	ev_dma = 1;
	wake_up_interruptible(&dma_waitq);/* 唤醒休眠的进程 */
	
	return IRQ_HANDLED;
}


static int s3c_dma_init(void)
{
	if(request_irq(IRQ_DMA3, s3c_dma_irq, 0, "s3c_dma", 1)){/* 如果返回不等于0的话表示失败 */
		printk("can't request_irq for DMA\n");
		return -EBUSY;
	}

	
	/************************************
	分配src, dst对应的缓冲区， 
	不能用kmalloc，因为用它分配的内存，其物理地址不一定是连续的，而使用DMA处理的内存必须是连续的。
	***************************************/
	src = dma_alloc_writecombine(NULL, BUF_SIZE, &src_phys, GFP_KERNEL);
	if(src == NULL){

		
		printk("can't alloc buffer for src\n");
		free_irq(IRQ_DMA3, 1);
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


