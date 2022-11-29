#include "my_printf.h"
#include "s3c2440_soc.h"


#define TACLS  0
#define TWRPH0 1 
#define TWRPH1 0


void nand_init(void)
{
	/* ����Nand Flash��ʱ�����üĴ������� */
	NFCONF = (TACLS << 12) | (TWRPH0 << 8) | (TWRPH1 << 4);

	/*��ʼ��ECC����ֹƬѡ��ʹ��Nand Flash�����������ƼĴ������� */
	NFCONT = (1 << 4) | (1 << 1) | (1 << 0);
}

/* ʹ��Nand FlashƬѡ */
void nand_select(void)
{
	NFCONT &= ~(1 << 1);	
}

/* ��ֹNand FlashƬѡ */
void nand_deselect(void)
{
	NFCONT |= (1 << 1);	
}

/* ����Nand Flash���� */
void nand_cmd(unsigned char cmd)
{
	volatile int i;
	
	NFCMD = cmd;
	for(i=0; i<10; i++);/* ��ʱ�Ա�֤�����źŵ��ȶ� */
}

/* ����Nand Flash��ַ */
void nand_addr_byte(unsigned char addr)
{
	volatile int i;
	
	NFADDR = addr;
	for(i=0; i<10; i++);/* ��ʱ�Ա�֤�����źŵ��ȶ� */
}

/* ��ȡNand Flash�ֽ����� */
unsigned char nand_data(void)
{
	return NFDATA;
}

/* �ȴ����� */
void wait_ready(void)
{
	while (!(NFSTAT & 1));/*0:busy, 1:ready*/
}


/* ��ȡNand Flash ID��Ϣ */
void nand_read_id(void)
{
	unsigned char id_msg[5] = {0};

	unsigned char page_size;
	unsigned char block_size;
	
	
	nand_select();	
	nand_cmd(0x90);
	nand_addr_byte(0x00);

	id_msg[0] = nand_data();
	id_msg[1] = nand_data();
	id_msg[2] = nand_data();
	id_msg[3] = nand_data();
	id_msg[4] = nand_data();
	nand_deselect();	

	/* ��ӡ��ID��Ϣ */
	#if 1
	/* 1st Byte */
	printf("Maker Code           = 0x%x\n\r", id_msg[0]);
	/* 2nd Byte */
	printf("Device Code          = 0x%x\n\r", id_msg[1]);
	/* 3rd Byte */
	printf("Internal Chip Number = %d\n\r", (1 << (id_msg[2] & 0x03)));
	printf("Cell Type            = %d Level Cell\n\r", (2 << ((id_msg[2] >> 2) & 0x03)));
	printf("Number of Simultaneously Programmed Pages = %d\n\r", (1 << ((id_msg[2] >> 4) & 0x03)));

	if(id_msg[2] & 0x40)
		printf("Support Interleave Program Between multiple chips.\n\r");
	else
		printf("Not Support Interleave Program Between multiple chips.\n\r");
		
	if(id_msg[2] & 0x80)
		printf("Support Cache Program.\n\r");
	else
		printf("Not Support Cache Program.\n\r");

	/* 4th Byte */
	printf("Page Size            = %d KB\n\r", (1 << (id_msg[3] & 0x03)));
	printf("Block Size           = %d KB\n\r", (64 << ((id_msg[3] >> 4) & 0x03)));
	
	/* 5th Byte */
	printf("Plane Number         = %d \n\r", (1 << ((id_msg[4] >> 2) & 0x03)));
	printf("Plane Size           = %d Mb\n\r", (64 << ((id_msg[4] >> 4) & 0x07)));
	
	#else
	
	printf("id_msg[0]   = 0x%x\n\r", id_msg[0]);
	printf("id_msg[1]   = 0x%x\n\r", id_msg[1]);
	printf("id_msg[2]   = 0x%x\n\r", id_msg[2]);
	printf("id_msg[3]   = 0x%x\n\r", id_msg[3]);
	printf("id_msg[4]   = 0x%x\n\r", id_msg[4]);

	#endif
	
}


void nand_read(unsigned int addr, unsigned char *buf, unsigned int len)
{
	int i = 0;
	int page = addr / 2048;
	int col  = addr & (2048 - 1);
	
	nand_select(); 

	while (i < len)
	{
		/* ����00h���� */
		nand_cmd(00);

		/* ������ַ */
		/* col addr */
		nand_addr_byte(     col & 0xff);
		nand_addr_byte((col>>8) & 0xff);

		/* row/page addr */
		nand_addr_byte(      page & 0xff);
		nand_addr_byte( (page>>8) & 0xff);
		nand_addr_byte((page>>16) & 0xff);

		/* ����30h���� */
		nand_cmd(0x30);

		/* �ȴ����� */
		wait_ready();

		/* ������ */
		for (; (col < 2048) && (i < len); col++)
		{
			buf[i++] = nand_data(); 		
		}
		
		if (i == len)
			break;
		
		page++; /*��һҳ*/
		col = 0;/*�ص�0��*/
	}
	
	nand_deselect();	
}


void nand_flash_test(void)
{
	char c;

	while(1)
	{
		/* ��ӡ�˵���������ѡ��������� */
		printf("[s] Scan nand flash\n\r");
		printf("[e] Erase nand flash\n\r");
		printf("[w] Write nand flash\n\r");
		printf("[r] Read nand flash\n\r");
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
					nand_read_id();
					break;
				}
				case 'e':
				case 'E':
				{
					
					break;
				}
				case 'w':
				case 'W':
				{
					
					break;
				}
				case 'r':
				case 'R':
				{
										
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









