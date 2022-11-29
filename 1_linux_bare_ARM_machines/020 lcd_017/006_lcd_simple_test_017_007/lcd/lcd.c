#include "lcd.h"
#include "lcd_controller.h"
#include "lcd_4.3.h"

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


/******
��ȡ��ѡ��LCD�����е�:framebuffer����ַ��x�ֱ��ʡ�y�ֱ��ʡ�bpp
*******/
int get_lcd_params(unsigned int *fb_base, int *xres, int *yres, int *bpp)
{
	*fb_base = g_p_lcd_selected->fb_base;
	*xres 	 = g_p_lcd_selected->xres;
	*yres 	 = g_p_lcd_selected->yres;
	*bpp 	 = g_p_lcd_selected->bpp;
}



void lcd_add(void)
{	
	register_lcd(&lcd_4_3);
}

/* ʹ��LCD */
void lcd_enable(void)
{	
	lcd_controller_enable();
}

/* ��ֹLCD */
void lcd_disable(void)
{	
	lcd_controller_disable();
}

/* LCD��ʼ�� */
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







