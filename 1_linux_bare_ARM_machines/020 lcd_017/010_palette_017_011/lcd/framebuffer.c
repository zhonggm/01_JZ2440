
#include "lcd.h"

/* ʵ�ֻ��㹦�� */
static unsigned int fb_base;
static int xres, yres, bpp;

/* rgb : 0x00RRGGBB */
unsigned short convert32bppto16bpp(unsigned int rgb)
{
	int r = (rgb >> 16) & 0xff;
	int g = (rgb >> 8) & 0xff;
	int b =  rgb & 0xff;

	/* rgb 565 */
	r = r >> 3;/* ȡ��5λ����3λ�ӵ� */
	g = g >> 2;/* ȡ��6λ����2λ�ӵ� */
	b = b >> 3;/* ȡ��5λ����3λ�ӵ� */

	return ((r << 11) | (g << 5) | (b << 0));
}

/* ���LCD���� */
void fb_get_lcd_params(void)
{
	get_lcd_params(&fb_base,&xres,&yres,&bpp);
}


/*��λ��(x, y)�����ص�д��ɫcolor
 *color : 32bit, 0x00RRGGBB
 */
void fb_put_pixel(int x, int y, unsigned int color)
{
	unsigned char  *pc;   /*  8bppʱframebuffer��ַ */
	unsigned short *pw;  /* 16bppʱframebuffer��ַ */
	unsigned int   *pdw;   /* 32bppʱframebuffer��ַ */

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




