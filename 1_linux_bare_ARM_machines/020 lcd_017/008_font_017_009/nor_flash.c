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
	
	
	/* ��ӡ����ID���豸ID */
	nor_cmd(0x555, 0xAA);  /* ���� */
	nor_cmd(0x2AA, 0x55);
	nor_cmd(0x555, 0x90);
	manufacturer_id = nor_dat(0x00);/*�õ�����ID*/
	device_id = nor_dat(0x01); /*�õ��豸ID*/
	nor_cmd(0, 0xf0);/* Reset */
	
	/* ����CFIģʽ */
	nor_cmd(0x55, 0x98); 

	qry[0] = nor_dat(0x10);/*�õ�'Q'*/
	qry[1] = nor_dat(0x11);/*�õ�'R'*/
	qry[2] = nor_dat(0x12);/*�õ�'Y'*/
	qry[3] = '\0';

	printf("qry = %s\n\r", qry);

	/* ��ӡ���� ��0x27����ȡ */
	size = 1<<(nor_dat(0x27));
	printf("Manufacturer ID = 0x%x, Device ID = 0x%x, nor flash size = 0x%x, %dM\n\r", manufacturer_id, device_id, size, size / (1024*1024));	
	
	/* ��ӡ������������ʼ��ַ */
	/* ���ʽ���:
	 *    erase block region : ���溬��1������block, ���ǵĴ�Сһ��
	 * һ��nor flash����1������region
	 * һ��region����1������block(����)

	 * Erase block region information:
	 *    ǰ(��)1�ֽ� + 1  : ��ʾ��region�ж��ٸ�block 
	 *    ��(��)1�ֽ�*256  : ��ʾblock�Ĵ�С
	 */
	regions = nor_dat(0x2c);/*Number of erase regions within device*/
	region_info_base = 0x2d;
	block_addr = 0;
	block_index = 0;
	printf("Block/Sector start Address:\n\r");
	for(i = 0; i < regions; i++)
		{
			/* �õ���ǰregion��block�� */
			blocks = 1 + nor_dat(region_info_base) + (nor_dat(region_info_base + 1) << 8);
			/* �õ���ǰregion��ÿ��block�Ĵ�С */
			block_size = (nor_dat(region_info_base + 2) + (nor_dat(region_info_base + 3) << 8)) * 256;
			/* ��ӡ��ǰregion��ÿ��block����ʼ��ַ */
			//printf("\n\rregion %d, blocks = %d, block_size = 0x%x, block_addr = 0x%x\n\r", i, blocks, block_size, block_addr);//debug
			for(j = 0; j < blocks; j++)
				{
					printHex(block_addr);
					putchar(' '); 
					block_addr += block_size;
					block_index++;
					if(block_index % 5 == 0)/* ÿ�д�ӡ5��block����ʼ��ַ */
						printf("\n\r");
				}
			
			/* �õ��洢�¸�region��Ϣ�ĵ�ַ */
			region_info_base += 4;
		}

	printf("\n\r");

	/* �˳�CFIģʽ */
	nor_cmd(0, 0xF0);
	
}




void do_erase_nor_flash(void)
{
	unsigned int start_erase_addr;/* CPU �Ƕ�*/

	/* ��ȡ��ַ */
	printf("Enter the address of sector to erase: ");
	start_erase_addr = get_uint();

	printf("erasing......\n\r ");
	nor_cmd(0x555, 0xAA);  /* ���� */
	nor_cmd(0x2AA, 0x55);
	nor_cmd(0x555, 0x80); /* Sector Erase */

	nor_cmd(0x555, 0xAA);  /* ���� */
	nor_cmd(0x2AA, 0x55);
	nor_cmd(start_erase_addr >> 1, 0x30); /* ����������ַ */
	wait_done(start_erase_addr);
}


void do_write_nor_flash(void)
{
	unsigned int   start_write_addr;/* CPU �Ƕ�*/
	unsigned char the_string_to_write[100];
	int i, j;
	unsigned int val;
	
	/* ��ȡ��ַ */
	printf("Enter the address of sector to write: ");
	start_write_addr = get_uint();

	printf("Enter the string to write: ");
	gets(the_string_to_write );/*��ȡҪд������*/

	printf("Writing... \n\r ");


	/*
	����д�룬����Ҫ���ַ���������ÿ����Ԫ�����16λ���֣�
	str[0],str[1]==>16bit
	str[2],str[3]==>16bit
	*/
	i = 0;
	j = 1;

	while(the_string_to_write[i] && the_string_to_write[j])
		{
			val = the_string_to_write[i] + (the_string_to_write[j] << 8);
			nor_cmd(0x555, 0xAA);  /* ���� */
			nor_cmd(0x2AA, 0x55);
			nor_cmd(0x555, 0xA0); /* Program */
			nor_cmd(start_write_addr>>1, val); /* Program */
			/* �ȴ���д��� : ������, Q6�ޱ仯ʱ��ʾ���� */
			wait_done(start_write_addr);

			i += 2;
			j += 2;
			start_write_addr += 2;
		}

	/*
	������ʣ�µ��������Ƕ�Ϊ��������:
	the_string_to_write[i] != 0, the_string_to_write[j] == 0, 
	val = the_string_to_write[i] + the_string_to_write[j] << 8;

	the_string_to_write[i] == 0,�Ѿ��ǽ�������
	val = the_string_to_write[i] + 0 << 8;

	the_string_to_write[i] == 0, the_string_to_write[j] == 0,
	val = the_string_to_write[i] + 0 << 8;
	*/
	val = the_string_to_write[i];
	nor_cmd(0x555, 0xAA);  /* ���� */
	nor_cmd(0x2AA, 0x55);
	nor_cmd(0x555, 0xA0); /* Program */
	nor_cmd(start_write_addr>>1, val); /* Program */
	/* �ȴ���д��� : ������, Q6�ޱ仯ʱ��ʾ���� */
	wait_done(start_write_addr);
}


/* ����nor flash�����ݣ���ת��������ұ���ʾ���� */
void do_read_nor_flash(void)
{
	unsigned int addr;
	unsigned char c;
	int i, j;
	volatile unsigned char *p;
	unsigned char str[16];
	
	/* ��ʾ�û�����Ҫ�����ݵĵ�ַ */
	printf("Enter the address to read: " );
	addr = get_uint();/* �Ӵ��ڻ�õ�ַ */

	p = (volatile unsigned char *)addr;

	printf("Data : \n\r");
	/* ����ȡ�����ݳ��ȶ�Ϊ64�� */
	for(i = 0; i < 4; i++)/*��4�д�ӡ*/
		{
			/*�ȴ�ӡ����(ASCII)*/
			for(j = 0;  j < 16;  j++)/*ÿ�д�ӡ16������*/
				{
					c = *p++;
					str[j] = c;
					printf("%02x ", c);
				}
			
			printf("  ;");

			/*�����ұߴ�ӡ�ַ�*/
			for(j = 0;  j < 16;  j++)/*ÿ�д�ӡ16�����ݶ�Ӧ���ַ�*/
				{
					/* �ɴ�ӡ�ַ���0x20-0x7E��*/
					if((str[j] < 0x20)  ||  (str[j] > 0x7E))
						putchar('.');/*������*/
					else
						putchar(str[j]);/*����*/
				}
			printf("\n\r");	
		}
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



















