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


void Isr_Tc(void)
{
	printf("ADCUPDN = 0x%x, ADCDAT0 = 0x%x, ADCDAT1 = 0x%x, ADCTSC = 0x%x\n\r", ADCUPDN, ADCDAT0, ADCDAT1, ADCTSC);
	
	if (ADCDAT0 & (1<<15))
	{
		printf("pen up\n\r");
		enter_wait_pen_down_mode();
	}
	else	
	{
		printf("pen down\n\r");
		/* 进入"等待触摸笔松开的模式" */
		enter_wait_pen_up_mode();
	}
}


/*
 *ADC中断 或者 触摸屏中断服务函数
 */
 
void AdcTsIntHandle(int irq)
{
	if (SUBSRCPND & (1<<TC_INT_BIT))  /* 如果是触摸屏中断 */
		Isr_Tc();

//	if (SUBSRCPND & (1<<ADC_INT_BIT))  /* 如果是ADC中断 */
//		Isr_Adc();
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
	ADCDLY = 0xff;	/* 设为默认值 */
}




/* 触摸屏初始化 */
void touchscreen_init(void)
{
	/* 设置触摸屏寄存器 */
	adc_ts_reg_init();

	/* 设置中断 */
	adc_ts_int_init();


	/* 让触摸屏控制器进入"等待中断模式" */
	enter_wait_pen_down_mode();
	
}







