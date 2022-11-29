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
	/*����GPF0, 2Ϊ��������*/
	*gpfcon &= ~((0x3<<(0*2)) | (0x3<<(2*2)));
	/*����GPG3, 11Ϊ��������*/
	*gpgcon &= ~((0x3<<(3*2)) | (0x3<<(11*2)));

	return 0;
}

static ssize_t second_drv_read(struct file *file, const char __user *buf, size_t size, loff_t * ppos)
{
	unsigned char key_vals[4];
	int regval;
	unsigned long ret;

	if(size != sizeof(key_vals))//�������ж�ʲô�
		return -EINVAL;

	/* ��GPF0, 2 */
	regval = *gpfdat;
	key_vals[0] = (regval & (1<<0)) ? 1 : 0;//S2
	key_vals[1] = (regval & (1<<2)) ? 1 : 0;//S3

	/* ��GPG3, 11 */
	regval = *gpgdat;
	key_vals[2] = (regval & (1<<3)) ? 1 : 0;//S4
	key_vals[3] = (regval & (1<<11)) ? 1 : 0;//S5
	//���ں˿ռ俽�����ݵ��û��ռ�
	ret = copy_to_user(buf, key_vals, sizeof(key_vals));
	if(ret)
		printk("copy_to_user fail.\n");
	
	return sizeof(key_vals);
}

/* ����ṹ���ַ��豸��������ĺ���
 * ��Ӧ�ó�������豸�ļ�ʱ�����õ�open��read��write�Ⱥ�����
 * ���ջ��������ṹ��ָ���Ķ�Ӧ����
 */
static struct file_operations second_drv_fops = {
    .owner  =   THIS_MODULE,    /* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
    .open   =   second_drv_open,        
	.read	=	second_drv_read,	   
};

int major;
/*
 * ִ��insmod����ʱ�ͻ����������� 
 */
static int second_drv_init(void)  
{
	//ע�ᣬ�����ں�
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
 * ִ��rmmod����ʱ�ͻ����������� 
 */
static void second_drv_exit(void)
{
	unregister_chrdev(major, "second_drv");//ж������
	device_destroy(seconddrv_class, MKDEV(major, 0));
	class_destroy(seconddrv_class);

	iounmap(gpfcon);
	iounmap(gpgcon);	
}

module_init(second_drv_init);
module_exit(second_drv_exit);
MODULE_LICENSE("GPL");

