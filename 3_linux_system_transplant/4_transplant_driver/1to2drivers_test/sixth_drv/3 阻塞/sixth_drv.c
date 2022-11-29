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
DECLARE_WAIT_QUEUE_HEAD����һ���꣬��include\linux\wait.h�ж����ˣ�
���������Ƕ���һ��wait_queue_head_t�ṹ�����Ͷ���ͷ�����ú���ĺ�
__WAIT_QUEUE_HEAD_INITIALIZER����ʼ����
*/
static DECLARE_WAIT_QUEUE_HEAD(button_waitq);
/* 
�ж��¼�������־���жϷ����������1����forth_drv_read������0 ;
�ڵȴ��¼��жϺ����л��ev_press�����жϣ����ev_press����0�Ļ���
�����ǽ������ߣ��ѽ������ߵĽ��̹��� button_waitq �����У�
*/
static volatile int ev_press = 0;

struct pin_desc{
	unsigned int pin;
	unsigned int key_val;
};


/* ��ֵ : ����ʱ��0x01, 0x02, 0x03, 0x04 */
/* ��ֵ : �ɿ�ʱ��0x81, 0x82, 0x83, 0x84 */
static unsigned char key_val;

struct pin_desc pins_desc[4]={
		{S3C2410_GPF0,  0x01},//S2
		{S3C2410_GPF2,  0x02},//S3
		{S3C2410_GPG3,  0x03},//S4
		{S3C2410_GPG11, 0x04},//S5
};

#if 0

static atomic_t canopen = ATOMIC_INIT(1);     //����ԭ�ӱ���canopen����ʼ��Ϊ1

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
		/* �ɿ� */
		key_val = 0x80 | (pindesc->key_val);
	}else{
		/* ���� */
		key_val = (pindesc->key_val);
	}

	ev_press = 1;/* ��ʾ�жϷ����� */
	wake_up_interruptible(&button_waitq);/*�������ߵĽ���*/

	kill_fasync (&button_async, SIGIO, POLL_IN);
		
	return IRQ_RETVAL(IRQ_HANDLED);
}


static int sixth_drv_open(struct inode *inode, struct file *file)
{
	/*����GPF0, 2Ϊ��������*/ 
	/*����GPG3, 11Ϊ��������*/
	//�ⲿ�ж�0/2/11/19
	//       S2/3/ 4/5
	/*
	����ǵ�һ�α��򿪣��Լ�����Ϊ0����˳��ִ����ȥ��
	������ǵ�һ�α��򿪣��Լ�����Ϊ������������0���ͷ��ش���
	*/
	#if 0
	
	if(!atomic_dec_and_test(&canopen)){//�Լ�������������Ƿ�Ϊ0��Ϊ0�򷵻�true�����򷵻�false��
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

	if(size != 1)//�������ж�ʲô?		
		return -EINVAL;

	if(file->f_flags & O_NONBLOCK){
		if(!ev_press)
			return -EAGAIN;
	}else{
		/*���û�а�������������*/
		wait_event_interruptible(button_waitq, ev_press);
	}
	

	/*����а������������ؼ�ֵ*/
	copy_to_user(buf, &key_val, 1);
	ev_press = 0;

	return 1;
	
	#if 0
	/* ��GPF0, 2 */
	regval = *gpfdat;
	key_vals[0] = (regval & (1<<0)) ? 1 : 0;//S2
	key_vals[1] = (regval & (1<<2)) ? 1 : 0;//S3

	/* ��GPG3, 11 */
	regval = *gpgdat;
	key_vals[2] = (regval & (1<<3)) ? 1 : 0;//S4
	key_vals[3] = (regval & (1<<11)) ? 1 : 0;//S5

	copy_to_user(buf, key_vals, sizeof(key_vals));
	return sizeof(key_vals);
	#endif
}

//�ͷ��ж�
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

	poll_wait(file, &button_waitq, wait);//������������
	if(ev_press)//������
		mask |= POLLIN | POLLRDNORM;

	return mask;
}


static int sixth_drv_fasync (int fd, struct file *filp, int on)
{
	printk("driver: sixth_drv_fasync\n");
	return fasync_helper (fd, filp, on, &button_async);
}

/* ����ṹ���ַ��豸��������ĺ���
 * ��Ӧ�ó�������豸�ļ�ʱ�����õ�open��read��write�Ⱥ�����
 * ���ջ��������ṹ��ָ���Ķ�Ӧ����
 */
static struct file_operations sixth_drv_fops = {
    .owner  =   THIS_MODULE,    /* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
    .open   =   sixth_drv_open,        
	.read	=	sixth_drv_read,	   
	.release =	sixth_drv_close,	
	.poll   =	sixth_drv_poll,
	.fasync = sixth_drv_fasync,
};


int major;
/*
 * ִ��insmod����ʱ�ͻ����������� 
 */
static int sixth_drv_init(void)  
{
	//ע�ᣬ�����ں�
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
 * ִ��rmmod����ʱ�ͻ����������� 
 */
static void sixth_drv_exit(void)
{
	unregister_chrdev(major, "sixth_drv");//ж������
	class_device_unregister(sixthdrv_class_dev);
	class_destroy(sixthdrv_class);

	iounmap(gpfcon);
	iounmap(gpgcon);
		
	return 0;
}

module_init(sixth_drv_init);
module_exit(sixth_drv_exit);
MODULE_LICENSE("GPL");

