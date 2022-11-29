#include "lcd.h"
#include "lcd_controller.h"
#include "lcd_4.3.h"

#define LCD_NUM 10

static p_lcd_params p_array_lcd[LCD_NUM];
static p_lcd_params g_p_lcd_selected;
/*
LCD控制器函数注册函数
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
用名字选择LCD控制器函数
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
获取所选的LCD参数中的:framebuffer基地址、x分辨率、y分辨率、bpp
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

/* 使能LCD */
void lcd_enable(void)
{	
	lcd_controller_enable();
}

/* 禁止LCD */
void lcd_disable(void)
{	
	lcd_controller_disable();
}

/* LCD初始化 */
int lcd_init(void)
{
	/* 注册全部要用到的LCD */
	lcd_add();

	/* 注册全部要用到的LCD控制器 */
	lcd_controller_add();

	/* 选择某款LCD，前提是注册了相应的LCD */
	select_lcd("lcd_4.3");	

	/* 选择某款LCD控制器，前提是注册了相应的LCD控制器 */
	select_lcd_controller("s3c2440");
	
	/* 使用LCD参数，初始化LCD控制器 */
	lcd_controller_init(g_p_lcd_selected);
}







