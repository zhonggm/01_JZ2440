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

/*һ���ֽ�д*/
void nand_w_data(unsigned char val)
{
	NFDATA = val;
}

/* �ȴ����� */
void wait_ready(void)
{
	while (!(NFSTAT & 1));/*0:busy, 1:ready*/
}

void nand_addr(unsigned int addr)
{
	 volatile int i;
	 int col  = addr % 2048;
	 int page = addr / 2048;
	
	NFADDR = (       col & 0xff);
	for(i=0; i<10; i++);
	NFADDR = ((col >> 8) & 0xff);
	for(i=0; i<10; i++);
	
	NFADDR  =(         page & 0xff);
	for(i=0; i<10; i++);
	NFADDR  =(  (page >> 8) & 0xff);
	for(i=0; i<10; i++);
	NFADDR  =( (page >> 16) & 0xff);
	for(i=0; i<10; i++);	

}

void nand_page(unsigned int page)
{
	volatile int i;
	
	NFADDR  = page & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR  = (page >> 8) & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR  = (page >> 16) & 0xff;
	for (i = 0; i < 10; i++);	
}

void nand_col(unsigned int col)
{
	volatile int i;

	NFADDR = col & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR = (col >> 8) & 0xff;
	for (i = 0; i < 10; i++);
}


int nand_bad(unsigned int addr)
{
	unsigned int col  = 2048;
	unsigned int page = addr / 2048;
	unsigned char val;

	/* 1. ѡ�� */
	nand_select();
	
	/* 2. ����������00h */
	nand_cmd(0x00);
	
	/* 3. ������ַ(��5������) */
	nand_col(col);
	nand_page(page);
	
	/* 4. ����������30h */
	nand_cmd(0x30);
	
	/* 5. �ж�״̬ */
	wait_ready();

	/* 6. ������ */
	val = nand_data();
	
	/* 7. ȡ��ѡ�� */		
	nand_deselect();


	if (val != 0xff)
		return 1;  /* bad blcok */
	else
		return 0;
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
	int col  = addr & (2048 - 1);
	//int page = addr / 2048;
		

	while (i < len)
	{
		if (!(addr & 0x1FFFF) && nand_bad(addr)) /* һ��blockֻ�ж�һ�� */
		{
			addr += (128*1024);  /* ������ǰblock */
			continue;
		}
		nand_select(); 

		/* ����00h���� */
		nand_cmd(0x00);

		/* ������ַ */
		
		#if 1
	
		nand_addr(addr);
		
		#else
		
		/* col addr */
		nand_addr_byte(     col & 0xff);
		nand_addr_byte((col>>8) & 0xff);

		/* row/page addr */
		nand_addr_byte(      page & 0xff);
		nand_addr_byte( (page>>8) & 0xff);
		nand_addr_byte((page>>16) & 0xff);
		
		#endif
		
		/* ����30h���� */
		nand_cmd(0x30);

		/* �ȴ����� */
		wait_ready();

		/* ������ */
		for (; (col < 2048) && (i < len); col++)
		{
			buf[i++] = nand_data(); 
			addr++;
		}
		
		if (i == len)
			break;
		
		//page++; /*��һҳ*/
		col = 0;/*�ص�0��*/
		nand_deselect();
	}
}


/* Nand Flash��������� */
int nand_erase(unsigned int addr, unsigned int len)
{
	int page = addr / 2048;

	/*һ������128K�������ַ���ǿ����ʼ��ַ(128K��������)����������ͬ��*/
	if (addr & (0x1FFFF))
	{
		printf("nand_erase err, addr is not block align\n\r");
		return -1;
	}
	
	if (len & (0x1FFFF))
	{
		printf("nand_erase err, len is not block align\n\r");
		return -1;
	}
	
	nand_select(); 

	while (1)
	{
		page = addr / 2048;
		
		nand_cmd(0x60);
		
		/* row/page addr */
		nand_addr_byte(      page & 0xff);
		nand_addr_byte( (page>>8) & 0xff);
		nand_addr_byte((page>>16) & 0xff);

		nand_cmd(0xD0);

		wait_ready();

		len -= (128*1024);
		if (len == 0)
			break;
		addr += (128*1024);
	}
	
	nand_deselect(); 	
	return 0;
}

/* ��buf��len���ȸ�����д��Nand��addr��ַ��ʼ�� */
void nand_write(unsigned int addr, unsigned char *buf, unsigned int len)
{
	int page = addr / 2048;
	int col  = addr & (2048 - 1);
	int i = 0;

	nand_select(); 

	while (1)/*ÿ�����дһҳ*/
	{
		nand_cmd(0x80);

		/* ������ַ */
		/* col addr */
		nand_addr_byte(col & 0xff);
		nand_addr_byte((col>>8) & 0xff);
		
		/* row/page addr */
		nand_addr_byte(page & 0xff);
		nand_addr_byte((page>>8) & 0xff);
		nand_addr_byte((page>>16) & 0xff);

		/* �������� */
		for (; (col < 2048) && (i < len); )
		{
			nand_w_data(buf[i++]);
		}
		nand_cmd(0x10);
		wait_ready();

		if (i == len)
			break;
		else
		{
			/* ��ʼ��һ��ѭ��page */
			col = 0;
			page++;
		}
	}
	nand_deselect(); 	
}


void do_read_nand_flash(void)
{
	unsigned int addr;
	volatile unsigned char *p;
	int i, j;
	unsigned char c;
	unsigned char str[16];
	unsigned char buf[64];
	
	/* ��õ�ַ */
	printf("Enter the address to read: ");
	addr = get_uint();

	nand_read(addr, buf, 64);
	p = (volatile unsigned char *)buf;

	printf("Data : \n\r");
	/* ���ȹ̶�Ϊ64 */
	for (i = 0; i < 4; i++)
	{
		/* ÿ�д�ӡ16������ */
		for (j = 0; j < 16; j++)
		{
			/* �ȴ�ӡ��ֵ */
			c = *p++;
			str[j] = c;
			printf("%02x ", c);
		}

		printf("   ; ");

		for (j = 0; j < 16; j++)
		{
			/* ���ӡ�ַ� */
			if (str[j] < 0x20 || str[j] > 0x7e)  /* �������ַ� */
				putchar('.');
			else
				putchar(str[j]);
		}
		printf("\n\r");
	}
}


void do_erase_nand_flash(void)
{
	unsigned int addr;
	
	/* ��õ�ַ */
	printf("Enter the address of sector to erase: ");
	addr = get_uint();

	printf("erasing ...\n\r");
	nand_erase(addr, 128*1024);
}

void do_write_nand_flash(void)
{
	unsigned int addr;
	unsigned char str[100];
	int i, j;
	unsigned int val;
	
	/* ��õ�ַ */
	printf("Enter the address of sector to write: ");
	addr = get_uint();

	printf("Enter the string to write: ");
	gets(str);

	printf("writing ...\n\r");
	nand_write(addr, str, strlen(str)+1);

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
					do_erase_nand_flash();
					break;
				}
				case 'w':
				case 'W':
				{
					do_write_nand_flash();
					break;
				}
				case 'r':
				case 'R':
				{
					do_read_nand_flash();					
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









