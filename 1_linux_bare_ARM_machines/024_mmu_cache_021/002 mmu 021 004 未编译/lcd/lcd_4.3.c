#include "lcd.h"


#define LCD_FB_BASE 0x33c00000

//����4.3������ṹ��
lcd_params lcd_4_3 = {
	.name ="lcd_4.3",
	.pins_pol = {
		.de = NORMAL,    /* normal: �ߵ�ƽʱ���Դ������� , <2440> Figure 15-6 TFT LCD Timing Example */
		.pwren = NORMAL, /* normal: �ߵ�ƽ��Ч */
		.vclk = NORMAL,  /* normal: ���½��ػ�ȡ���� */
		.rgb = NORMAL,   /* normal: �ߵ�ƽ��ʾ1( ���߼� ) */
		.hsync = INVERT, /* normal: ��������Ч INVERT: ��������Ч */
		.vsync = INVERT, /* normal: ��������Ч INVERT: ��������Ч */
		},
		
	.time_seq = {

		//ȡ����ֵ
		.tvp = 10, /* vysnc������ */
		.tvb = 2, /* �ϱߺڿ�, Vertical Back porch */
		.tvf = 2, /* �±ߺڿ�, Vertical Front porch */

		.thp = 41, /* hsync������ */
		.thb = 2, /* ��ߺڿ�, Horizontal Back porch */
		.thf = 2, /* �ұߺڿ�, Horizontal Front porch */

		.vclk = 9,/* MHz */
	
	},
	.xres = 480,//thd
	.yres = 272,//tvd
	.bpp = 32,//
	/*32, 16 , ����Ϊ24, �����Ϊ24�Ļ���
	�ڼ���pixplaceʱ��Ҫ������ˣ�24ppʵ��ռ��32λ<2440> P412 */
	
	/* framebuffer�ĵ�ַ */
	.fb_base = LCD_FB_BASE,/* �Զ���ָ��û�б�ʹ�õ��ڴ� */

};










