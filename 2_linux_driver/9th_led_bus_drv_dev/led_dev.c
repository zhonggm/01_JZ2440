#include <linux/module.h>
#include <linux/version.h>

#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>


/* ����/����/ע��һ�� platform_device */
////1.1 �����ʼ��һ����Դ�����
static struct resource led_resource[] = {
    [0] = {
        .start = 0x56000050,
        .end   = 0x56000050 + 8 - 1,
        .flags = IORESOURCE_MEM,
    },
    [1] = {
        .start = 5,
        .end   = 5,
        .flags = IORESOURCE_IRQ,
    }
};

static void led_release(struct device * dev)
{
	printk("led_release, remove led_dev\n");
}

////1.2 �����ʼ��һ��ƽ̨�豸�����
static struct platform_device led_dev = {
    .name         = "myled",
    .id       = -1,
    .num_resources    = ARRAY_SIZE(led_resource),
    .resource     = led_resource,
    .dev = { 
    	.release = led_release, 
	},
};


////2.1 ��ں�����ע��ƽ̨�豸
static int led_dev_init(void)
{
	platform_device_register(&led_dev);
	return 0;
}


////2.2 ���ں���, ж��ƽ̨�豸
static void led_dev_exit(void)
{	
	platform_device_unregister(&led_dev);
}


////3 ���κ���
module_init(led_dev_init);
module_exit(led_dev_exit);
MODULE_LICENSE("GPL");



