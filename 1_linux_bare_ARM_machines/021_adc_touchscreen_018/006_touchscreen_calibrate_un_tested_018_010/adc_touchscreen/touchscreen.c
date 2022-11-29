#include "../s3c2440_soc.h"
#include "../interrupt.h"

#include "touchscreen.h"
#include "adc.h"


#define ADC_INT_BIT (10)
#define TC_INT_BIT  (9)
#define INT_ADC_TC   (31)


/* ADCTSC寄存器位 */
#define DETECT_UP    (1<<8)/* 1检测松开中断信号，0检测按下中断信号 */
#define DETECT_DOWN  (0<<8)/* 1检测松开中断信号，0检测按下中断信号 */

#define YM_ENABLE   (1<<7)/* 1YM enable，0YM disable */
#define YM_DISABLE  (0<<7)

#define YP_ENABLE   (0<<6)/* 0YP enable，1YP disable */
#define YP_DISABLE  (1<<6)/* 0YP enable，1YP disable */

#define XM_ENABLE   (1<<5)/* 1XM enable，0XM disable */
#define XM_DISABLE  (0<<5)/* 1XM enable，0XM disable */

#define XP_ENABLE   (0<<4)/* 0XP enable，1XP disable */
#define XP_DISABLE  (1<<4)/* 0XP enable，1XP disable */

#define XP_PULL_UP_ENABLE   (0<<3)/* 0 XP_PULL_UP enable，1 XP_PULL_UP disable */
#define XP_PULL_UP_DISABLE  (1<<3)/* 0 XP_PULL_UP enable，1 XP_PULL_UP disable */


#define AUTO_PST_MODE  (1<<2)/* 1自动模式，0正常模式 */
#define NORMAL_MODE    (0<<2)/* 1自动模式，0正常模式 */

#define NO_OPERATION   (0<<0)/* 没有操作 */
#define X_MEASUREMENT  (1<<0)/* X坐标测量中 */
#define Y_MEASUREMENT  (2<<0)/* Y坐标测量中 */
#define WAITING_FOR_INTERRUPT_MODE  (3<<0)/* 等待中断模式 */

static volatile g_ts_timer_enable = 0;
static int g_ts_x = 0;
static int g_ts_y = 0;
static volatile char g_ts_data_valid = 0;

/* Waiting for Interrupt Mode: 松开检测 */
void enter_wait_pen_up_mode(void)
{
	ADCTSC = DETECT_UP | XP_PULL_UP_ENABLE | YM_ENABLE | YP_DISABLE | XP_DISABLE | XM_DISABLE | WAITING_FOR_INTERRUPT_MODE;
}


/* Waiting for Interrupt Mode: 按下检测 */
void enter_wait_pen_down_mode(void)
{
	ADCTSC = DETECT_DOWN | XP_PULL_UP_ENABLE | YM_ENABLE | YP_DISABLE | XP_DISABLE | XM_DISABLE | WAITING_FOR_INTERRUPT_MODE;
}

/* 自动测量模式 */
void enter_auto_measure_mode(void)
{
	ADCTSC = AUTO_PST_MODE | NO_OPERATION;
}



void Isr_Tc(void)
{
	//printf("ADCUPDN = 0x%x, ADCDAT0 = 0x%x, ADCDAT1 = 0x%x, ADCTSC = 0x%x\n\r", ADCUPDN, ADCDAT0, ADCDAT1, ADCTSC);
	
	if (ADCDAT0 & (1<<15))//up
	{
		//printf("pen up\n\r");
		enter_wait_pen_down_mode();
	}
	else	//down
	{
		//printf("pen down\n\r");
		/* 进入"等待触摸笔松开的模式" */
		enter_auto_measure_mode();

		/* 启动ADC */
		ADCCON |= (ENABLE_START_1<<0);
	}
}

static void ts_timer_enable(void)
{
	g_ts_timer_enable = 1;
}

static void ts_timer_disable(void)
{
	g_ts_timer_enable = 0;
}

static int get_status_of_ts_timer(void)
{
	return g_ts_timer_enable;
}

