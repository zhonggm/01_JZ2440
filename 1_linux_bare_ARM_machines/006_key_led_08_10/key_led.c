
#include "s3c2440_soc.h"

void delay(volatile int d)
{
	while (d--);
}

int main(void)
{
	int val1, val2;
	int tmp;

	/* 配置GPFCON让GPF4/GPF5/GPF6为输出引脚 */
	GPFCON &= ~((3<<8) | (3<<10) | (3<<12));
	GPFCON |=  ((1<<8) | (1<<10) | (1<<12));

	/*配置3个按键为输入引脚，在while中不断读取
	 *GPF0(S2/EINT0)，GPF2(S3/EINT2)，GPG3(S4/EINT11)
	 */
	GPFCON &= ~((3<<0) | (3<<4));
	GPGCON &= ~((3<<6));

		
	/* 循环点亮 */
	while(1)
	{
		val1 = GPFDAT;
		val2 = GPGDAT;

		if(val1 & (1<<0))/* S2控制GPF6*/
		{//松开
			GPFDAT |= (1<<6);
		}
		else
		{//按下
			GPFDAT &= ~(1<<6);
		}
			
		if(val1 & (1<<2))/* S3控制GPF5*/
		{//松开
			GPFDAT |= (1<<5);
		}
		else
		{//按下
			GPFDAT &= ~(1<<5);
		}

		if(val2 & (1<<3))/* S4控制GPF4*/
		{//松开
			GPFDAT |= (1<<4);
		}
		else
		{//按下
			GPFDAT &= ~(1<<4);
		}
	
	}

	return 0;
}




