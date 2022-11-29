#include "my_printf.h"
#include "string_utils.h"




#define NOR_FLASH_BASE 0/*  nor flash ��Ƭѡ�ӵ���nGCS0�����Ի���ַ��0 */



/* ����:   55H 98 
 * ������: ��(0 + (0x55)<<1)д��0x98
 */
void nor_write_word(unsigned int base_addr, unsigned int  nor_addr, unsigned int  val)
{
	volatile unsigned short *p = (volatile unsigned short *)(base_addr + (nor_addr<<1));
	*p = val;
}

void nor_cmd(unsigned int  nor_addr, unsigned int  cmd)
{
	nor_write_word(NOR_FLASH_BASE, nor_addr, cmd);
}

unsigned int nor_read_word(unsigned int base_addr, unsigned int  nor_addr)
{	
	volatile unsigned short *p = (volatile unsigned short *)(base_addr + (nor_addr<<1));
	
	return *p;
}

unsigned int  nor_dat(unsigned int  nor_addr)
{
	return nor_read_word(NOR_FLASH_BASE, nor_addr);
}






void do_scan_nor_flash(void)
{
	char str[4];
	unsigned int size;

	/* ��ӡ����ID���豸ID */
	/* ����CFIģʽ */
	nor_cmd(0x55, 0x98); 

	str[0] = nor_dat(0x10);/*�õ�'Q'*/
	str[1] = nor_dat(0x11);/*�õ�'R'*/
	str[2] = nor_dat(0x12);/*�õ�'Y'*/
	str[3] = '\0';

	printf("str = %s\n\r", str);

	/* ��ӡ���� */
	size = 1<<(nor_dat(0x27));
	printf("nor flash size = 0x%x, %dM\n\r", size, size / (1024*1024));	
	




	/* ��ӡ������������ʼ��ַ */



	/* �˳�CFIģʽ */
	nor_cmd(0, 0xF0);
	
}



void do_erase_nor_flash(void)
{

}


void do_write_nor_flash(void)
{
	
}


void do_read_nor_flash(void)
{
	
}



void nor_flash_test(void)
{
	char c;

	while(1)
	{
		/* ��ӡ�˵���������ѡ��������� */
		printf("[s] Scan nor flash\n\r");
		printf("[e] Erase nor flash\n\r");
		printf("[w] Write nor flash\n\r");
		printf("[r] Read nor flash\n\r");
		printf("[q] quit\n\r");
		printf("Enter selection: ");

		c = getchar();/* ����������Ļ���� */
		printf("%c\n\r", c);

		/* ��������:
		 * 1. ʶ��nor flash
		 * 2. ����nor flashĳ������
		 * 3. ��дĳ����ַ
		 * 4. ��ĳ����ַ
		 */
		switch(c)
			{
				case 's':
				case 'S':
				{
					do_scan_nor_flash();
					break;
				}
				case 'e':
				case 'E':
				{
					do_erase_nor_flash();
					return;
					break;
				}
				case 'w':
				case 'W':
				{
					do_write_nor_flash();
					return;
					break;
				}
				case 'r':
				case 'R':
				{
					do_read_nor_flash();
					return;
					break;
				}
				case 'q':
				case 'Q':
				{
					return;
					break;
				}
			}


		
	}

	
	 
	
}



