void report_ts_xy(int x, int y)
{
//	printf("x = %08d, y = %08d\n\r", x, y);
	if(g_ts_data_valid == 0)
		{	
			g_ts_x = x;
			g_ts_y = y;
			g_ts_data_valid = 1;
		}
}

/* 读原始数据 */
void ts_read_raw(int *px, int *py)
{
	while(g_ts_data_valid == 0);/*等待读完数据*/
	
	*px = g_ts_x;
	*py = g_ts_y;
	g_ts_data_valid = 0;
}


/*
每10ms执行一次
*/
void touchsrceen_timer_irq(void)
{
	/*如果触摸屏仍被按下，进入" 自动测量模式，启动ADC "*/
	if(0 == get_status_of_ts_timer())/* 定时器中断没有使能 */
		return;

	if(ADCDAT0 & (1<<15))/*松开*/
		{
			ts_timer_disable();
			enter_wait_pen_down_mode();
			return;
		}
	else/*按下*/
		{
			enter_auto_measure_mode();
			/* 启动ADC */
			ADCCON |= (ENABLE_START_1<<0);
		}
	
}


/* 获取 x/y 坐标*/
void Isr_Adc(void)
{
	int x = ADCDAT0;
	int y = ADCDAT1 ;

	if(!(x & (1<<15)))/* 按下 */
		{
			x &= 0x3ff;
			y &= 0x3ff;
			
			//printf("x = %08d, y = %08d\n\r", x, y);
			report_ts_xy(x,y);

			/* 启动定时器中断来读取数据 */
			ts_timer_enable();
		}
	else			/* 松开 */
		{
			ts_timer_disable();
			enter_wait_pen_down_mode();
		}
	
	enter_wait_pen_up_mode();
}



/*
 *ADC中断 或者 触摸屏中断服务函数
 */
void AdcTsIntHandle(int irq)
{
	if (SUBSRCPND & (1<<TC_INT_BIT))  /* 如果是触摸屏中断 */
		Isr_Tc();

	if (SUBSRCPND & (1<<ADC_INT_BIT))  /* 如果是ADC中断，就测量坐标 */
		Isr_Adc();
	
	SUBSRCPND = (1<<TC_INT_BIT) | (1<<ADC_INT_BIT);/* 清中断标志 */
}


/*
 *ADC中断 或者 触摸屏中断初始化函数
 */
void adc_ts_int_init(void)
{
	SUBSRCPND = (1<<TC_INT_BIT) | (1<<ADC_INT_BIT);

	/* 注册ADC中断处理函数 */
	register_irq(INT_ADC_TC, AdcTsIntHandle);

	/* 使能中断 */
	INTSUBMSK &= ~((1 << ADC_INT_BIT) | (1 << TC_INT_BIT));
		
}

/* 设置触摸屏接口寄存器 */
void adc_ts_reg_init(void)
{
	/* [15] : ECFLG,  1 = End of A/D conversion
	 * [14] : PRSCEN, 1 = A/D converter prescaler enable
	 * [13:6]: PRSCVL, adc clk = PCLK / (PRSCVL + 1)
	 * [5:3] : SEL_MUX, 000 = AIN 0
	 * [2]	 : STDBM
	 * [0]	 : 1 = A/D conversion starts and this bit is cleared after the startup.
	 */
	ADCCON = (PRSCEN_1<<14) | (PRSCVL<<6) | (AIN_0<<3);

	/*
	按下触摸屏，延时一会再发出TC中断，
	延时时间 = ADCDLY * 晶振周期 = ADCDLY * 1 / 12000000 = 5ms
	-> ADCDLY = 60000
	*/
	ADCDLY = 60000;	/* 设为默认值为0xff，值太小了 */
}



/* 触摸屏初始化 */
void touchscreen_init(void)
{
	/* 设置触摸屏寄存器 */
	adc_ts_reg_init();

	/* 设置中断 */
	adc_ts_int_init();

	/* 注册定时器中断函数 */
	register_timer("touchsrceen_timer",touchsrceen_timer_irq);

	
	/* 让触摸屏控制器进入"等待中断模式" */
	enter_wait_pen_down_mode();
	
}







