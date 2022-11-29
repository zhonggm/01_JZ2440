


/* �򵥲���LCD:��framebufferд���ݣ����ܲ�����ȷ��ʾ��*/

void lcd_test(void)
{
	unsigned int fb_base;
	int xres, yres, bpp;
	int x, y;
	unsigned short *p;
	unsigned int *p2;

	/* ��ʼ��LCD */
	lcd_init();

	/* ʹ��LCD */
	lcd_enable();

	/*��ȡ��ѡ��LCD�����е�: framebuffer����ַ��x�ֱ��ʡ�y�ֱ��ʡ�bpp*/
	get_lcd_params(&fb_base, &xres, &yres, &bpp);

	/*�� framebuffer��д��ɫ����*/
	if(16 == bpp)
		{
			/*565 ��ɫ 0xf800*/
			p = (unsigned short *)fb_base;
			for(x = 0; x < xres; x++)
				for(y = 0; y < yres; y++)
					*p++ = 0xf800;

			/*565 ��ɫ 0x000007e0*/
			p = (unsigned short *)fb_base;
			for(x = 0; x < xres; x++)
				for(y = 0; y < yres; y++)
					*p++ = 0x07e0;
				
			/*565 ��ɫ 0xf800*/
			p = (unsigned short *)fb_base;
			for(x = 0; x < xres; x++)
				for(y = 0; y < yres; y++)
					*p++ = 0x001f;
		}
	else if(32 == bpp)
		{
			/* ��ɫ 0xff0000*/
			p2 = (unsigned int *)fb_base;
			for(x = 0; x < xres; x++)
				for(y = 0; y < yres; y++)
					*p2++ = 0xff0000;

			/* ��ɫ 0x00ff00*/
			p2 = (unsigned int *)fb_base;
			for(x = 0; x < xres; x++)
				for(y = 0; y < yres; y++)
					*p2++ = 0x00ff00;
				
			/* ��ɫ 0x0000ff*/
			p2 = (unsigned int *)fb_base;
			for(x = 0; x < xres; x++)
				for(y = 0; y < yres; y++)
					*p2++ = 0x0000ff;
		}
	
}




