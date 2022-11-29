
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




