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


static struct class *firstdrv_class;
static struct class_device	*firstdrv_class_dev;



static int first_drv_open(struct inode *inode, struct file *file)
{
	printk("first_drv_open.\n");
	return 0;
}

static ssize_t first_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	printk("first_drv_write.\n");
	return 0;
}



/* ����ṹ���ַ��豸��������ĺ���
 * ��Ӧ�ó�������豸�ļ�ʱ�����õ�open��read��write�Ⱥ�����
 * ���ջ��������ṹ��ָ���Ķ�Ӧ����
 */
static struct file_operations first_drv_fops = {
    .owner  =   THIS_MODULE,    /* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
    .open   =   first_drv_open,        
	.write	=	first_drv_write,	   
};

int major;
/*
 * ִ��insmod����ʱ�ͻ����������� 
 */
static int first_drv_init(void)  
{
	//ע�ᣬ�����ں�
	major = register_chrdev(0, "first_drv", &first_drv_fops);

	firstdrv_class = class_create(THIS_MODULE, "first_drv");
	firstdrv_class_dev = class_device_create(firstdrv_class, NULL, MKDEV(major, 0), NULL, "xyz");;
	
	return 0;
}

/*
 * ִ��rmmod����ʱ�ͻ����������� 
 */
static void first_drv_exit(void)
{
	unregister_chrdev(major, "first_drv");//ж������
	class_device_unregister(firstdrv_class_dev);

	class_destroy(firstdrv_class);
}

module_init(first_drv_init);
module_exit(first_drv_exit);
MODULE_LICENSE("GPL");


