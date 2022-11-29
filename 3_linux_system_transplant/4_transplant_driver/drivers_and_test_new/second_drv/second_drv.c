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
static struct class *seconddrv_class;
static struct device *seconddrv_dev;

volatile unsigned long *gpfcon;
volatile unsigned long *gpfdat;

volatile unsigned long *gpgcon;
volatile unsigned long *gpgdat;

static int second_drv_open(struct inode *inode, struct file *file)
{
	/*配置GPF0, 2为输入引脚*/
	*gpfcon &= ~((0x3<<(0*2)) | (0x3<<(2*2)));
	/*配置GPG3, 11为输入引脚*/
	*gpgcon &= ~((0x3<<(3*2)) | (0x3<<(11*2)));

	return 0;
}

static ssize_t second_drv_read(struct file *file, const char __user *buf, size_t size, loff_t * ppos)
{
	unsigned char key_vals[4];
	int regval;
	unsigned long ret;

	if(size != sizeof(key_vals))//这里是判断什么�
		return -EINVAL;

	/* 读GPF0, 2 */
	regval = *gpfdat;
	key_vals[0] = (regval & (1<<0)) ? 1 : 0;//S2
	key_vals[1] = (regval & (1<<2)) ? 1 : 0;//S3

	/* 读GPG3, 11 */
	regval = *gpgdat;
	key_vals[2] = (regval & (1<<3)) ? 1 : 0;//S4
	key_vals[3] = (regval & (1<<11)) ? 1 : 0;//S5
	//从内核空间拷贝数据到用户空间
	ret = copy_to_user(buf, key_vals, sizeof(key_vals));
	if(ret)
		printk("copy_to_user fail.\n");
	
	return sizeof(key_vals);
}

/* 这个结构是字符设备驱动程序的核心
 * 当应用程序操作设备文件时所调用的open、read、write等函数，
 * 最终会调用这个结构中指定的对应函数
 */
static struct file_operations second_drv_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   second_drv_open,        
	.read	=	second_drv_read,	   
};

int major;
/*
 * 执行insmod命令时就会调用这个函数 
 */
static int second_drv_init(void)  
{
	//注册，告诉内核
	major = register_chrdev(0, "second_drv", &second_drv_fops);

	seconddrv_class = class_create(THIS_MODULE, "second_drv");
	seconddrv_dev = device_create(seconddrv_class, NULL, MKDEV(major, 0), NULL, "buttons");

	gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);
	gpfdat = gpfcon + 1;

	gpgcon = (volatile unsigned long *)ioremap(0x56000060, 16);
	gpgdat = gpgcon + 1;
	
	return 0;
}

/*
 * 执行rmmod命令时就会调用这个函数 
 */
static void second_drv_exit(void)
{
	unregister_chrdev(major, "second_drv");//卸载驱动
	device_destroy(seconddrv_class, MKDEV(major, 0));
	class_destroy(seconddrv_class);

	iounmap(gpfcon);
	iounmap(gpgcon);	
}

module_init(second_drv_init);
module_exit(second_drv_exit);
MODULE_LICENSE("GPL");

