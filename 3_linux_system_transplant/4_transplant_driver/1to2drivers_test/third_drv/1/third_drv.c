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
	/*≈‰÷√GPF0, 2Œ™ ‰»Î“˝Ω≈*/ 
	/*≈‰÷√GPG3, 11Œ™ ‰»Î“˝Ω≈*/
	//Õ‚≤ø÷–∂œ0/2/11/19
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

	if(size != sizeof(key_vals))//’‚¿Ô «≈–∂œ ≤√¥Â
		return -EINVAL;

	/* ∂¡GPF0, 2 */
	regval = *gpfdat;
	key_vals[0] = (regval & (1<<0)) ? 1 : 0;//S2
	key_vals[1] = (regval & (1<<2)) ? 1 : 0;//S3

	/* ∂¡GPG3, 11 */
	regval = *gpgdat;
	key_vals[2] = (regval & (1<<3)) ? 1 : 0;//S4
	key_vals[3] = (regval & (1<<11)) ? 1 : 0;//S5

	copy_to_user(buf, key_vals, sizeof(key_vals));

	return sizeof(key_vals);
}

// Õ∑≈÷–∂œ
int third_drv_close(struct inode *inode, struct file *file)
{
	free_irq(IRQ_EINT0, 1);
		free_irq(IRQ_EINT2, 1);
			free_irq(IRQ_EINT11, 1);
				free_irq(IRQ_EINT19, 1);
	return 0;
}



/* ’‚∏ˆΩ·ππ «◊÷∑˚…Ë±∏«˝∂Ø≥Ã–Úµƒ∫À–ƒ
 * µ±”¶”√≥Ã–Ú≤Ÿ◊˜…Ë±∏Œƒº˛ ±À˘µ˜”√µƒopen°¢read°¢writeµ»∫Ø ˝£¨
 * ◊Ó÷’ª·µ˜”√’‚∏ˆΩ·ππ÷–÷∏∂®µƒ∂‘”¶∫Ø ˝
 */
static struct file_operations third_drv_fops = {
    .owner  =   THIS_MODULE,    /* ’‚ «“ª∏ˆ∫Í£¨Õ∆œÚ±‡“Îƒ£øÈ ±◊‘∂Ø¥¥Ω®µƒ__this_module±‰¡ø */
    .open   =   third_drv_open,        
	.read	=	third_drv_read,	   
	.release =	third_drv_close,	
};

int major;
/*
 * ÷¥––insmod√¸¡Ó ±æÕª·µ˜”√’‚∏ˆ∫Ø ˝ 
 */
static int third_drv_init(void)  
{
	//◊¢≤·£¨∏ÊÀﬂƒ⁄∫À
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
 * ÷¥––rmmod√¸¡Ó ±æÕª·µ˜”√’‚∏ˆ∫Ø ˝ 
 */
static void third_drv_exit(void)
{
	unregister_chrdev(major, "third_drv");//–∂‘ÿ«˝∂Ø
	class_device_unregister(thirddrv_class_dev);
	class_destroy(thirddrv_class);

	iounmap(gpfcon);
	iounmap(gpgcon);
		
	return 0;
}

module_init(third_drv_init);
module_exit(third_drv_exit);
MODULE_LICENSE("GPL");

