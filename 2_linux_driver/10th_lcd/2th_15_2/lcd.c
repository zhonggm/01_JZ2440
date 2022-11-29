
//in 10th_lcd file

//先参考内核驱动文件，参考里面的头文件
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

//参考atmel_lcdfb_ops的定义
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

//入口函数
static int lcd_init(void)
{
	/* 1.分配一个fb_info结构体 */
	s3c_lcd = framebuffer_alloc(0,NULL);
	//这里不可以添加出错处理

	/* 2.设置fb_info结构体 */
	/* 2.1 设置固定的参数 */
	strcpy(s3c_lcd->fix.id, "mylcd");
//	s3c_lcd->fix.mmio_start = xxx;//在3.3设置
	s3c_lcd->fix.smem_len = 240*320*16/8;//每个像素是16位 : 5 6 5
	s3c_lcd->fix.type = FB_TYPE_PACKED_PIXELS;
//	s3c_lcd->fix.type_aux = xxx;//如果是平板的话还有一个类型，这里不需要
	s3c_lcd->fix.visual = FB_VISUAL_TRUECOLOR;/* TFT */
//	s3c_lcd->fix.xpanstep = FB_VISUAL_;
//	s3c_lcd->fix.ypanstep = FB_VISUAL_;
//	s3c_lcd->fix.ywrapstep = FB_VISUAL_;
	s3c_lcd->fix.line_length = 240*2;/* 一行有240个像素，每个16位，也就是2个字节 */
	#if 0
	//如果要应用程序去访问，则要设置。
	s3c_lcd->fix.mmio_start = FB_VISUAL_;
	s3c_lcd->fix.mmio_len = FB_VISUAL_;
	s3c_lcd->fix.accel = FB_VISUAL_;
	s3c_lcd->fix.reserved = xxx;
	#endif


	/* 2.2 设置可变的参数 */
	s3c_lcd->var.xres = 240;//物理分辨率
	s3c_lcd->var.yres = 320;
	s3c_lcd->var.xres_virtual = 240;//虚拟分辨率
	s3c_lcd->var.yres_virtual = 320;
	s3c_lcd->var.xoffset = 0;//物理分辨率和虚拟分辨率的差值 
	s3c_lcd->var.yoffset = 0;
	
	s3c_lcd->var.bits_per_pixel = 16;//每个像素用16位表示
//	s3c_lcd->var.grayscale      = ; //不需要，已设设置了bpp

	/* RGB : 5 6 5 */
	s3c_lcd->var.red.offset = 11;//从第11位开始，总共5位
	s3c_lcd->var.red.length = 5;
	s3c_lcd->var.red.msb_right = 0;//最高位在左边，下同

	s3c_lcd->var.green.offset = 5;
	s3c_lcd->var.green.length = 6;
	s3c_lcd->var.green.msb_right = 0;

	s3c_lcd->var.blue.offset = 0;
	s3c_lcd->var.blue.length = 5;
	s3c_lcd->var.blue.msb_right = 0;

//	s3c_lcd->var.transp = ;//透明度
	s3c_lcd->var.nonstd   = 0;//这里是标准格式
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
	
	/* 2.3 设置操作函数 */
	s3c_lcd->fbops = &s3c_lcdfb_ops;

	/* 2.4 其他的设置 */
//	s3c_lcd->screen_base = ;//显存的虚拟地址
	s3c_lcd->screen_size = 240*320*16/8;
//	s3c_lcd->pseudo_palette = xxx;//假的调色板


	/* 3. 硬件相关的设置 */
	/* 3.1 配置GPIO用于LCD */
	
	/* 3.2 根据LCD手册设置LCD控制器，比如VCLK的频率等等 */
	
	/* 3.3 分配显存(framebuffer),并把地址告诉LCD控制器 */
	//s3c_lcd->fix.mmio_start = xxx;//显存的物理地址


	/* 4. 注册  */
	register_framebuffer(s3c_lcd);



	return 0;
	
}

//出口函数
static void lcd_exit(void)
{
	
}

module_init(lcd_init);
module_exit(lcd_exit);
MODULE_LICENSE("GPL");//协议



