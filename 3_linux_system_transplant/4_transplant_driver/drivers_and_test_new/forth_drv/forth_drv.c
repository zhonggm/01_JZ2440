#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>

#include <linux/irqreturn.h>
#include <linux/irq.h>
#include <linux/poll.h>

#include <linux/device.h>
#include <mach/gpio.h>
#include <linux/sched.h>
#include <linux/interrupt.h>

static struct class *forthdrv_class;
static struct device *forthdrv_dev;

volatile unsigned long *gpfcon;
volatile unsigned long *gpfdat;

volatile unsigned long *gpgcon;
volatile unsigned long *gpgdat;

/*
DECLARE_WAIT_QUEUE_HEADÕâÊÇÒ»¸öºê£¬ÔÚinclude\linux\wait.hÖÐ¶¨ÒåÁË£¬
ËüµÄ×÷ÓÃÊÇ¶¨ÒåÒ»¸öwait_queue_head_t½á¹¹ÌåÀàÐÍ¶ÓÁÐÍ·£¬²¢ÓÃºóÃæµÄºê
__WAIT_QUEUE_HEAD_INITIALIZERÀ´³õÊ¼»¯£»
*/
static DECLARE_WAIT_QUEUE_HEAD(button_waitq);
/* 
ÖÐ¶ÏÊÂ¼þ·¢Éú±êÖ¾£¬ÖÐ¶Ï·þÎñ³ÌÐò½«ËüÖÃ1£¬ÔÚforth_drv_read½«ËüÇå0 ;
ÔÚµÈ´ýÊÂ¼þÖÐ¶Ïº¯ÊýÖÐ»á¶Ôev_press½øÐÐÅÐ¶Ï£¬Èç¹ûev_pressµÈÓÚ0µÄ»°£¬
Ëü¾ÍÊÇ½øÈëÐÝÃß£¬°Ñ½øÈëÐÝÃßµÄ½ø³Ì¹ÒÔÚ button_waitq ¶ÓÁÐÖÐ£»
*/
static volatile int ev_press = 0;

struct pin_desc{
	unsigned int pin;
	unsigned int key_val;
};
#if 1
/* ¼üÖµ : °´ÏÂÊ±£¬0x01, 0x02, 0x03, 0x04 */
/* ¼üÖµ : ËÉ¿ªÊ±£¬0x81, 0x82, 0x83, 0x84 */
static unsigned char key_val;
#endif

struct pin_desc pins_desc[4]={
		{S3C2410_GPF(0),  0x01},//S2
		{S3C2410_GPF(2),  0x02},//S3
		{S3C2410_GPG(3),  0x03},//S4
		{S3C2410_GPG(11), 0x04},//S5
};

static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	/*
	S2 - 16
	S3 - 18
	S4 - 55
	S5 - 63
	*/
//	printk("irq = %d\n", irq);

	struct pin_desc * pindesc = (struct pin_desc *)dev_id;
	unsigned int pinval;

	pinval =  s3c2410_gpio_getpin(pindesc->pin);

	if(pinval){
		/* ËÉ¿ª */
		key_val = 0x80 | (pindesc->key_val);
	}else{
		/* °´ÏÂ */
		key_val = (pindesc->key_val);
	}

	ev_press = 1;/* ±íÊ¾ÖÐ¶Ï·¢ÉúÁË */
	wake_up_interruptible(&button_waitq);/*»½ÐÑÐÝÃßµÄ½ø³Ì*/
		
	return IRQ_RETVAL(IRQ_HANDLED);
}


static int forth_drv_open(struct inode *inode, struct file *file)
{
	int ret;
	/*ÅäÖÃGPF0, 2ÎªÊäÈëÒý½Å*/ 
	/*ÅäÖÃGPG3, 11ÎªÊäÈëÒý½Å*/
	//Íâ²¿ÖÐ¶Ï0/2/11/19
	//		 S2/3/ 4/5
//	request_irq(unsigned int irq,irq_handler_t handler,unsigned long flags,const char *devname,void *dev_id);
	ret = request_irq(IRQ_EINT0,  buttons_irq, (IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING), "S2", &pins_desc[0]);
	if(ret){
		printk("IRQ_EINT0 fail.");
	}
	ret = request_irq(IRQ_EINT2,  buttons_irq, (IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING), "S3", &pins_desc[1]);
	if(ret){
		printk("IRQ_EINT2 fail.");
	}
	ret = request_irq(IRQ_EINT11, buttons_irq, (IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING), "S4", &pins_desc[2]);
	if(ret){
		printk("IRQ_EINT11 fail.");
	}
	ret = request_irq(IRQ_EINT19, buttons_irq, (IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING), "S5", &pins_desc[3]);
	if(ret){
		printk("IRQ_EINT19 fail.");
	}

	return 0;
}


