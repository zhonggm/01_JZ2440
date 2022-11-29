
#include "s3c2440_soc.h"
#include "nand_flash.h"


void sdram_init(void)
{
	BWSCON = 0x22000000;

	BANKCON6 = 0x18001;
	BANKCON7 = 0x18001;

	REFRESH  = 0x8404f5;

	BANKSIZE = 0xb1;

	MRSRB6   = 0x20;
	MRSRB7   = 0x20;
}

int sdram_test(void)
{
	volatile unsigned char *p = (volatile unsigned char *)0x30000000;
	int i;

	// write sdram
	for (i = 0; i < 1000; i++)
		p[i] = 0x55;

	// read sdram
	for (i = 0; i < 1000; i++)
		if (p[i] != 0x55)
			return -1;

	return 0;
}

/*判断启动方式*/
int isBootFromNorFlash(void)
{
	volatile unsigned int *p = (volatile unsigned int *)0;/*赋地址0*/
	unsigned int val = *p;

	*p = 0x12345678;/* 把0x12345678写到0地址处的内存*/
	if (*p == 0x12345678)/* 读0地址处的内存的内容是否为0x12345678*/
	{
		/* 写成功, 对应nand启动 */
		*p = val;
		return 0;
	}
	else
	{
		return 1;
	}
}


void copy2sdram(void)
{
	/* 要从lds文件中获得 __code_start, __bss_start
	 * 然后从0地址把数据复制到从__code_start开始到__bss_start前一单元结束的内存中
	 */

	extern int __code_start, __bss_start;

	volatile unsigned int *dest = (volatile unsigned int *)&__code_start;
	volatile unsigned int *end = (volatile unsigned int *)&__bss_start;
	volatile unsigned int *src = (volatile unsigned int *)0;
	int len;
	
	len = ((int)&__bss_start - (int)&__code_start);
	if(isBootFromNorFlash())
		{	
			/*Nor启动*/
			while (dest < end)
			{
				*dest++ = *src++;
			}
		}
	else
		{
			/*Nand启动*/
			nand_init();
			nand_read(src, dest, len);
		}
}



void clean_bss(void)
{
	/* 要从lds文件中获得 __bss_start, _end */
	extern int _end, __bss_start;

	volatile unsigned int *start = (volatile unsigned int *)&__bss_start;
	volatile unsigned int *end = (volatile unsigned int *)&_end;

	while (start <= end)
	{
		*start++ = 0;
	}
}










