#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/device.h>


//#include <asm/arch/regs-gpio.h>
//#include <asm/hardware.h>


static struct class *firstdrv_class;
static struct device	*firstdrv_dev;

//定义两个对应GPF的控制寄存和数据寄存器的虚拟地址
volatile unsigned long *gpfcon =NULL;
volatile unsigned long *gpfdat =NULL;

static int first_drv_open(struct inode *inode, struct file *file)
{
	//printk("first_drv_open.\n");
	/*配置GPF456引脚为输出*/
	*gpfcon &= ~((0x3 << (4*2)) | (0x3 << (5*2)) | (0x3 << (6*2))); 
	*gpfcon |=  ((0x1 << (4*2)) | (0x1 << (5*2)) | (0x1 << (6*2)));
	
	return 0;
}

static ssize_t first_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	//printk("first_drv_write.\n");
	int val;
	unsigned long ret;
	

	//从用户空间拷贝数据到内核空间
	ret = copy_from_user(&val, buf, count);
	if(ret)
		printk("copy_from_user fail.\n");
	
	if(val == 1)
		{
			//点灯，低电平有效
			*gpfdat &= ~((1<<4) | (1<<5) | (1<<6));
		}
	else
		{
			//灭灯
			*gpfdat |=  ((1<<4) | (1<<5) | (1<<6));
		}
	
	return 0;
}

/* 这个结构是字符设备驱动程序的核心
 * 当应用程序操作设备文件时所调用的open、read、write等函数，
 * 最终会调用这个结构中指定的对应函数
 */
static struct file_operations first_drv_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   first_drv_open,        
	.write	=	first_drv_write,	   
};

int major;
/*
 * 执行insmod命令时就会调用这个函数 
 */
static int first_drv_init(void)  
{
	//注册，告诉内核
	major = register_chrdev(0, "first_drv", &first_drv_fops);

	firstdrv_class = class_create(THIS_MODULE, "first_drv");
	firstdrv_dev = device_create(firstdrv_class, NULL, MKDEV(major, 0), NULL, "xyz");
	gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);
	gpfdat = gpfcon + 1;
	
	return 0;
}

/*
 * 执行rmmod命令时就会调用这个函数 
 */
static void first_drv_exit(void)
{
	unregister_chrdev(major, "first_drv");//卸载驱动
	device_destroy(firstdrv_class, MKDEV(major, 0));
	class_destroy(firstdrv_class);
	iounmap(gpfcon);//解除映射
}

module_init(first_drv_init);
module_exit(first_drv_exit);
MODULE_LICENSE("GPL");

