
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



#define X_RESOLUTION 480//240
#define Y_RESOLUTION 272//320

struct lcd_regs { 
	unsigned long lcdcon1; // 0X4D000000 LCD 控制器1
	unsigned long lcdcon2; // 0X4D000004 LCD 控制器 2 
	unsigned long lcdcon3; // 0X4D000008 LCD 控制器 3 
	unsigned long lcdcon4; // 0X4D00000C LCD 控制器 4 
	unsigned long lcdcon5; // 0X4D000010 LCD 控制器 5 
	unsigned long lcdsaddr1; // 0X4D000014 STN/TFT：帧缓冲区开始地址 1 
	unsigned long lcdsaddr2; // 0X4D000018 STN/TFT：帧缓冲区开始地址 2 
	unsigned long lcdsaddr3; // 0X4D00001C STN/TFT：虚拟屏幕地址设置 
	unsigned long redlut;   // 0X4D000020 STN： 红色查找表 
	unsigned long greenlut; // 0X4D000024 STN： 绿色查找表 
	unsigned long bluelut; // 0X4D000028 STN： 蓝色查找表 
	unsigned long reserved[9];// 0X4D000028 - 0X4D00004C = 0x24 十进制:36.36/4=9.故中间相差 9 个单元（1个单元4	个字节)。 
	unsigned long dithmode; // 0X4D00004C STN： 抖动模式 
	unsigned long tpal;     // 0X4D000050 TFT： 临时调色板 
	unsigned long lcdintpnd; // 0X4D000054 LCD 中断等待 
	unsigned long lcdsrcpnd; // 0X4D000058 LCD 中断源 
	unsigned long lcdintmsk; // 0X4D00005C LCD 中断屏蔽 
	unsigned long lpcsel;    // 0X4D000060 TCON(LPC3600/LCC3600)控制 
}; 

static int s3c_lcdfb_setcolreg(unsigned int regno, unsigned int red,
			     unsigned int green, unsigned int blue,
			     unsigned int transp, struct fb_info *info);


//参考atmel_lcdfb_ops的定义
static struct fb_ops s3c_lcdfb_ops = {
	.owner		= THIS_MODULE,
//	.fb_check_var	= atmel_lcdfb_check_var,
//	.fb_set_par	= atmel_lcdfb_set_par,

