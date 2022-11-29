


#define LCD_CONTROLLER_NUM 10

static p_lcd_controller p_array_lcd_controller[LCD_CONTROLLER_NUM];
static p_lcd_controller g_p_lcd_controller_selected;
/*
LCD����������ע�ắ��
*/
int register_lcd_controller(p_lcd_controller plcdcon)
{
	int i;

	for(i = 0; i < LCD_CONTROLLER_NUM; i++)
		{
			if(!p_array_lcd_controller[i])
				{
					p_array_lcd_controller[i] = plcdcon;

					return i;
				}
		}

	return -1;
}


/*
������ѡ��LCD����������
*/
int select_lcd_controller(char *name)
{
	int i;

	for(i = 0; i < LCD_CONTROLLER_NUM; i++)
		{
			if(p_array_lcd_controller[i] && (!strcmp(p_array_lcd_controller[i]->name, name)))
				{
					g_p_lcd_controller_selected = p_array_lcd_controller[i];
					return i;
				}
		}

	return -1;
}



/* �������µ�����:
 * ����: ���ղ�ͬLCD�Ĳ���
 * ����: ʹ����Щ�������ö�Ӧ��LCD������
 */

int lcd_controller_init(p_lcd_params plcd)
{
	#if 0
	/* ����2440��LCD�������ĳ�ʼ������ */
	lcd_controller.init(plcdparams);
	#endif

	/* ������ѡ���LCD�������ĳ�ʼ������ */
	if(g_p_lcd_controller_selected)
		{
			g_p_lcd_controller_selected->init(plcd);
			return 0;
		}

	return -1;
}

void lcd_controller_add(void)
{
	s3c2440_lcd_controller_add();
}


