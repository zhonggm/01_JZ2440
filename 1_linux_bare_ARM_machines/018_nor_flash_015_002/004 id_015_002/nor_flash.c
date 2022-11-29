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
	int manufacturer_id, device_id;
	char str[4];
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

	str[0] = nor_dat(0x10);/*�õ�'Q'*/
	str[1] = nor_dat(0x11);/*�õ�'R'*/
	str[2] = nor_dat(0x12);/*�õ�'Y'*/
	str[3] = '\0';

	printf("str = %s\n\r", str);

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



















