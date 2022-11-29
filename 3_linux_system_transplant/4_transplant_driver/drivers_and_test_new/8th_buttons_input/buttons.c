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
	/*10m后启动定时器中断*/
	irq_pd = (struct pin_desc *)dev_id;
	mod_timer(&buttons_timer, jiffies + (HZ / 100));
	return IRQ_RETVAL(IRQ_HANDLED);
}

/* 定时器中断处理函数 */
static void buttons_timer_fucntion(unsigned long data)
{
	//这里只需要上报"按键值":调用input_event来上报
	struct pin_desc * pindesc = irq_pd;
	unsigned int pinval;

	if(!pindesc)
		return;

	pinval =  s3c2410_gpio_getpin(pindesc->pin);

	if(pinval){
		/* 松开 : 0表示松开，1表示按下，为什么? */
		input_event(buttons_dev, EV_KEY, pindesc->key_val, 0);
		input_sync(buttons_dev);//上报同步事件
	}else{
		/* 按下 */
		input_event(buttons_dev, EV_KEY, pindesc->key_val, 1);
		input_sync(buttons_dev);//上报同步事件
	}
}


//入口函数
static int buttons_init(void)
{
	int i;
	int ret;

	/* 1. 分配一个input_dev结构体 */

	buttons_dev = input_allocate_device();

	/* 2. 设置 */
	/* 2.1 设置能产生的操作类型 */
	set_bit(EV_KEY, buttons_dev->evbit);

	/* 2.2 设置能产生的 "重复" 操作类型 */
	set_bit(EV_REP, buttons_dev->evbit);

	/* 2.2 设置能产生该类操作中的具体事件:
	   S2 - KEY_L
   	   S3 - KEY_S
   	   S4 - KEY_ENTER
   	   S5 - KEY_LEFTSHIFT
	*/
	set_bit(KEY_L, buttons_dev->keybit);
	set_bit(KEY_S, buttons_dev->keybit);
	set_bit(KEY_ENTER, buttons_dev->keybit);
	set_bit(KEY_LEFTSHIFT, buttons_dev->keybit);

	/* 3. 注册设备 */
	ret = input_register_device(buttons_dev);
	if(ret)
		printk("input_register_device fail.\n");

	

	/* 4. 硬件相关操作 */
	/*4.1 初始化定时器 */
	init_timer(&buttons_timer);
	buttons_timer.function = buttons_timer_fucntion;
	add_timer(&buttons_timer);
	
	/*4.2 注册这4个中断 */
	for(i = 0; i < 4; i++){
		ret = request_irq(pins_desc[i].irq, buttons_irq, (IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING), pins_desc[i].name, &pins_desc[i]);
		if(ret)
			printk("%d request_irq fail.\n", i);
		
	}
	
	return 0;
}
//出口函数
static void buttons_exit(void)
{
	int i;
	//释放中断
	for(i = 0; i < 4; i++){
		free_irq(pins_desc[i].irq, &pins_desc[i]);
	}
	//消除定时器
	del_timer(&buttons_timer);
	//卸载input_dev结构体
	input_unregister_device(buttons_dev);
	//释放input_dev结构体的分配空间
	input_free_device(buttons_dev);

}

module_init(buttons_init);
module_exit(buttons_exit);
MODULE_LICENSE("GPL");

