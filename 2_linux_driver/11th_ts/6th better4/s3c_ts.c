
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

struct s3c_ts_regs {
	unsigned long adccon;
	unsigned long adctsc;
	unsigned long adcdly;
	unsigned long adcdat0;
	unsigned long adcdat1;
	unsigned long adcupdn;
};

static struct input_dev *s3c_ts_dev;
static volatile struct s3c_ts_regs *s3c_ts_regs;

//进入按下“等待中断模式”
static void enter_wait_pen_down_mode(void)
{
	s3c_ts_regs->adctsc = 0xd3;
}

//进入松开“等待中断模式”
static void enter_wait_pen_up_mode(void)
{
	s3c_ts_regs->adctsc = 0x1d3;//(Bit[8] = 1)
}

//进入自动转换XY模式
static void enter_measure_xy_mode(void)
{
	/*
	Bit[3]:Pull-up Switch Disable, 
	Bit[2]:Automatically sequencing conversion of X-Position and YPosition
	*/
	s3c_ts_regs->adctsc = (1<<3)|(1<<2);
}


static void start_adc(void)
{
	//A/D conversion starts by enable
	s3c_ts_regs->adccon |= (1<<0);
}


//中断处理函数
static irqreturn_t pen_down_up_irq(int irq, void *dev_id)
{
	//2440 手册中 ADCDAT0 寄存器 bit15 置 0 时表示触摸笔按下状态，置 1 时表示触摸笔松开状态。
	if (s3c_ts_regs->adcdat0 & (1<<15)){
		printk("pen up\n"); //已松开
		enter_wait_pen_down_mode();//进入等待按下检测
	}else{
		//printk("pen down\n");//已按下
		//enter_wait_pen_up_mode();  //进入等待松开检测
		enter_measure_xy_mode();
		start_adc();
	}
	
	return IRQ_HANDLED;
}

static int s3c_filter_ts(int x[], int y[])
{
#define ERR_LIMIT 10 //若定义了误差是 10.这是一个经验值。
	
	int avr_x, avr_y;// //4 次测量值，两两的平均值。
	int det_x, det_y; //误差值。
	
	avr_x = (x[0] + x[1])/2; //取平均值
	avr_y = (y[0] + y[1])/2;
	det_x = (x[2] > avr_x) ? (x[2] - avr_x) : (avr_x - x[2]); //求出误差值。
	det_y = (y[2] > avr_y) ? (y[2] - avr_y) : (avr_y - y[2]);
	if ((det_x > ERR_LIMIT) || (det_y > ERR_LIMIT)) //若误差大过 ERR_TIMET 就认为错直接返回.
		return 0;
	
	avr_x = (x[1] + x[2])/2;
	avr_y = (y[1] + y[2])/2;
	det_x = (x[3] > avr_x) ? (x[3] - avr_x) : (avr_x - x[3]);
	det_y = (y[3] > avr_y) ? (y[3] - avr_y) : (avr_y - y[3]);
	if ((det_x > ERR_LIMIT) || (det_y > ERR_LIMIT))return 0;
		return 1;
}


static irqreturn_t adc_irq(int irq, void *dev_id)
{
	static int cnt = 0;
	int adcdat0, adcdat1;
	static int x[4], y[4]; //XY 坐标电压值都测量 4 次。
	
	/* 4.2,优化措施 2: 如果 ADC 完成时, 发现触摸笔已经松开, 则丢弃此次结果 */
		//ADCDAT0 寄存的低 10 位就是 X 坐标值.ADCDAT1 低 10 就是 Y 坐标值.
	adcdat0 = s3c_ts_regs->adcdat0;
	adcdat1 = s3c_ts_regs->adcdat1;
	
	if (s3c_ts_regs->adcdat0 & (1<<15)){ //ADCDAT0 bit15=1 时是松开状态
	
		/* 已经松开,就等待触摸笔按下模式。这时不打印. */
		enter_wait_pen_down_mode();
		cnt = 0;
		
	}else{ //否则就是 ADCDAT0 bit15=0，是触摸笔按下的状态，这时打印 XY 坐标值。
	
		//ADCDAT0 寄存的低 10 位就是 X 坐标值.ADCDAT1 低 10 就是 Y 坐标值.打印完后再等待松开模式.
		//printk("adc_irq cnt = %d, x = %d, y = %d\n", ++cnt, adcdat0 & 0x3ff, adcdat1 & 0x3ff);
		/* 优化措施 3: 多次测量求平均值 */
		x[cnt] = adcdat0 & 0x3ff;
		y[cnt] = adcdat1 & 0x3ff;
		++cnt;
		if(cnt == 4){
			/* 优化措施4: 软件过滤 */
			if (s3c_filter_ts(x, y)){
				printk("x = %d, y = %d \n", (x[0] + x[1] + x[2] + x[3]) / 4, (y[0] + y[1] + y[2] + y[3]) / 4);
			}
			
			cnt = 0;
			enter_wait_pen_up_mode();//进入等待松开模式才能连续
		}else{
			//还没到达4次再进入测量
			enter_measure_xy_mode();
			start_adc();
		}
	}
	
	return IRQ_HANDLED;
}




