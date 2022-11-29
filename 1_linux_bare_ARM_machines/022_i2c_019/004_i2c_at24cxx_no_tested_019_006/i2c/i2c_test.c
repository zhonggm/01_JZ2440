


void do_write_at24cxx(void)
{
	unsigned int   start_write_addr;/* CPU 角度*/
	unsigned char the_string_to_write[100];
	int err;
	
	/* 获取地址 */
	printf("Enter the address of sector to write: ");
	start_write_addr = get_uint();

	if(start_write_addr > 256)
		{
			printf("address > 256, error!\n\r");
			return;
		}
	
	printf("Enter the string to write: ");
	gets(the_string_to_write );/*获取要写的内容*/

	printf("Writing... \n\r ");
	err = at24cxx_write(start_write_addr, the_string_to_write, strlen(the_string_to_write) + 1);
	printf("at24cxx_write ret = %d\n\r", err);
	
}


/* 读出nor flash上数据，并转义出来在右边显示出来 */
void do_read_at24cxx(void)
{
	unsigned int addr;
	unsigned char c;
	int i, j, cnt;
	unsigned char data[100];
	unsigned char str[16];
	int len, err;
	
	/* 提示用户输入要读数据的地址 */
	printf("Enter the address to read: " );
	addr = get_uint();/* 从串口获得地址 */

	p = (volatile unsigned char *)addr;

	if(addr > 256)
		{
			printf("address > 256, error!\n\r");
			return;
		}
	/* 提示用户输入要读数据的长度 */
	printf("Enter the length to read: " );
	len = get_int();
	
	err = at24cxx_read(addr, data, len);	
	printf("at24cxx_read ret = %d\n\r", err);

	printf("Data : \n\r");
	/* 所读取的数据长度定为64个 */
	for(i = 0; i < 4; i++)/*分4行打印*/
		{
			/*先打印数据(ASCII)*/
			for(j = 0;  j < 16;  j++)/*每行打印16个数据*/
				{
					c = data[cnt++];
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



void i2c_test(void)
{
	char c;
	
	/* 初始化 : 选择I2C控制器 */
	i2c_init();
	
	/* 提供菜单来测试 */
	while(1)
	{
		/* 打印菜单，供我们选择测试内容 */
		printf("[w] Write AT24Cxx\n\r");
		printf("[r] Read AT24Cxx\n\r");
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













