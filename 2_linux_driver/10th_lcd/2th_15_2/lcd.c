
//in 10th_lcd file

//�Ȳο��ں������ļ����ο������ͷ�ļ�
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/clk.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/div64.h>

#include <asm/mach/map.h>
#include <asm/arch/regs-lcd.h>
#include <asm/arch/regs-gpio.h>
#include <asm/arch/fb.h>

static struct fb_info *s3c_lcd;

//�ο�atmel_lcdfb_ops�Ķ���
static struct fb_ops s3c_lcdfb_ops = {
	.owner		= THIS_MODULE,
//	.fb_check_var	= atmel_lcdfb_check_var,
//	.fb_set_par	= atmel_lcdfb_set_par,

//	.fb_setcolreg	= atmel_lcdfb_setcolreg,
//	.fb_pan_display	= atmel_lcdfb_pan_display,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};

//��ں���
static int lcd_init(void)
{
	/* 1.����һ��fb_info�ṹ�� */
	s3c_lcd = framebuffer_alloc(0,NULL);
	//���ﲻ������ӳ�����

	/* 2.����fb_info�ṹ�� */
	/* 2.1 ���ù̶��Ĳ��� */
	strcpy(s3c_lcd->fix.id, "mylcd");
//	s3c_lcd->fix.mmio_start = xxx;//��3.3����
	s3c_lcd->fix.smem_len = 240*320*16/8;//ÿ��������16λ : 5 6 5
	s3c_lcd->fix.type = FB_TYPE_PACKED_PIXELS;
//	s3c_lcd->fix.type_aux = xxx;//�����ƽ��Ļ�����һ�����ͣ����ﲻ��Ҫ
	s3c_lcd->fix.visual = FB_VISUAL_TRUECOLOR;/* TFT */
//	s3c_lcd->fix.xpanstep = FB_VISUAL_;
//	s3c_lcd->fix.ypanstep = FB_VISUAL_;
//	s3c_lcd->fix.ywrapstep = FB_VISUAL_;
	s3c_lcd->fix.line_length = 240*2;/* һ����240�����أ�ÿ��16λ��Ҳ����2���ֽ� */
	#if 0
	//���ҪӦ�ó���ȥ���ʣ���Ҫ���á�
	s3c_lcd->fix.mmio_start = FB_VISUAL_;
	s3c_lcd->fix.mmio_len = FB_VISUAL_;
	s3c_lcd->fix.accel = FB_VISUAL_;
	s3c_lcd->fix.reserved = xxx;
	#endif


	/* 2.2 ���ÿɱ�Ĳ��� */
	s3c_lcd->var.xres = 240;//����ֱ���
	s3c_lcd->var.yres = 320;
	s3c_lcd->var.xres_virtual = 240;//����ֱ���
	s3c_lcd->var.yres_virtual = 320;
	s3c_lcd->var.xoffset = 0;//����ֱ��ʺ�����ֱ��ʵĲ�ֵ 
	s3c_lcd->var.yoffset = 0;
	
	s3c_lcd->var.bits_per_pixel = 16;//ÿ��������16λ��ʾ
//	s3c_lcd->var.grayscale      = ; //����Ҫ������������bpp

	/* RGB : 5 6 5 */
	s3c_lcd->var.red.offset = 11;//�ӵ�11λ��ʼ���ܹ�5λ
	s3c_lcd->var.red.length = 5;
	s3c_lcd->var.red.msb_right = 0;//���λ����ߣ���ͬ

	s3c_lcd->var.green.offset = 5;
	s3c_lcd->var.green.length = 6;
	s3c_lcd->var.green.msb_right = 0;

	s3c_lcd->var.blue.offset = 0;
	s3c_lcd->var.blue.length = 5;
	s3c_lcd->var.blue.msb_right = 0;

//	s3c_lcd->var.transp = ;//͸����
	s3c_lcd->var.nonstd   = 0;//�����Ǳ�׼��ʽ
	s3c_lcd->var.activate = FB_ACTIVATE_NOW;//FB_ACTIVATE_
	#if 0
	s3c_lcd->var.height = 0;
	s3c_lcd->var.width = 0;
	
	s3c_lcd->var.accel_flags = 0;
	s3c_lcd->var.pixclock = 0;
	s3c_lcd->var.left_margin = 0;
	s3c_lcd->var.right_margin = 0;
	s3c_lcd->var.upper_margin = 0;
	s3c_lcd->var.lower_margin = 0;
	
	s3c_lcd->var.hsync_len = 0;
	s3c_lcd->var.vsync_len = 0;
	s3c_lcd->var.sync = 0;
	s3c_lcd->var.vmode = 0;
	s3c_lcd->var.rotate = 0;
	s3c_lcd->var.reserved = 0;
	#endif
	
	/* 2.3 ���ò������� */
	s3c_lcd->fbops = &s3c_lcdfb_ops;

	/* 2.4 ���������� */
//	s3c_lcd->screen_base = ;//�Դ�������ַ
	s3c_lcd->screen_size = 240*320*16/8;
//	s3c_lcd->pseudo_palette = xxx;//�ٵĵ�ɫ��


	/* 3. Ӳ����ص����� */
	/* 3.1 ����GPIO����LCD */
	
	/* 3.2 ����LCD�ֲ�����LCD������������VCLK��Ƶ�ʵȵ� */
	
	/* 3.3 �����Դ�(framebuffer),���ѵ�ַ����LCD������ */
	//s3c_lcd->fix.mmio_start = xxx;//�Դ�������ַ


	/* 4. ע��  */
	register_framebuffer(s3c_lcd);



	return 0;
	
}

//���ں���
static void lcd_exit(void)
{
	
}

module_init(lcd_init);
module_exit(lcd_exit);
MODULE_LICENSE("GPL");//Э��



