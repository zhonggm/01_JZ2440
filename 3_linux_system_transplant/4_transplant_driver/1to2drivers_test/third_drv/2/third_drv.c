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

/*
DECLARE_WAIT_QUEUE_HEADÕâÊÇÒ»¸öºê£¬ÔÚinclude\linux\wait.hÖĞ¶¨ÒåÁË£¬
ËüµÄ×÷ÓÃÊÇ¶¨ÒåÒ»¸öwait_queue_head_t½á¹¹ÌåÀàĞÍ¶ÓÁĞÍ·£¬²¢ÓÃºóÃæµÄºê
__WAIT_QUEUE_HEAD_INITIALIZERÀ´³õÊ¼»¯£»
*/
static DECLARE_WAIT_QUEUE_HEAD(button_waitq);
/* 
ÖĞ¶ÏÊÂ¼ş·¢Éú±êÖ¾£¬ÖĞ¶Ï·şÎñ³ÌĞò½«ËüÖÃ1£¬ÔÚthird_drv_read½«ËüÇå0 ;
ÔÚµÈ´ıÊÂ¼şÖĞ¶Ïº¯ÊıÖĞ»á¶Ôev_press½øĞĞÅĞ¶Ï£¬Èç¹ûev_pressµÈÓÚ0µÄ»°£¬
Ëü¾ÍÊÇ½øÈëĞİÃß£¬°Ñ½øÈëĞİÃßµÄ½ø³Ì¹ÒÔÚ button_waitq ¶ÓÁĞÖĞ£»
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
		{S3C2410_GPF0,  0x01},//S2
		{S3C2410_GPF2,  0x02},//S3
		{S3C2410_GPG3,  0x03},//S4
		{S3C2410_GPG11, 0x04},//S5
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

	ev_press = 1;/* ±íÊ¾ÖĞ¶Ï·¢ÉúÁË */
	wake_up_interruptible(&button_waitq);/*»½ĞÑĞİÃßµÄ½ø³Ì*/
		
	return IRQ_RETVAL(IRQ_HANDLED);
}


static int third_drv_open(struct inode *inode, struct file *file)
{
	/*ÅäÖÃGPF0, 2ÎªÊäÈëÒı½Å*/ 
	/*ÅäÖÃGPG3, 11ÎªÊäÈëÒı½Å*/
	//Íâ²¿ÖĞ¶Ï0/2/11/19
	//       S2/3/ 4/5
//	request_irq(unsigned int irq,irq_handler_t handler,unsigned long flags,const char *devname,void *dev_id);
	request_irq(IRQ_EINT0, buttons_irq, IRQT_BOTHEDGE, "S2", &pins_desc[0]);
	request_irq(IRQ_EINT2, buttons_irq, IRQT_BOTHEDGE, "S3", &pins_desc[1]);
	request_irq(IRQ_EINT11, buttons_irq, IRQT_BOTHEDGE, "S4", &pins_desc[2]);
	request_irq(IRQ_EINT19, buttons_irq, IRQT_BOTHEDGE, "S5", &pins_desc[3]);

	return 0;
}

static ssize_t third_drv_read(struct file *file, const char __user *buf, size_t size, loff_t * ppos)
{
//	unsigned char key_vals[4];
//	int regval;

	if(size != 1)//ÕâÀïÊÇÅĞ¶ÏÊ²Ã´å
		return -EINVAL;

	/*Èç¹ûÃ»ÓĞ°´¼ü¶¯×÷£¬ĞİÃß*/
	wait_event_interruptible(button_waitq, ev_press);

	/*Èç¹ûÓĞ°´¼ü¶¯×÷£¬·µ»Ø¼üÖµ*/
	copy_to_user(buf, &key_val, 1);
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

//ÊÍ·ÅÖĞ¶Ï
int third_drv_close(struct inode *inode, struct file *file)
{
	free_irq(IRQ_EINT0,  &pins_desc[0]);
	free_irq(IRQ_EINT2,  &pins_desc[1]);
	free_irq(IRQ_EINT11, &pins_desc[2]);
	free_irq(IRQ_EINT19, &pins_desc[3]);
	
	return 0;
}



/* Õâ¸ö½á¹¹ÊÇ×Ö·ûÉè±¸Çı¶¯³ÌĞòµÄºËĞÄ
 * µ±Ó¦ÓÃ³ÌĞò²Ù×÷Éè±¸ÎÄ¼şÊ±Ëùµ÷ÓÃµÄopen¡¢read¡¢writeµÈº¯Êı£¬
 * ×îÖÕ»áµ÷ÓÃÕâ¸ö½á¹¹ÖĞÖ¸¶¨µÄ¶ÔÓ¦º¯Êı
 */
static struct file_operations third_drv_fops = {
    .owner  =   THIS_MODULE,    /* ÕâÊÇÒ»¸öºê£¬ÍÆÏò±àÒëÄ£¿éÊ±×Ô¶¯´´½¨µÄ__this_module±äÁ¿ */
    .open   =   third_drv_open,        
	.read	=	third_drv_read,	   
	.release =	third_drv_close,	
};

int major;
/*
 * Ö´ĞĞinsmodÃüÁîÊ±¾Í»áµ÷ÓÃÕâ¸öº¯Êı 
 */
static int third_drv_init(void)  
{
	//×¢²á£¬¸æËßÄÚºË
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
 * Ö´ĞĞrmmodÃüÁîÊ±¾Í»áµ÷ÓÃÕâ¸öº¯Êı 
 */
static void third_drv_exit(void)
{
	unregister_chrdev(major, "third_drv");//Ğ¶ÔØÇı¶¯
	class_device_unregister(thirddrv_class_dev);
	class_destroy(thirddrv_class);

	iounmap(gpfcon);
	iounmap(gpgcon);
		
	return 0;
}

module_init(third_drv_init);
module_exit(third_drv_exit);
MODULE_LICENSE("GPL");