	.fb_setcolreg	= s3c_lcdfb_setcolreg,
//	.fb_pan_display	= atmel_lcdfb_pan_display,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};



static struct fb_info *s3c_lcd;
static volatile unsigned long *gpbcon;
static volatile unsigned long *gpbdat;
static volatile unsigned long *gpccon;
static volatile unsigned long *gpdcon;
static volatile unsigned long *gpgcon;
static volatile struct lcd_regs* lcd_regs;
static u32 pseudo_palette[16];//假的调色板

static inline unsigned int chan_to_field(unsigned int chan,
					 struct fb_bitfield *bf)
{
	chan &= 0xffff;
	chan >>= 16 - bf->length;
	return chan << bf->offset;
}


static int s3c_lcdfb_setcolreg(unsigned int regno, unsigned int red,
			     unsigned int green, unsigned int blue,
			     unsigned int transp, struct fb_info *info)
{
	unsigned int val;
	
	if(regno > 16)
		return 1;

	/* 用red green blue 三原色构造出val */
	
	val  = chan_to_field(red,	&info->var.red);
	val |= chan_to_field(green, &info->var.green);
	val |= chan_to_field(blue,	&info->var.blue);


//	((u32 *)(info->pseudo_palette))[regno] = val;
	pseudo_palette[regno] = val;//本地的调色板，假的调色板
	
	return 0;	
}


//入口函数
static int lcd_init(void)
{
/* 1.分配一个fb_info结构体 */
	s3c_lcd = framebuffer_alloc(0, NULL);
	//这里不可以添加出错处理

/* 2.设置fb_info结构体 */
/* 2.1 设置固定的参数 */
	strcpy(s3c_lcd->fix.id, "mylcd");
//	s3c_lcd->fix.mmio_start = xxx;//在3.3设置
	s3c_lcd->fix.smem_len = X_RESOLUTION*Y_RESOLUTION*16/8;//每个像素是16位 : 5 6 5
	s3c_lcd->fix.type = FB_TYPE_PACKED_PIXELS;
//	s3c_lcd->fix.type_aux = xxx;//如果是平板的话还有一个类型，这里不需要
	s3c_lcd->fix.visual = FB_VISUAL_TRUECOLOR;/* TFT */
//	s3c_lcd->fix.xpanstep = FB_VISUAL_;
//	s3c_lcd->fix.ypanstep = FB_VISUAL_;
//	s3c_lcd->fix.ywrapstep = FB_VISUAL_;
	s3c_lcd->fix.line_length = X_RESOLUTION*2;/* 一行有240个像素，每个16位，也就是2个字节 */
	#if 0
	//如果要应用程序去访问，则要设置。
	s3c_lcd->fix.mmio_start = FB_VISUAL_;
	s3c_lcd->fix.mmio_len = FB_VISUAL_;
	s3c_lcd->fix.accel = FB_VISUAL_;
	s3c_lcd->fix.reserved = xxx;
	#endif
	
/* 2.2 设置可变的参数 */
	s3c_lcd->var.xres = X_RESOLUTION;//物理分辨率
	s3c_lcd->var.yres = Y_RESOLUTION;
	s3c_lcd->var.xres_virtual = X_RESOLUTION;//虚拟分辨率
	s3c_lcd->var.yres_virtual = Y_RESOLUTION;
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
	s3c_lcd->screen_size = X_RESOLUTION*Y_RESOLUTION*16/8;
	s3c_lcd->pseudo_palette = pseudo_palette;//假的调色板

/* 3. 硬件相关的设置 */
/* 3.1 根据原理图配置GPIO用于LCD */
	gpbcon = ioremap(0x56000010, 8);//为什么这里映射8个字节，而下面映射4个字节
	gpbdat = gpbcon + 1;

	gpccon = ioremap(0x56000020, 4);
	gpdcon = ioremap(0x56000030, 4);
	gpgcon = ioremap(0x56000060, 4);
	/* GPIO管脚用于VD[7..0],LCDVF[2..0],VM,VFRAME,VLINE,VCLK,LEND  */
	*gpccon = 0xaaaaaaaa;
	*gpdcon = 0xaaaaaaaa;/* GPIO管脚用于VD[23..8]  */
	*gpbcon &= ~(3);/* GPB0设设置为输出引脚，控制背光 */
	*gpbcon |= 1;
	*gpbdat &= ~(1<<0);/* 输出低电平，不点亮背光 */

	*gpgcon |= (3<<8);/* GPG4用作LCD_PWREN */
		
/* 3.2 根据LCD手册设置LCD控制器，比如VCLK的频率等等 */
	lcd_regs = ioremap(0x4D000000, sizeof(struct lcd_regs));

/*LCDCON1
 bit[17:8]:CLKVAL是用来决定VCLK的，LCD手册P14周期为100ns;
	 VCLK = HCLK /[(CLKVAL + 1) x 2]
	 VCLK = 100Mhz / 	[(CLKVAL + 1) x 2] = 100ns = 10Mhz
	 CLKVAL = 4

 bit[6:5]:0b11, TFT LCD,
 bit[4:1]:0b1100, 16 bpp for TFT,
 bit[0]:0 = Disable the video output and the LCD control signal,
 */
	lcd_regs->lcdcon1 = (4<<8)| (3<<5) | (0xc<<1);

/*
LCDCON2
//垂直方向的时间参数
 bit[31:24]:VBPD,VSYNC之后再过多长时间才能发出第1行数据，
            LCD手册 T0 - T2 - T1 = 4，
            VBPD = 3；
 bit[23:14]:多少行，320行，所以LINEVAL = 320 - 1 = 319；
 bit[13:6] :VFPD，发出最后一行数据之后，再过多长时间才发出VSYNC，
 		   LCD手册T2-T5 = 322 - 320 =2,所以VFPD = 2 - 1 = 1；
 bit[5:0]  :VSPW，VSYNC信号的脉冲宽度，LCD手册T1 = 1, 所以VSPW = 1 - 1 =0；
 */
	lcd_regs->lcdcon2 = (3<<24) | ((Y_RESOLUTION - 1)<<14) | (1<<6) | (0<<0);

/*
LCDCON3
//水平方向的时间参数
 bit[25:19]:HBPD,HSYNC之后再过多长时间才能发出第1列数据，
            LCD手册 T6 - T7 - T8 = 17，
            HBPD = 16；
 bit[18:8]:多少列，240列，所以HOZVAL = 240 - 1 = 239；
 bit[7:0] :HFPD，发出最后一列数据之后，再过多长时间才发出HSYNC，
           LCD手册 T8 - T11 = 251 - 240 = 11,
           所以HFPD = 11 - 1 = 10;
 */
	lcd_regs->lcdcon3 = (16<<19) | ((X_RESOLUTION - 1)<<8) | (10<<0);

/*
 LCDCON4
 水平方向的同步信号
 bit[7:0]: HSPW, HSYNC信号的脉冲宽度，LCD手册 T7 = 5,所以HSPW = 5 - 1 = 4；	
 */
	lcd_regs->lcdcon4 = 4;

/*
 LCDCON5
 信号的极性
 bit[11] FRM565   : 1 = 565 format
 bit[10] INVVCLK  : 0 = The video data is fetched at VCLK falling edge, LCD手册时序图在下降沿取数据
 bit[ 9] INVVLINE : 1，HSYNC信号要反转，即低电平有效
 bit[8] INVVFRAME : 1，VSYNC要反转
 bit[7] INVVD     : 0，video data不要反转
 bit[6] INVVDEN   : 0，VDEN不要反转
 bit[5] INVPWREN  : 0，PWREN不要反转

 bit[3] PWREN     : 0， LCD_PWREN 输出低电平
 bit[1] BSWP      : 0， 
 bit[0] HWSWP     : 1， 2440手册P413
*/
	lcd_regs->lcdcon5 = (1<<11) | (0<<10) | (1<<9) | (1<<8) | (1<<0);

/* 3.3 分配显存(framebuffer),并把地址告诉LCD控制器 */
	//s3c_lcd->fix.mmio_start = xxx;//显存的物理地址
	//虚拟地址
	s3c_lcd->screen_base = dma_alloc_writecombine(NULL, s3c_lcd->fix.smem_len, &s3c_lcd->fix.smem_start, GFP_KERNEL);
	
	lcd_regs->lcdsaddr1 = (s3c_lcd->fix.smem_start >> 1) & (~(3<<30));//起始地址，只有29-0位有效//显存的物理地址
	lcd_regs->lcdsaddr2 = ((s3c_lcd->fix.smem_start + s3c_lcd->fix.smem_len)>>1)&(0x1fffff);//结束地址，只有21-0位有效
	lcd_regs->lcdsaddr3 = (X_RESOLUTION*16/16);//一行的长度( 单位是:半字，2字节 )

/* 启动LCD */
	lcd_regs->lcdcon1 |= (1<<0);/* 使能LCD控制器 */
	lcd_regs->lcdcon5 |= (1<<3);/* 使能LCD本身 */
	*gpbdat |= 1; 				/* 输出高电平，使能背光灯 */

/* 4. 注册  */
	register_framebuffer(s3c_lcd);

	return 0;
	
}

//出口函数
static void lcd_exit(void)
{
	/* 卸载时关闭LCD,关掉背光 */
	unregister_framebuffer(s3c_lcd);
	lcd_regs->lcdcon1 &= ~(1<<0);
	*gpbdat &= ~1; 
	//释放内存
	dma_free_writecombine(NULL,s3c_lcd->fix.smem_len,s3c_lcd->screen_base,s3c_lcd->fix.smem_start);
	//解除地址映射，倒过来看:先映射的后解除，这样不会漏
	iounmap(lcd_regs);
	iounmap(gpbcon);
	iounmap(gpccon);
	iounmap(gpdcon);
	iounmap(gpgcon);
	//释放结构体
	framebuffer_release(s3c_lcd);	
}

module_init(lcd_init);
module_exit(lcd_exit);
MODULE_LICENSE("GPL");//协议



