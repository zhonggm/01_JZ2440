
#include "s3c2440_soc.h"
#include "uart.h"
#include "init.h"

char g_Char = 'A';
char g_Char3 = 'a';
const char g_Char2 = 'B';
int g_A = 0;
int g_B;

int main(void)
{
	//uart0_init();
	led_init();
	interrupt_init(); /* ��ʼ���жϿ����� */
	key_eint_init(); /* ��ʼ����������Ϊ�ж�Դ */
	timer_init();
	
	

	puts("\n\rg_A = ");
	printHex(g_A);
	puts("\n\r");

	while (1)
	{
		putchar(g_Char);
		g_Char++;

		putchar(g_Char3);
		g_Char3++;
		delay(1000000);


		printHex(TCNTO0);/* ��ʱ��0�Ĺ۲�Ĵ��� */
	}

	
	return 0;
}



