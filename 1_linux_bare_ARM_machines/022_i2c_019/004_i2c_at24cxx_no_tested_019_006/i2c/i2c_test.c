


void do_write_at24cxx(void)
{
	unsigned int   start_write_addr;/* CPU �Ƕ�*/
	unsigned char the_string_to_write[100];
	int err;
	
	/* ��ȡ��ַ */
	printf("Enter the address of sector to write: ");
	start_write_addr = get_uint();

	if(start_write_addr > 256)
		{
			printf("address > 256, error!\n\r");
			return;
		}
	
	printf("Enter the string to write: ");
	gets(the_string_to_write );/*��ȡҪд������*/

	printf("Writing... \n\r ");
	err = at24cxx_write(start_write_addr, the_string_to_write, strlen(the_string_to_write) + 1);
	printf("at24cxx_write ret = %d\n\r", err);
	
}


/* ����nor flash�����ݣ���ת��������ұ���ʾ���� */
void do_read_at24cxx(void)
{
	unsigned int addr;
	unsigned char c;
	int i, j, cnt;
	unsigned char data[100];
	unsigned char str[16];
	int len, err;
	
	/* ��ʾ�û�����Ҫ�����ݵĵ�ַ */
	printf("Enter the address to read: " );
	addr = get_uint();/* �Ӵ��ڻ�õ�ַ */

	p = (volatile unsigned char *)addr;

	if(addr > 256)
		{
			printf("address > 256, error!\n\r");
			return;
		}
	/* ��ʾ�û�����Ҫ�����ݵĳ��� */
	printf("Enter the length to read: " );
	len = get_int();
	
	err = at24cxx_read(addr, data, len);	
	printf("at24cxx_read ret = %d\n\r", err);

	printf("Data : \n\r");
	/* ����ȡ�����ݳ��ȶ�Ϊ64�� */
	for(i = 0; i < 4; i++)/*��4�д�ӡ*/
		{
			/*�ȴ�ӡ����(ASCII)*/
			for(j = 0;  j < 16;  j++)/*ÿ�д�ӡ16������*/
				{
					c = data[cnt++];
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



void i2c_test(void)
{
	char c;
	
	/* ��ʼ�� : ѡ��I2C������ */
	i2c_init();
	
	/* �ṩ�˵������� */
	while(1)
	{
		/* ��ӡ�˵���������ѡ��������� */
		printf("[w] Write AT24Cxx\n\r");
		printf("[r] Read AT24Cxx\n\r");
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
				case 'w':
				case 'W':
				{
					do_write_at24cxx();
					break;
				}
				case 'r':
				case 'R':
				{
					do_read_at24cxx();
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













