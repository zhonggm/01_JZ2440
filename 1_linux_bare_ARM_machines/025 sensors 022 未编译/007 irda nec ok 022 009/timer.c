
#include "s3c2440_soc.h"
#include "timer.h"

#define TIMER_NUM 32
#define NULL ((void *)0)


timer_desc timer_arry[TIMER_NUM];
static unsigned long long g_system_time_10ms_cnt = 0;/* 系统时间10ms计数值 */

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


/* 遍历每个fp，如果不是空，就执行这个函数 */
void timer_irq(void)
{
	int i;
	
	g_system_time_10ms_cnt++;

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
	/* 设置TIMER0的时钟 */
	/* Timer clk = PCLK / {prescaler value+1} / {divider value} 
				 = 50000000/(4+1)/2
				 = 5000000(Hz)
	 */
	TCFG0 = 4;  /* Prescaler 0 = 49, 用于timer0,1的预分频 */
	TCFG1 &= ~0xf;
	
	/* 设置TIMER0的初值 */
	TCNTB0 = 50000;  /* 1s计数62500，计625就用10ms，所以是10ms中断一次 */

	/* 加载初值, 启动timer0 */
	TCON |= (1<<1);   /*  Manual Update from TCNTB0 & TCMPB0 */

	/* 设置为自动加载并启动 */
	TCON &= ~(1<<1);         /* 因为接下来还要写TCON，所以要清零*/
	TCON |= (1<<0) | (1<<3);  /* bit0: start, bit3: auto reload */

	/* 设置中断 */
	register_irq(10, timer_irq);
}

/* 尽量少调用函数 */
void udelay(int n)
{
	int cnt = n * 5;  /* u us 对应n*5个计数值 */
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
		mdelay(60000); /* 延时1分钟 */
		printf("%d\n\r", ++cnt);
	}
}

unsigned long long get_system_time_us(void)
{
	unsigned long long us = (50000 - TCNTO0)/5;/* 计数每减少1用时0.2us */
	return g_system_time_10ms_cnt * 10 * 1000 + us;/*返回系统时间总us数*/
}

unsigned int delta_time_us(unsigned long long pre, unsigned long long now)
{
	return (now - pre);
}

