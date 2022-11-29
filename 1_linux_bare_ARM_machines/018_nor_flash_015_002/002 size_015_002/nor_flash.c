#include "my_printf.h"
#include "string_utils.h"




#define NOR_FLASH_BASE 0/*  nor flash 的片选接的是nGCS0，所以基地址是0 */



/* 比如:   55H 98 
 * 本意是: 往(0 + (0x55)<<1)写入0x98
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

	/* 打印厂家ID、设备ID */
	/* 进入CFI模式 */
	nor_cmd(0x55, 0x98); 

	str[0] = nor_dat(0x10);/*得到'Q'*/
	str[1] = nor_dat(0x11);/*得到'R'*/
	str[2] = nor_dat(0x12);/*得到'Y'*/
	str[3] = '\0';

	printf("str = %s\n\r", str);

	/* 打印容量 */
	size = 1<<(nor_dat(0x27));
	printf("nor flash size = 0x%x, %dM\n\r", size, size / (1024*1024));	
	




	/* 打印各个扇区的起始地址 */



	/* 退出CFI模式 */
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
		/* 打印菜单，供我们选择测试内容 */
		printf("[s] Scan nor flash\n\r");
		printf("[e] Erase nor flash\n\r");
		printf("[w] Write nor flash\n\r");
		printf("[r] Read nor flash\n\r");
		printf("[q] quit\n\r");
		printf("Enter selection: ");

		c = getchar();/* 键盘输入屏幕回显 */
		printf("%c\n\r", c);

		/* 测试内容:
		 * 1. 识别nor flash
		 * 2. 擦除nor flash某个扇区
		 * 3. 编写某个地址
		 * 4. 读某个地址
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



















