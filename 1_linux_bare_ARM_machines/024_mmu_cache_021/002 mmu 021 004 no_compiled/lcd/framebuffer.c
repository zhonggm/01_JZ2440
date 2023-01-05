
#include "lcd.h"

/* 实现画点功能 */
static unsigned int fb_base;
static int xres, yres, bpp;

/* rgb : 0x00RRGGBB */
unsigned short convert32bppto16bpp(unsigned int rgb)
{
	int r = (rgb >> 16) & 0xff;
	int g = (rgb >> 8) & 0xff;
	int b =  rgb & 0xff;

	/* rgb 565 */
	r = r >> 3;/* 取高5位，低3位接地 */
	g = g >> 2;/* 取高6位，低2位接地 */
	b = b >> 3;/* 取高5位，低3位接地 */

	return ((r << 11) | (g << 5) | (b << 0));
}

/* 获得LCD参数 */
void fb_get_lcd_params(void)
{
	get_lcd_params(&fb_base,&xres,&yres,&bpp);
}


/*往位置(x, y)的像素点写颜色color
 *color : 32bit, 0x00RRGGBB
 */
void fb_put_pixel(int x, int y, unsigned int color)
{
	unsigned char  *pc;   /*  8bpp时framebuffer地址 */
	unsigned short *pw;  /* 16bpp时framebuffer地址 */
	unsigned int   *pdw;   /* 32bpp时framebuffer地址 */

	unsigned int pixel_base = fb_base + (xres * bpp / 8) * y + x * (bpp / 8);

	switch(bpp)
		{
			case 8:
			{	
				pc = (unsigned char *)pixel_base;
				*pc = color;
				break;
			}
			case 16:
			{	
				pw = (unsigned short *)pixel_base;
				*pw = convert32bppto16bpp(color);
				break;
			}
			case 32:
			{	
				pdw = (unsigned int *)pixel_base;
				*pdw = color;
				break;
			}
		}
}


void clear_screen(unsigned int color)
{
	int x, y;
	unsigned char *p_8;
	unsigned short *p_16;
	unsigned int *p_32;

	/* 往framebuffer中写数据 */
	if (bpp == 8)
	{
		/* bpp: palette[color] */

		p_8 = (unsigned char *)fb_base;
		for (x = 0; x < xres; x++)
			for (y = 0; y < yres; y++)
				*p_8++ = color;
	}
	else if (bpp == 16)
	{
		/* 让LCD输出整屏的红色 */

		/* 565: 0xf800 */

		p_16 = (unsigned short *)fb_base;
		for (x = 0; x < xres; x++)
			for (y = 0; y < yres; y++)
				*p_16++ = convert32bppto16bpp(color);
			
	}
	else if (bpp == 32)
	{
		p_32 = (unsigned int *)fb_base;
		for (x = 0; x < xres; x++)
			for (y = 0; y < yres; y++)
				*p_32++ = color;
	}
}


