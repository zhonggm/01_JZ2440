
#include "s3c2440_soc.h"
#include "timer.h"

void delay(volatile int d)
{
	while (d--);
}

/*每10ms该函数执行一次
 *每500ms闪烁的话，就用timer_num来增加延时
 */
void led_timer_irq(void)
{
	/* 点灯计数 */
	static int timer_num = 0;
	static int cnt = 0;
	int tmp;
	
	timer_num++;
	if(timer_num < 50)
		return;
	
	timer_num = 0;
	
	cnt++;
	tmp = ~cnt;
	tmp &= 7;
	GPFDAT &= ~(7<<4);
	GPFDAT |= (tmp<<4);
}

int led_init(void)
{
	/* 设置GPFCON让GPF4/5/6配置为输出引脚 */
	GPFCON &= ~((3<<8) | (3<<10) | (3<<12));
	GPFCON |=  ((1<<8) | (1<<10) | (1<<12));

	register_timer("led", led_timer_irq);
}



