
#include "s3c2440_soc.h"


/* 115200,8n1 */
void uart0_init()
{
	/* 设置引脚用于串口 */
	/* GPH2,3用于TxD0, RxD0 */
	GPHCON &= ~((3<<4) | (3<<6));
	GPHCON |= ((2<<4) | (2<<6));

	GPHUP &= ~((1<<2) | (1<<3));  /* 使能内部上拉 */
	

	/* 设置波特率 */
	/* UBRDIVn = (int)( UART clock / ( buad rate x 16) ) –1
	 *  UART clock = 50M
	 *  UBRDIVn = (int)( 50000000 / ( 115200 x 16) ) –1 = 26
	 */
	UCON0 = 0x00000005; /* PCLK,中断/查询模式 */
	UBRDIV0 = 26;

	/* 设置数据格式 */
	ULCON0 = 0x00000003; /* 8n1: 8个数据位, 无较验位, 1个停止位 */

	/*  */

}

int putchar(int c)
{
	/* UTRSTAT0 */
	/* UTXH0 */

	while (!(UTRSTAT0 & (1<<2)));
	UTXH0 = (unsigned char)c;
	
}

int getchar(void)
{
	while (!(UTRSTAT0 & (1<<0)));
	return URXH0;
}

int puts(const char *s)
{
	while (*s)
	{
		putchar(*s);
		s++;
	}
}



/*0xABCDEF12*/
void printHex(unsigned int val)
{
	int i;
	unsigned char arr[8];//每个元素保存4位，共32位

	/*先取出每一位的值*/
	for(i = 0; i < 8; i++)
		{
			arr[i] = val & 0xf;/*每次取低4位 */
			val >>= 4;/* arr[0] = 0x2, arr[0] = 0x1, arr[0] = 0xF, arr[0] = 0xE */
		}

	/* 打印 */
	puts("0x");
	for(i = 7; i >= 0; i--)
		{
			if(arr[i] >= 0 && arr[i] <= 9)
				putchar(arr[i]+ '0');
			else if(arr[i] >= 0xA && arr[i] <= 0xF)
				putchar(arr[i] - 0xA + 'A');
		}
	
}


