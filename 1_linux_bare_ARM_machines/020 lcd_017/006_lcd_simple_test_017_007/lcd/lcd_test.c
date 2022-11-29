


/* 简单测试LCD:往framebuffer写数据，看能不能正确显示。*/

void lcd_test(void)
{
	unsigned int fb_base;
	int xres, yres, bpp;
	int x, y;
	unsigned short *p;
	unsigned int *p2;

	/* 初始化LCD */
	lcd_init();

	/* 使能LCD */
	lcd_enable();

	/*获取所选的LCD参数中的: framebuffer基地址、x分辨率、y分辨率、bpp*/
	get_lcd_params(&fb_base, &xres, &yres, &bpp);

	/*往 framebuffer中写颜色数据*/
	if(16 == bpp)
		{
			/*565 红色 0xf800*/
			p = (unsigned short *)fb_base;
			for(x = 0; x < xres; x++)
				for(y = 0; y < yres; y++)
					*p++ = 0xf800;

			/*565 绿色 0x000007e0*/
			p = (unsigned short *)fb_base;
			for(x = 0; x < xres; x++)
				for(y = 0; y < yres; y++)
					*p++ = 0x07e0;
				
			/*565 蓝色 0xf800*/
			p = (unsigned short *)fb_base;
			for(x = 0; x < xres; x++)
				for(y = 0; y < yres; y++)
					*p++ = 0x001f;
		}
	else if(32 == bpp)
		{
			/* 红色 0xff0000*/
			p2 = (unsigned int *)fb_base;
			for(x = 0; x < xres; x++)
				for(y = 0; y < yres; y++)
					*p2++ = 0xff0000;

			/* 绿色 0x00ff00*/
			p2 = (unsigned int *)fb_base;
			for(x = 0; x < xres; x++)
				for(y = 0; y < yres; y++)
					*p2++ = 0x00ff00;
				
			/* 蓝色 0x0000ff*/
			p2 = (unsigned int *)fb_base;
			for(x = 0; x < xres; x++)
				for(y = 0; y < yres; y++)
					*p2++ = 0x0000ff;
		}
	
}




