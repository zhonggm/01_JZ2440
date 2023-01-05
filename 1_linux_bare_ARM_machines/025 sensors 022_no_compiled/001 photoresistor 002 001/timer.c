
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


/* 遍历每个fp，如果不是空，就执行这个函数 */
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
	/* 设置TIMER0的时钟 */
	/* Timer clk = PCLK / {prescaler value+1} / {divider value} 
				 = 50000000/(49+1)/16
				 = 62500(Hz)
	 */
	TCFG0 = 49;  /* Prescaler 0 = 49, 用于timer0,1的预分频 */
	TCFG1 &= ~0xf;
	TCFG1 |= 3;  /* MUX0 : 1/16 */

	/* 设置TIMER0的初值 */
	TCNTB0 = 625;  /* 1s计数62500，计625就用10ms，所以是10ms中断一次 */

	/* 加载初值, 启动timer0 */
	TCON |= (1<<1);   /*  Manual Update from TCNTB0 & TCMPB0 */

	/* 设置为自动加载并启动 */
	TCON &= ~(1<<1);         /* 因为接下来还要写TCON，所以要清零*/
	TCON |= (1<<0) | (1<<3);  /* bit0: start, bit3: auto reload */

	/* 设置中断 */
	register_irq(10, timer_irq);
}



