#include "lcd.h"


#define LCD_FB_BASE 0x33c00000

//定义4.3寸参数结构体
lcd_params lcd_4_3 = {
	.name ="lcd_4.3",
	.pins_pol = {
		.de = NORMAL,    /* normal: 高电平时可以传输数据 , <2440> Figure 15-6 TFT LCD Timing Example */
		.pwren = NORMAL, /* normal: 高电平有效 */
		.vclk = NORMAL,  /* normal: 在下降沿获取数据 */
		.rgb = NORMAL,   /* normal: 高电平表示1( 正逻辑 ) */
		.hsync = INVERT, /* normal: 高脉冲有效 INVERT: 低脉冲有效 */
		.vsync = INVERT, /* normal: 高脉冲有效 INVERT: 低脉冲有效 */
		},
		
	.time_seq = {

		//取典型值
		.tvp = 10, /* vysnc脉冲宽度 */
		.tvb = 2, /* 上边黑框, Vertical Back porch */
		.tvf = 2, /* 下边黑框, Vertical Front porch */

		.thp = 41, /* hsync脉冲宽度 */
		.thb = 2, /* 左边黑框, Horizontal Back porch */
		.thf = 2, /* 右边黑框, Horizontal Front porch */

		.vclk = 9,/* MHz */
	
	},
	.xres = 480,//thd
	.yres = 272,//tvd
	.bpp = 32,//
	/*32, 16 , 不设为24, 如果设为24的话，
	在计算pixplace时又要分情况了，24pp实际占了32位<2440> P412 */
	
	/* framebuffer的地址 */
	.fb_base = LCD_FB_BASE,/* 自定义指向没有被使用的内存 */

};










