#include "../s3c2440_soc.h"
#include "../interrupt.h"

#include "touchscreen.h"
#include "adc.h"


#define ADC_INT_BIT (10)
#define TC_INT_BIT  (9)
#define INT_ADC_TC   (31)


/* ADCTSC�Ĵ���λ */
#define DETECT_UP    (1<<8)/* 1����ɿ��ж��źţ�0��ⰴ���ж��ź� */
#define DETECT_DOWN  (0<<8)/* 1����ɿ��ж��źţ�0��ⰴ���ж��ź� */

#define YM_ENABLE   (1<<7)/* 1YM enable��0YM disable */
#define YM_DISABLE  (0<<7)

#define YP_ENABLE   (0<<6)/* 0YP enable��1YP disable */
#define YP_DISABLE  (1<<6)/* 0YP enable��1YP disable */

#define XM_ENABLE   (1<<5)/* 1XM enable��0XM disable */
#define XM_DISABLE  (0<<5)/* 1XM enable��0XM disable */

#define XP_ENABLE   (0<<4)/* 0XP enable��1XP disable */
#define XP_DISABLE  (1<<4)/* 0XP enable��1XP disable */

#define XP_PULL_UP_ENABLE   (0<<3)/* 0 XP_PULL_UP enable��1 XP_PULL_UP disable */
#define XP_PULL_UP_DISABLE  (1<<3)/* 0 XP_PULL_UP enable��1 XP_PULL_UP disable */


#define AUTO_PST_MODE  (1<<2)/* 1�Զ�ģʽ��0����ģʽ */
#define NORMAL_MODE    (0<<2)/* 1�Զ�ģʽ��0����ģʽ */

#define NO_OPERATION   (0<<0)/* û�в��� */
#define X_MEASUREMENT  (1<<0)/* X��������� */
#define Y_MEASUREMENT  (2<<0)/* Y��������� */
#define WAITING_FOR_INTERRUPT_MODE  (3<<0)/* �ȴ��ж�ģʽ */

static volatile g_ts_timer_enable = 0;
static int g_ts_x = 0;
static int g_ts_y = 0;
static volatile char g_ts_data_valid = 0;

/* Waiting for Interrupt Mode: �ɿ���� */
void enter_wait_pen_up_mode(void)
{
	ADCTSC = DETECT_UP | XP_PULL_UP_ENABLE | YM_ENABLE | YP_DISABLE | XP_DISABLE | XM_DISABLE | WAITING_FOR_INTERRUPT_MODE;
}


/* Waiting for Interrupt Mode: ���¼�� */
void enter_wait_pen_down_mode(void)
{
	ADCTSC = DETECT_DOWN | XP_PULL_UP_ENABLE | YM_ENABLE | YP_DISABLE | XP_DISABLE | XM_DISABLE | WAITING_FOR_INTERRUPT_MODE;
}

/* �Զ�����ģʽ */
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
		/* ����"�ȴ��������ɿ���ģʽ" */
		enter_auto_measure_mode();

		/* ����ADC */
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

/* ��ԭʼ���� */
void ts_read_raw(int *px, int *py)
{
	while(g_ts_data_valid == 0);/*�ȴ���������*/
	
	*px = g_ts_x;
	*py = g_ts_y;
	g_ts_data_valid = 0;
}


/*
ÿ10msִ��һ��
*/
void touchsrceen_timer_irq(void)
{
	/*����������Ա����£�����" �Զ�����ģʽ������ADC "*/
	if(0 == get_status_of_ts_timer())/* ��ʱ���ж�û��ʹ�� */
		return;

	if(ADCDAT0 & (1<<15))/*�ɿ�*/
		{
			ts_timer_disable();
			enter_wait_pen_down_mode();
			return;
		}
	else/*����*/
		{
			enter_auto_measure_mode();
			/* ����ADC */
			ADCCON |= (ENABLE_START_1<<0);
		}
	
}


/* ��ȡ x/y ����*/
void Isr_Adc(void)
{
	int x = ADCDAT0;
	int y = ADCDAT1 ;

	if(!(x & (1<<15)))/* ���� */
		{
			x &= 0x3ff;
			y &= 0x3ff;
			
			//printf("x = %08d, y = %08d\n\r", x, y);
			report_ts_xy(x,y);

			/* ������ʱ���ж�����ȡ���� */
			ts_timer_enable();
		}
	else			/* �ɿ� */
		{
			ts_timer_disable();
			enter_wait_pen_down_mode();
		}
	
	enter_wait_pen_up_mode();
}



/*
 *ADC�ж� ���� �������жϷ�����
 */
void AdcTsIntHandle(int irq)
{
	if (SUBSRCPND & (1<<TC_INT_BIT))  /* ����Ǵ������ж� */
		Isr_Tc();

	if (SUBSRCPND & (1<<ADC_INT_BIT))  /* �����ADC�жϣ��Ͳ������� */
		Isr_Adc();
	
	SUBSRCPND = (1<<TC_INT_BIT) | (1<<ADC_INT_BIT);/* ���жϱ�־ */
}


/*
 *ADC�ж� ���� �������жϳ�ʼ������
 */
void adc_ts_int_init(void)
{
	SUBSRCPND = (1<<TC_INT_BIT) | (1<<ADC_INT_BIT);

	/* ע��ADC�жϴ����� */
	register_irq(INT_ADC_TC, AdcTsIntHandle);

	/* ʹ���ж� */
	INTSUBMSK &= ~((1 << ADC_INT_BIT) | (1 << TC_INT_BIT));
		
}

/* ���ô������ӿڼĴ��� */
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
	���´���������ʱһ���ٷ���TC�жϣ�
	��ʱʱ�� = ADCDLY * �������� = ADCDLY * 1 / 12000000 = 5ms
	-> ADCDLY = 60000
	*/
	ADCDLY = 60000;	/* ��ΪĬ��ֵΪ0xff��ֵ̫С�� */
}



/* ��������ʼ�� */
void touchscreen_init(void)
{
	/* ���ô������Ĵ��� */
	adc_ts_reg_init();

	/* �����ж� */
	adc_ts_int_init();

	/* ע�ᶨʱ���жϺ��� */
	register_timer("touchsrceen_timer",touchsrceen_timer_irq);

	
	/* �ô���������������"�ȴ��ж�ģʽ" */
	enter_wait_pen_down_mode();
	
}







