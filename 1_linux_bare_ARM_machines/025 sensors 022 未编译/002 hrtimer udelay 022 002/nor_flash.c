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


void wait_done(unsigned int cpu_addr)
{
	unsigned int first;
	unsigned int second;
	

	first     = nor_dat(cpu_addr >> 1);
	second = nor_dat(cpu_addr >> 1);

	while((first & (1<<6)) != (second & (1<<6)))
	{
		first = second;
		second = nor_dat(cpu_addr >> 1);
	}
}





void do_scan_nor_flash(void)
{
	int manufacturer_id, device_id;
	char qry[4];
	unsigned int size;
	int regions, 
		i,
		region_info_base,
		block_addr,
		blocks,
		block_size,
		j,
		block_index;
	
	
	/* 打印厂家ID、设备ID */
	nor_cmd(0x555, 0xAA);  /* 解锁 */
	nor_cmd(0x2AA, 0x55);
	nor_cmd(0x555, 0x90);
	manufacturer_id = nor_dat(0x00);/*得到厂家ID*/
	device_id = nor_dat(0x01); /*得到设备ID*/
	nor_cmd(0, 0xf0);/* Reset */
	
	/* 进入CFI模式 */
	nor_cmd(0x55, 0x98); 

	qry[0] = nor_dat(0x10);/*得到'Q'*/
	qry[1] = nor_dat(0x11);/*得到'R'*/
	qry[2] = nor_dat(0x12);/*得到'Y'*/
	qry[3] = '\0';

	printf("qry = %s\n\r", qry);

	/* 打印容量 在0x27处读取 */
	size = 1<<(nor_dat(0x27));
	printf("Manufacturer ID = 0x%x, Device ID = 0x%x, nor flash size = 0x%x, %dM\n\r", manufacturer_id, device_id, size, size / (1024*1024));	
	
	/* 打印各个扇区的起始地址 */
	/* 名词解释:
	 *    erase block region : 里面含有1个或多个block, 它们的大小一样
	 * 一个nor flash含有1个或多个region
	 * 一个region含有1个或多个block(扇区)

	 * Erase block region information:
	 *    前(高)1字节 + 1  : 表示该region有多少个block 
	 *    后(低)1字节*256  : 表示block的大小
	 */
	regions = nor_dat(0x2c);/*Number of erase regions within device*/
	region_info_base = 0x2d;
	block_addr = 0;
	block_index = 0;
	printf("Block/Sector start Address:\n\r");
	for(i = 0; i < regions; i++)
		{
			/* 得到当前region中block数 */
			blocks = 1 + nor_dat(region_info_base) + (nor_dat(region_info_base + 1) << 8);
			/* 得到当前region中每个block的大小 */
			block_size = (nor_dat(region_info_base + 2) + (nor_dat(region_info_base + 3) << 8)) * 256;
			/* 打印当前region的每个block的起始地址 */
			//printf("\n\rregion %d, blocks = %d, block_size = 0x%x, block_addr = 0x%x\n\r", i, blocks, block_size, block_addr);//debug
			for(j = 0; j < blocks; j++)
				{
					printHex(block_addr);
					putchar(' '); 
					block_addr += block_size;
					block_index++;
					if(block_index % 5 == 0)/* 每行打印5个block的起始地址 */
						printf("\n\r");
				}
			
			/* 得到存储下个region信息的地址 */
			region_info_base += 4;
		}

	printf("\n\r");

	/* 退出CFI模式 */
	nor_cmd(0, 0xF0);
	
}




void do_erase_nor_flash(void)
{
	unsigned int start_erase_addr;/* CPU 角度*/

	/* 获取地址 */
	printf("Enter the address of sector to erase: ");
	start_erase_addr = get_uint();

	printf("erasing......\n\r ");
	nor_cmd(0x555, 0xAA);  /* 解锁 */
	nor_cmd(0x2AA, 0x55);
	nor_cmd(0x555, 0x80); /* Sector Erase */

	nor_cmd(0x555, 0xAA);  /* 解锁 */
	nor_cmd(0x2AA, 0x55);
	nor_cmd(start_erase_addr >> 1, 0x30); /* 发出扇区地址 */
	wait_done(start_erase_addr);
}


void do_write_nor_flash(void)
{
	unsigned int   start_write_addr;/* CPU 角度*/
	unsigned char the_string_to_write[100];
	int i, j;
	unsigned int val;
	
	/* 获取地址 */
	printf("Enter the address of sector to write: ");
	start_write_addr = get_uint();

	printf("Enter the string to write: ");
	gets(the_string_to_write );/*获取要写的内容*/

	printf("Writing... \n\r ");


	/*
	按字写入，所以要把字符串数组中每两个元素组成16位的字，
	str[0],str[1]==>16bit
	str[2],str[3]==>16bit
	*/
	i = 0;
	j = 1;

	while(the_string_to_write[i] && the_string_to_write[j])
		{
			val = the_string_to_write[i] + (the_string_to_write[j] << 8);
			nor_cmd(0x555, 0xAA);  /* 解锁 */
			nor_cmd(0x2AA, 0x55);
			nor_cmd(0x555, 0xA0); /* Program */
			nor_cmd(start_write_addr>>1, val); /* Program */
			/* 等待烧写完成 : 读数据, Q6无变化时表示结束 */
			wait_done(start_write_addr);

			i += 2;
			j += 2;
			start_write_addr += 2;
		}

	/*
	以下是剩下的两个不是都为非零的情况:
	the_string_to_write[i] != 0, the_string_to_write[j] == 0, 
	val = the_string_to_write[i] + the_string_to_write[j] << 8;

	the_string_to_write[i] == 0,已经是结束符了
	val = the_string_to_write[i] + 0 << 8;

	the_string_to_write[i] == 0, the_string_to_write[j] == 0,
	val = the_string_to_write[i] + 0 << 8;
	*/
	val = the_string_to_write[i];
	nor_cmd(0x555, 0xAA);  /* 解锁 */
	nor_cmd(0x2AA, 0x55);
	nor_cmd(0x555, 0xA0); /* Program */
	nor_cmd(start_write_addr>>1, val); /* Program */
	/* 等待烧写完成 : 读数据, Q6无变化时表示结束 */
	wait_done(start_write_addr);
}


/* 读出nor flash上数据，并转义出来在右边显示出来 */
void do_read_nor_flash(void)
{
	unsigned int addr;
	unsigned char c;
	int i, j;
	volatile unsigned char *p;
	unsigned char str[16];
	
	/* 提示用户输入要读数据的地址 */
	printf("Enter the address to read: " );
	addr = get_uint();/* 从串口获得地址 */

	p = (volatile unsigned char *)addr;

	printf("Data : \n\r");
	/* 所读取的数据长度定为64个 */
	for(i = 0; i < 4; i++)/*分4行打印*/
		{
			/*先打印数据(ASCII)*/
			for(j = 0;  j < 16;  j++)/*每行打印16个数据*/
				{
					c = *p++;
					str[j] = c;
					printf("%02x ", c);
				}
			
			printf("  ;");

			/*再在右边打印字符*/
			for(j = 0;  j < 16;  j++)/*每行打印16个数据对应的字符*/
				{
					/* 可打印字符（0x20-0x7E）*/
					if((str[j] < 0x20)  ||  (str[j] > 0x7E))
						putchar('.');/*不可视*/
					else
						putchar(str[j]);/*可视*/
				}
			printf("\n\r");	
		}
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
					break;
				}
				case 'w':
				case 'W':
				{
					do_write_nor_flash();
					break;
				}
				case 'r':
				case 'R':
				{
					do_read_nor_flash();
					
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



















