
#include "s3c2440_soc.h"

void delay(volatile int d)
{
	while (d--);
}

int led_init(void)
{

	/* 设置GPFCON让GPF4/5/6配置为输出引脚 */
	GPFCON &= ~((3<<8) | (3<<10) | (3<<12));
	GPFCON |=  ((1<<8) | (1<<10) | (1<<12));
}



