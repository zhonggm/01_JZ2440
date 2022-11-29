
#include "adc.h"

void adc_test(void)
{
	int val;
	double vol;
	int m; /*因这里的printf不能打印浮点数，所以要拆分为 整数部分 */
	int n; /* 小数部分 */
	
	//adc_init();

	while (1)
	{
		val = adc_read(AIN_0);
		vol = (double)val/1023 * 3.3;   /* 1023----3.3v */
		m = (int)vol;   /* 3.01, m = 3 */
		vol = vol - m;  /* 小数部分: 0.01 */
		n = vol * 1000;  /* 0.01 ----> 10 */

		/* 在串口上打印 */
		printf("vol: %d.%03dv\r", m, n);  /* 3.010v */

		/* 在LCD上打印 */
		//fb_print_string();
	}
}


