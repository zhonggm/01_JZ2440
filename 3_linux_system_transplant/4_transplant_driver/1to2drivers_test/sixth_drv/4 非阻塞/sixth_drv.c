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
#include <linux/poll.h>


static struct class *sixthdrv_class;
static struct class_device	*sixthdrv_class_dev;

volatile unsigned long *gpfcon;
volatile unsigned long *gpfdat;

volatile unsigned long *gpgcon;
volatile unsigned long *gpgdat;

static struct fasync_struct *button_async;


/*
DECLARE_WAIT_QUEUE_HEAD这是一个宏，在include\linux\wait.h中定义了，
它的作用是定义一个wait_queue_head_t结构体类型队列头，并用后面的宏
__WAIT_QUEUE_HEAD_INITIALIZER来初始化；
*/
static DECLARE_WAIT_QUEUE_HEAD(button_waitq);
/* 
中断事件发生标志，中断服务程序将它置1，在forth_drv_read将它清0 ;
在等待事件中断函数中会对ev_press进行判断，如果ev_press等于0的话，
它就是进入休眠，把进入休眠的进程挂在 button_waitq 队列中；
*/
static volatile int ev_press = 0;

struct pin_desc{
	unsigned int pin;
	unsigned int key_val;
};


/* 键值 : 按下时，0x01, 0x02, 0x03, 0x04 */
/* 键值 : 松开时，0x81, 0x82, 0x83, 0x84 */
static unsigned char key_val;

struct pin_desc pins_desc[4]={
		{S3C2410_GPF0,  0x01},//S2
		{S3C2410_GPF2,  0x02},//S3
		{S3C2410_GPG3,  0x03},//S4
		{S3C2410_GPG11, 0x04},//S5
};

#if 0

static atomic_t canopen = ATOMIC_INIT(1);     //定义原子变量canopen并初始化为1

#else

static DECLARE_MUTEX(button_lock);//define a semaphore variate, named button_lock

#endif

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
		/* 松开 */
		key_val = 0x80 | (pindesc->key_val);
	}else{
		/* 按下 */
		key_val = (pindesc->key_val);
	}

	ev_press = 1;/* 表示中断发生了 */
	wake_up_interruptible(&button_waitq);/*唤醒休眠的进程*/

	kill_fasync (&button_async, SIGIO, POLL_IN);
		
	return IRQ_RETVAL(IRQ_HANDLED);
}


static int sixth_drv_open(struct inode *inode, struct file *file)
{
	/*配置GPF0, 2为输入引脚*/ 
	/*配置GPG3, 11为输入引脚*/
	//外部中断0/2/11/19
	//       S2/3/ 4/5
	/*
	如果是第一次被打开，自减后会变为0，则顺序执行下去；
	如果不是第一次被打开，自减后会变为负数，不等于0，就返回错误。
	*/
	#if 0
	
	if(!atomic_dec_and_test(&canopen)){//自减操作后测试其是否为0，为0则返回true，否则返回false。
		atomic_inc(&canopen);
		return -EBUSY;
	}
	#else
	if(file->f_flags & O_NONBLOCK){
		if(down_trylock(&button_lock))
			return -EBUSY;
	}else{
		//get the semaphore if the drv is opened first.
		down(&button_lock);
	}
	
	#endif
	
//	request_irq(unsigned int irq,irq_handler_t handler,unsigned long flags,const char *devname,void *dev_id);
	request_irq(IRQ_EINT0, buttons_irq, IRQT_BOTHEDGE, "S2", &pins_desc[0]);
	request_irq(IRQ_EINT2, buttons_irq, IRQT_BOTHEDGE, "S3", &pins_desc[1]);
	request_irq(IRQ_EINT11, buttons_irq, IRQT_BOTHEDGE, "S4", &pins_desc[2]);
	request_irq(IRQ_EINT19, buttons_irq, IRQT_BOTHEDGE, "S5", &pins_desc[3]);

	return 0;
}

static ssize_t sixth_drv_read(struct file *file, const char __user *buf, size_t size, loff_t * ppos)
{
//	unsigned char key_vals[4];
//	int regval;

	if(size != 1)//这里是判断什么?		
		return -EINVAL;

	if(file->f_flags & O_NONBLOCK){
		if(!ev_press)
			return -EAGAIN;
	}else{
		/*如果没有按键动作，休眠*/
		wait_event_interruptible(button_waitq, ev_press);
	}
	

	/*如果有按键动作，返回键值*/
	copy_to_user(buf, &key_val, 1);
	ev_press = 0;

	return 1;
	
	#if 0
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
	#endif
}

//释放中断
int sixth_drv_close(struct inode *inode, struct file *file)
{
	#if 0
	atomic_inc(&canopen);
	#else
	up(&button_lock);
	#endif

	free_irq(IRQ_EINT0,  &pins_desc[0]);
	free_irq(IRQ_EINT2,  &pins_desc[1]);
	free_irq(IRQ_EINT11, &pins_desc[2]);
	free_irq(IRQ_EINT19, &pins_desc[3]);
	
	return 0;
}

static unsigned sixth_drv_poll(struct file *file, poll_table *wait)
{
	unsigned mask = 0;

	poll_wait(file, &button_waitq, wait);//不会立即休眠
	if(ev_press)//键按下
		mask |= POLLIN | POLLRDNORM;

	return mask;
}


static int sixth_drv_fasync (int fd, struct file *filp, int on)
{
	printk("driver: sixth_drv_fasync\n");
	return fasync_helper (fd, filp, on, &button_async);
}

/* 这个结构是字符设备驱动程序的核心
 * 当应用程序操作设备文件时所调用的open、read、write等函数，
 * 最终会调用这个结构中指定的对应函数
 */
static struct file_operations sixth_drv_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   sixth_drv_open,        
	.read	=	sixth_drv_read,	   
	.release =	sixth_drv_close,	
	.poll   =	sixth_drv_poll,
	.fasync = sixth_drv_fasync,
};


int major;
/*
 * 执行insmod命令时就会调用这个函数 
 */
static int sixth_drv_init(void)  
{
	//注册，告诉内核
	major = register_chrdev(0, "sixth_drv", &sixth_drv_fops);

	sixthdrv_class = class_create(THIS_MODULE, "sixth_drv");
	sixthdrv_class_dev = class_device_create(sixthdrv_class, NULL, MKDEV(major, 0), NULL, "buttons");

	gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);
	gpfdat = gpfcon + 1;

	gpgcon = (volatile unsigned long *)ioremap(0x56000060, 16);
	gpgdat = gpgcon + 1;
	
	return 0;
}

/*
 * 执行rmmod命令时就会调用这个函数 
 */
static void sixth_drv_exit(void)
{
	unregister_chrdev(major, "sixth_drv");//卸载驱动
	class_device_unregister(sixthdrv_class_dev);
	class_destroy(sixthdrv_class);

	iounmap(gpfcon);
	iounmap(gpgcon);
		
	return 0;
}

module_init(sixth_drv_init);
module_exit(sixth_drv_exit);
MODULE_LICENSE("GPL");

