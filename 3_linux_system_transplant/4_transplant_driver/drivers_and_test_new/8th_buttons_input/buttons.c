#include <linux/module.h>
#include <linux/version.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/irq.h>

#include <asm/gpio.h>
#include <asm/io.h>


#include <linux/device.h>
#include <mach/gpio.h>
#include <linux/sched.h>
#include <linux/interrupt.h>


struct pin_desc{
	int irq;
	char *name;
	unsigned int pin;
	unsigned int key_val;
};

struct pin_desc pins_desc[4]={
	{IRQ_EINT0,  "S2", S3C2410_GPF(0),   KEY_L},//S2
	{IRQ_EINT2,  "S3", S3C2410_GPF(2),   KEY_S},//S3
	{IRQ_EINT11, "S4", S3C2410_GPG(3),   KEY_ENTER},//S4
	{IRQ_EINT19, "S5", S3C2410_GPG(11),  KEY_LEFTSHIFT},//S5
};

static struct input_dev *buttons_dev;

static struct pin_desc *irq_pd;
static struct timer_list buttons_timer;

static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	/*
	S2 - 16
	S3 - 18
	S4 - 55
	S5 - 63
	*/
	/*10m��������ʱ���ж�*/
	irq_pd = (struct pin_desc *)dev_id;
	mod_timer(&buttons_timer, jiffies + (HZ / 100));
	return IRQ_RETVAL(IRQ_HANDLED);
}

/* ��ʱ���жϴ����� */
static void buttons_timer_fucntion(unsigned long data)
{
	//����ֻ��Ҫ�ϱ�"����ֵ":����input_event���ϱ�
	struct pin_desc * pindesc = irq_pd;
	unsigned int pinval;

	if(!pindesc)
		return;

	pinval =  s3c2410_gpio_getpin(pindesc->pin);

	if(pinval){
		/* �ɿ� : 0��ʾ�ɿ���1��ʾ���£�Ϊʲô? */
		input_event(buttons_dev, EV_KEY, pindesc->key_val, 0);
		input_sync(buttons_dev);//�ϱ�ͬ���¼�
	}else{
		/* ���� */
		input_event(buttons_dev, EV_KEY, pindesc->key_val, 1);
		input_sync(buttons_dev);//�ϱ�ͬ���¼�
	}
}


//��ں���
static int buttons_init(void)
{
	int i;
	int ret;

	/* 1. ����һ��input_dev�ṹ�� */

	buttons_dev = input_allocate_device();

	/* 2. ���� */
	/* 2.1 �����ܲ����Ĳ������� */
	set_bit(EV_KEY, buttons_dev->evbit);

	/* 2.2 �����ܲ����� "�ظ�" �������� */
	set_bit(EV_REP, buttons_dev->evbit);

	/* 2.2 �����ܲ�����������еľ����¼�:
	   S2 - KEY_L
   	   S3 - KEY_S
   	   S4 - KEY_ENTER
   	   S5 - KEY_LEFTSHIFT
	*/
	set_bit(KEY_L, buttons_dev->keybit);
	set_bit(KEY_S, buttons_dev->keybit);
	set_bit(KEY_ENTER, buttons_dev->keybit);
	set_bit(KEY_LEFTSHIFT, buttons_dev->keybit);

	/* 3. ע���豸 */
	ret = input_register_device(buttons_dev);
	if(ret)
		printk("input_register_device fail.\n");

	

	/* 4. Ӳ����ز��� */
	/*4.1 ��ʼ����ʱ�� */
	init_timer(&buttons_timer);
	buttons_timer.function = buttons_timer_fucntion;
	add_timer(&buttons_timer);
	
	/*4.2 ע����4���ж� */
	for(i = 0; i < 4; i++){
		ret = request_irq(pins_desc[i].irq, buttons_irq, (IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING), pins_desc[i].name, &pins_desc[i]);
		if(ret)
			printk("%d request_irq fail.\n", i);
		
	}
	
	return 0;
}
//���ں���
static void buttons_exit(void)
{
	int i;
	//�ͷ��ж�
	for(i = 0; i < 4; i++){
		free_irq(pins_desc[i].irq, &pins_desc[i]);
	}
	//������ʱ��
	del_timer(&buttons_timer);
	//ж��input_dev�ṹ��
	input_unregister_device(buttons_dev);
	//�ͷ�input_dev�ṹ��ķ���ռ�
	input_free_device(buttons_dev);

}

module_init(buttons_init);
module_exit(buttons_exit);
MODULE_LICENSE("GPL");

