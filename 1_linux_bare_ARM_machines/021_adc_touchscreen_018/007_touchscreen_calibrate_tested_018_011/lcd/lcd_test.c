
#include "geometry.h"
#include "framebuffer.h"
#include "font.h"

#include "../led.h"


/* 简单测试LCD:往framebuffer写数据，看能不能正确显示。*/

void lcd_test(void)
{
	unsigned int fb_base;
	int xres, yres, bpp;
	int x, y;
	unsigned char *p_8;
	unsigned short *p_16;
	unsigned int *p_32;

	/* 初始化LCD */
	lcd_init();

	/* 使能LCD */
	lcd_enable();

	/*获取所选的LCD参数中的: framebuffer基地址、x分辨率、y分辨率、bpp*/
	get_lcd_params(&fb_base, &xres, &yres, &bpp);
	fb_get_lcd_params();
	font_init();
	
	/*往 framebuffer中写颜色数据*/
	if(8 == bpp)
		{
			/* bpp: palette[12], 共256项 */
			p_8 = (unsigned char *)fb_base;
			for(x = 0; x < xres; x++)
				for(y = 0; y < yres; y++)
					*p_8++ = 12;

			/* bpp: palette[100] */
			p_8 = (unsigned char *)fb_base;
			for(x = 0; x < xres; x++)
				for(y = 0; y < yres; y++)
					*p_8++ = 100;
				
			/* bpp: palette[200] */
			p_8 = (unsigned char *)fb_base;
			for(x = 0; x < xres; x++)
				for(y = 0; y < yres; y++)
					*p_8++ = 200;

			/* bpp: palette[0] */ 
			p_8 = (unsigned char *)fb_base;
			for(x = 0; x < xres; x++)
				for(y = 0; y < yres; y++)
					*p_8++ = 0;
		}
	else if(16 == bpp)
		{
			/*565 红色 0xf800*/
			p_16 = (unsigned short *)fb_base;
			for(x = 0; x < xres; x++)
				for(y = 0; y < yres; y++)
					*p_16++ = 0xf800;

			/*565 绿色 0x000007e0*/
			p_16 = (unsigned short *)fb_base;
			for(x = 0; x < xres; x++)
				for(y = 0; y < yres; y++)
					*p_16++ = 0x07e0;
				
			/*565 蓝色 0xf800*/  
			p_16 = (unsigned short *)fb_base;
			for(x = 0; x < xres; x++)
				for(y = 0; y < yres; y++)
					*p_16++ = 0x001f;

			/*565 黑色 0x0000*/  
			p_16 = (unsigned short *)fb_base;
			for(x = 0; x < xres; x++)
				for(y = 0; y < yres; y++)
					*p_16++ = 0;
		}
	else if(32 == bpp)
		{
			/* 红色 0xff0000*/
			p_32 = (unsigned int *)fb_base;
			for(x = 0; x < xres; x++)
				for(y = 0; y < yres; y++)
					*p_32++ = 0xff0000;

			/* 绿色 0x00ff00*/
			p_32 = (unsigned int *)fb_base;
			for(x = 0; x < xres; x++)
				for(y = 0; y < yres; y++)
					*p_32++ = 0x00ff00;
				
			/* 蓝色 0x0000ff*/
			p_32 = (unsigned int *)fb_base;
			for(x = 0; x < xres; x++)
				for(y = 0; y < yres; y++)
					*p_32++ = 0x0000ff;

			/* 黑色 0x000000*/
			p_32 = (unsigned int *)fb_base;
			for(x = 0; x < xres; x++)
				for(y = 0; y < yres; y++)
					*p_32++ = 0;
		}

	delay(1000000);

	/* 画线 */
	draw_line(0, 0, xres - 1, 0, 0xff00c2);//
	draw_line(xres - 1, 0, xres - 1, yres - 1, 0xffffd9);//
	draw_line(0, yres - 1, xres - 1, yres - 1, 0xff00aa);//
	draw_line(0, 0, 0, yres - 1, 0xff00ef);//
	draw_line(0, 0, xres - 1, yres - 1, 0xff45e8);//
	draw_line(xres - 1, 0, 0, yres - 1, 0xff07ff);//

	delay(1000000);

	/* 画圆 */
	draw_circle(xres/2, yres/2, yres/4, 0xff);

	fb_print_string(10, 10, "Zhong is good!\n\rYou are NO. 1.", 0xff);
}




