


#define LCD_NUM 10

static p_lcd_params p_array_lcd[LCD_NUM];
static p_lcd_params g_p_lcd_selected;
/*
LCD����������ע�ắ��
*/
int register_lcd(p_lcd_params plcd)
{
	int i;

	for(i = 0; i < LCD_NUM; i++)
		{
			if(!p_array_lcd[i])
				{
					p_array_lcd[i] = plcd;

					return i;
				}
		}

	return -1;
}


/*
������ѡ��LCD����������
*/
int select_lcd(char *name)
{
	int i;

	for(i = 0; i < LCD_NUM; i++)
		{
			if(p_array_lcd[i] && (!strcmp(p_array_lcd[i]->name, name)))
				{
					g_p_lcd_selected = p_array_lcd[i];
					return i;
				}
		}

	return -1;
}

int lcd_init(void)
{
	/* ע��ȫ��Ҫ�õ���LCD */
	lcd_add();

	/* ע��ȫ��Ҫ�õ���LCD������ */
	lcd_controller_add();

	/* ѡ��ĳ��LCD��ǰ����ע������Ӧ��LCD */
	select_lcd("lcd_4.3");	

	/* ѡ��ĳ��LCD��������ǰ����ע������Ӧ��LCD������ */
	select_lcd_controller("s3c2440");
	
	/* ʹ��LCD��������ʼ��LCD������ */
	lcd_controller_init(g_p_lcd_selected);
}







