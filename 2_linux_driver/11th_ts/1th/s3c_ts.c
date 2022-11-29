
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/serio.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/plat-s3c24xx/ts.h>
#include <asm/arch/regs-adc.h>
#include <asm/arch/regs-gpio.h>

static struct input_dev *s3c_ts_dev;

static int s3c_ts_init(void)
{
	//1.分配一个 input_dev 结构体。 
	s3c_ts_dev = input_allocate_device();


	
	//2.设置,设置分为两大类:
	//2.1，能产生哪类事件;
	set_bit(EV_KEY, s3c_ts_dev->evbit);//能产生按键类事件
	set_bit(EV_ABS, s3c_ts_dev->evbit);//能产生触摸屏绝对位移事件	

	//2.2, 能产生这类事件里的哪些事件. 
	set_bit(BTN_TOUCH, s3c_ts_dev->evbit);//能产生按键类里的触摸屏事件
	input_set_abs_params(s3c_ts_dev, ABS_X, 0, 0x3FF, 0, 0);//X方向最小值0，最大值0x3FF(10位ADC)
	input_set_abs_params(s3c_ts_dev, ABS_Y, 0, 0x3FF, 0, 0);//Y方向最小值0，最大值0x3FF(10位ADC)
	input_set_abs_params(s3c_ts_dev, ABS_PRESSURE, 0, 1, 0, 0);//压力方向最小值0，最大值1(要么按下，要么松开)
	
	//3.注册 
	input_register_device(s3c_ts_dev);
	
	//4.硬件相关的操作 

	return 0;
}


static int s3c_ts_exit(void)
{

}

module_init(s3c_ts_init);
module_exit(s3c_ts_exit);
MODULE_LICENSE("GPL");//协议