static int s3c_ts_init(void)
{
	struct clk* clk;

	//1.分配一个 input_dev 结构体。 
	s3c_ts_dev = input_allocate_device();
	
	//2.设置,设置分为两大类:
	//2.1，能产生哪类事件;
	set_bit(EV_KEY, s3c_ts_dev->evbit);//能产生按键类事件
	set_bit(EV_ABS, s3c_ts_dev->evbit);//能产生触摸屏绝对位移事件	

	//2.2, 能产生这类事件里的哪些事件. 
	set_bit(BTN_TOUCH, s3c_ts_dev->evbit);//能产生按键类里的触摸屏事件
	input_set_abs_params(s3c_ts_dev, 		ABS_X, 0, 0x3FF, 0, 0);//X方向最小值0，最大值0x3FF(10位ADC)
	input_set_abs_params(s3c_ts_dev, 		ABS_Y, 0, 0x3FF, 0, 0);//Y方向最小值0，最大值0x3FF(10位ADC)
	input_set_abs_params(s3c_ts_dev, ABS_PRESSURE, 0, 	  1, 0, 0);//压力方向最小值0，最大值1(要么按下，要么松开)
	
	//3.注册设备驱动
	input_register_device(s3c_ts_dev);
	
	//4.硬件相关的操作 
	/*4.1 使能时钟( CLKCON[15] )*/
	clk = clk_get(NULL, "adc");/* 参考s3c2410_ts.c里的probe函数 */
	clk_enable(clk);
	
	/*4.2 设置S3C2440的ADC / TS寄存器 */
	s3c_ts_regs = ioremap(0x58000000, sizeof(struct s3c_ts_regs));

	/*
	设置 ADCCON 寄存器
	PRSCEN Bit[14]: A/D converter prescaler enable,1 = Enable
	PRSCVL Bit[13:6]: A/D converter prescaler value, set 49,
					A/D converter freq. = 50MHz/(49+1) = 1MHz
	ENABLE_START[0][0]:A/D conversion starts by enable,这里先设为0，以后要启动ADC时再设置。
	*/	
	s3c_ts_regs->adccon = (1<<14) | (49<<6);

	//注册中断处理函数
	request_irq(IRQ_TC, pen_down_up_irq, IRQF_SAMPLE_RANDOM, "ts_pen", NULL);
	request_irq(IRQ_ADC, adc_irq, IRQF_SAMPLE_RANDOM, "adc", NULL);

	/*
	优化错施1:
	设置ADCDLY为最大值，这使得电压稳定后再发出IRQ_TC中断
	*/
	s3c_ts_regs->adcdly = 0xffff;

	enter_wait_pen_down_mode();//一开始先设置触摸笔等待被按下的模式

	return 0;
}

static void s3c_ts_exit(void)
{
	//释放 IRQ_TC.
	free_irq(IRQ_TC, NULL);
	
	//释放 IRQ_ADC 中断
	free_irq(IRQ_ADC, NULL);
	
	//iounmap 掉寄存器
	iounmap(s3c_ts_regs);
	
	//从内核释放此s3c_ts_dev结构体
	input_unregister_device(s3c_ts_dev);
	
	//释放s3c_ts_dev结构体空间.
	input_free_device(s3c_ts_dev);	
}

module_init(s3c_ts_init);
module_exit(s3c_ts_exit);
MODULE_LICENSE("GPL");//协议

