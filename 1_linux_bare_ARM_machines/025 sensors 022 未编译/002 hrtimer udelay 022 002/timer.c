
#include "s3c2440_soc.h"
#include "timer.h"

#define TIMER_NUM 32
#define NULL ((void *)0)


timer_desc timer_arry[TIMER_NUM];

int register_timer(char *name, timer_func fp)
{
	int i;

	for(i = 0; i < TIMER_NUM; i++)
		{
			if(!timer_arry[i].fp)
				{
					timer_arry[i].name = name;
					timer_arry[i].fp = fp;
					return 0;
				}
		}
	
	return -1;
}


void unregister_timer(char *name)
{
	int i;

	for(i = 0; i < TIMER_NUM; i++)
		{
			if(!strcmp(timer_arry[i].name, name))
				{
					timer_arry[i].name = NULL;
					timer_arry[i].fp = NULL;
				}
		}
}


/* ����ÿ��fp��������ǿգ���ִ��������� */
void timer_irq(void)
{
	int i;

	for(i = 0; i < TIMER_NUM; i++)
		{
			if(timer_arry[i].fp)
				{
					timer_arry[i].fp();
				}
		}
}

void timer_init(void)
{
	/* ����TIMER0��ʱ�� */
	/* Timer clk = PCLK / {prescaler value+1} / {divider value} 
				 = 50000000/(4+1)/2
				 = 5000000(Hz)
	 */
	TCFG0 = 4;  /* Prescaler 0 = 49, ����timer0,1��Ԥ��Ƶ */
	TCFG1 &= ~0xf;
	
	/* ����TIMER0�ĳ�ֵ */
	TCNTB0 = 50000;  /* 1s����62500����625����10ms��������10ms�ж�һ�� */

	/* ���س�ֵ, ����timer0 */
	TCON |= (1<<1);   /*  Manual Update from TCNTB0 & TCMPB0 */

	/* ����Ϊ�Զ����ز����� */
	TCON &= ~(1<<1);         /* ��Ϊ��������ҪдTCON������Ҫ����*/
	TCON |= (1<<0) | (1<<3);  /* bit0: start, bit3: auto reload */

	/* �����ж� */
	register_irq(10, timer_irq);
}

/* �����ٵ��ú��� */
void udelay(int n)
{
	int cnt = n * 5;  /* u us ��Ӧn*5������ֵ */
	int pre = TCNTO0;
	int cur;
	int delta;

	while (cnt > 0)
	{
		cur = TCNTO0;
		if (cur <= pre)
			delta = pre - cur;
		else
			delta = pre + (50000 - cur);

		cnt = cnt - delta;
		pre = cur;
	}
}

void mdelay(int m)
{
	udelay(m*1000);
}

void hrtimer_test(void)
{
	int cnt = 0;
	while (1)
	{
		printf("delay one min: ");
		mdelay(60000); /* ��ʱ1���� */
		printf("%d\n\r", ++cnt);
	}
}


