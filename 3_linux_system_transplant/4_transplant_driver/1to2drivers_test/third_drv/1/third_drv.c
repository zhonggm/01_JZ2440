#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>
#include <linux/irqreturn.h>
#include <linux/irq.h>

static struct class *thirddrv_class;
static struct class_device	*thirddrv_class_dev;

volatile unsigned long *gpfcon;
volatile unsigned long *gpfdat;

volatile unsigned long *gpgcon;
volatile unsigned long *gpgdat;


static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	printk("irq = %d\n", irq);
	return IRQ_HANDLED;
}


static int third_drv_open(struct inode *inode, struct file *file)
{
	/*配置GPF0, 2为输入引脚*/ 
	/*配置GPG3, 11为输入引脚*/
	//外部中断0/2/11/19
	//       S2/3/ 4/5
//	request_irq(unsigned int irq,irq_handler_t handler,unsigned long flags,const char *devname,void *dev_id);
	request_irq(IRQ_EINT0, buttons_irq, IRQT_BOTHEDGE, "S2", 1);
		request_irq(IRQ_EINT2, buttons_irq, IRQT_BOTHEDGE, "S3", 1);
			request_irq(IRQ_EINT11, buttons_irq, IRQT_BOTHEDGE, "S4", 1);
				request_irq(IRQ_EINT19, buttons_irq, IRQT_BOTHEDGE, "S5", 1);

	return 0;
}

static ssize_t third_drv_read(struct file *file, const char __user *buf, size_t size, loff_t * ppos)
{
	unsigned char key_vals[4];
	int regval;

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

	copy_to_user(buf, key_vals, sizeof(key_vals));

	return sizeof(key_vals);
}

//释放中断
int third_drv_close(struct inode *inode, struct file *file)
{
	free_irq(IRQ_EINT0, 1);
		free_irq(IRQ_EINT2, 1);
			free_irq(IRQ_EINT11, 1);
				free_irq(IRQ_EINT19, 1);
	return 0;
}



/* 这个结构是字符设备驱动程序的核心
 * 当应用程序操作设备文件时所调用的open、read、write等函数，
 * 最终会调用这个结构中指定的对应函数
 */
static struct file_operations third_drv_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   third_drv_open,        
	.read	=	third_drv_read,	   
	.release =	third_drv_close,	
};

int major;
/*
 * 执行insmod命令时就会调用这个函数 
 */
static int third_drv_init(void)  
{
	//注册，告诉内核
	major = register_chrdev(0, "third_drv", &third_drv_fops);

	thirddrv_class = class_create(THIS_MODULE, "third_drv");
	thirddrv_class_dev = class_device_create(thirddrv_class, NULL, MKDEV(major, 0), NULL, "buttons");

	gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);
	gpfdat = gpfcon + 1;

	gpgcon = (volatile unsigned long *)ioremap(0x56000060, 16);
	gpgdat = gpgcon + 1;
	
	return 0;
}

/*
 * 执行rmmod命令时就会调用这个函数 
 */
static void third_drv_exit(void)
{
	unregister_chrdev(major, "third_drv");//卸载驱动
	class_device_unregister(thirddrv_class_dev);
	class_destroy(thirddrv_class);

	iounmap(gpfcon);
	iounmap(gpgcon);
		
	return 0;
}

module_init(third_drv_init);
module_exit(third_drv_exit);
MODULE_LICENSE("GPL");