static ssize_t forth_drv_read(struct file *file, const char __user *buf, size_t size, loff_t * ppos)
{
//	unsigned char key_vals[4];
//	int regval;
	unsigned long ret;

	if(size != 1)//ÕâÀïÊÇÅÐ¶ÏÊ²Ã´å
		return -EINVAL;

	/*Èç¹ûÃ»ÓÐ°´¼ü¶¯×÷£¬ÐÝÃß*/
	wait_event_interruptible(button_waitq, ev_press);

	/*Èç¹ûÓÐ°´¼ü¶¯×÷£¬·µ»Ø¼üÖµ*/
	ret = copy_to_user(buf, &key_val, 1);
	if(ret)
		printk("copy_to_user fail.");
	ev_press = 0;

	return 1;
	
#if 0
	/* ¶ÁGPF0, 2 */
	regval = *gpfdat;
	key_vals[0] = (regval & (1<<0)) ? 1 : 0;//S2
	key_vals[1] = (regval & (1<<2)) ? 1 : 0;//S3

	/* ¶ÁGPG3, 11 */
	regval = *gpgdat;
	key_vals[2] = (regval & (1<<3)) ? 1 : 0;//S4
	key_vals[3] = (regval & (1<<11)) ? 1 : 0;//S5

	copy_to_user(buf, key_vals, sizeof(key_vals));
	return sizeof(key_vals);
#endif
}


//ÊÍ·ÅÖÐ¶Ï
int forth_drv_close(struct inode *inode, struct file *file)
{
	free_irq(IRQ_EINT0,  &pins_desc[0]);
	free_irq(IRQ_EINT2,  &pins_desc[1]);
	free_irq(IRQ_EINT11, &pins_desc[2]);
	free_irq(IRQ_EINT19, &pins_desc[3]);
	
	return 0;
}

static unsigned forth_drv_poll(struct file *file, poll_table *wait)
{
	unsigned mask = 0;

	poll_wait(file, &button_waitq, wait);//²»»áÁ¢¼´ÐÝÃß
	if(ev_press)//¼ü°´ÏÂ
		mask |= POLLIN | POLLRDNORM;

	return mask;
}



/* Õâ¸ö½á¹¹ÊÇ×Ö·ûÉè±¸Çý¶¯³ÌÐòµÄºËÐÄ
 * µ±Ó¦ÓÃ³ÌÐò²Ù×÷Éè±¸ÎÄ¼þÊ±Ëùµ÷ÓÃµÄopen¡¢read¡¢writeµÈº¯Êý£¬
 * ×îÖÕ»áµ÷ÓÃÕâ¸ö½á¹¹ÖÐÖ¸¶¨µÄ¶ÔÓ¦º¯Êý
 */
static struct file_operations forth_drv_fops = {
    .owner   =   THIS_MODULE,    /* ÕâÊÇÒ»¸öºê£¬ÍÆÏò±àÒëÄ£¿éÊ±×Ô¶¯´´½¨µÄ__this_module±äÁ¿ */
    .open    =   forth_drv_open,        
	.read	 =	forth_drv_read,	   
	.release =	forth_drv_close,	
	.poll 	 =	forth_drv_poll,
};


int major;
/*
 * Ö´ÐÐinsmodÃüÁîÊ±¾Í»áµ÷ÓÃÕâ¸öº¯Êý 
 */
static int forth_drv_init(void)  
{
	//×¢²á£¬¸æËßÄÚºË
	major = register_chrdev(0, "forth_drv", &forth_drv_fops);

	forthdrv_class = class_create(THIS_MODULE, "forth_drv");
	forthdrv_dev = device_create(forthdrv_class, NULL, MKDEV(major, 0), NULL, "buttons");

	gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);
	gpfdat = gpfcon + 1;

	gpgcon = (volatile unsigned long *)ioremap(0x56000060, 16);
	gpgdat = gpgcon + 1;
	
	return 0;
}

/*
 * Ö´ÐÐrmmodÃüÁîÊ±¾Í»áµ÷ÓÃÕâ¸öº¯Êý 
 */
static void forth_drv_exit(void)
{
	unregister_chrdev(major, "forth_drv");//Ð¶ÔØÇý¶¯
	
	device_destroy(forthdrv_class, MKDEV(major, 0));
	class_destroy(forthdrv_class);

	iounmap(gpfcon);
	iounmap(gpgcon);
}

module_init(forth_drv_init);
module_exit(forth_drv_exit);
MODULE_LICENSE("GPL");

