
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



#define X_RESOLUTION 480//240
#define Y_RESOLUTION 272//320

struct lcd_regs { 
	unsigned long lcdcon1; // 0X4D000000 LCD ������1
	unsigned long lcdcon2; // 0X4D000004 LCD ������ 2 
	unsigned long lcdcon3; // 0X4D000008 LCD ������ 3 
	unsigned long lcdcon4; // 0X4D00000C LCD ������ 4 
	unsigned long lcdcon5; // 0X4D000010 LCD ������ 5 
	unsigned long lcdsaddr1; // 0X4D000014 STN/TFT��֡��������ʼ��ַ 1 
	unsigned long lcdsaddr2; // 0X4D000018 STN/TFT��֡��������ʼ��ַ 2 
	unsigned long lcdsaddr3; // 0X4D00001C STN/TFT��������Ļ��ַ���� 
	unsigned long redlut;   // 0X4D000020 STN�� ��ɫ���ұ� 
	unsigned long greenlut; // 0X4D000024 STN�� ��ɫ���ұ� 
	unsigned long bluelut; // 0X4D000028 STN�� ��ɫ���ұ� 
	unsigned long reserved[9];// 0X4D000028 - 0X4D00004C = 0x24 ʮ����:36.36/4=9.���м���� 9 ����Ԫ��1����Ԫ4	���ֽ�)�� 
	unsigned long dithmode; // 0X4D00004C STN�� ����ģʽ 
	unsigned long tpal;     // 0X4D000050 TFT�� ��ʱ��ɫ�� 
	unsigned long lcdintpnd; // 0X4D000054 LCD �жϵȴ� 
	unsigned long lcdsrcpnd; // 0X4D000058 LCD �ж�Դ 
	unsigned long lcdintmsk; // 0X4D00005C LCD �ж����� 
	unsigned long lpcsel;    // 0X4D000060 TCON(LPC3600/LCC3600)���� 
}; 

static int s3c_lcdfb_setcolreg(unsigned int regno, unsigned int red,
			     unsigned int green, unsigned int blue,
			     unsigned int transp, struct fb_info *info);


//�ο�atmel_lcdfb_ops�Ķ���
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
static u32 pseudo_palette[16];//�ٵĵ�ɫ��

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

	/* ��red green blue ��ԭɫ�����val */
	
	val  = chan_to_field(red,	&info->var.red);
	val |= chan_to_field(green, &info->var.green);
	val |= chan_to_field(blue,	&info->var.blue);


//	((u32 *)(info->pseudo_palette))[regno] = val;
	pseudo_palette[regno] = val;//���صĵ�ɫ�壬�ٵĵ�ɫ��
	
	return 0;	
}


//��ں���
static int lcd_init(void)
{
/* 1.����һ��fb_info�ṹ�� */
	s3c_lcd = framebuffer_alloc(0, NULL);
	//���ﲻ������ӳ�����

/* 2.����fb_info�ṹ�� */
/* 2.1 ���ù̶��Ĳ��� */
	strcpy(s3c_lcd->fix.id, "mylcd");
//	s3c_lcd->fix.mmio_start = xxx;//��3.3����
	s3c_lcd->fix.smem_len = X_RESOLUTION*Y_RESOLUTION*16/8;//ÿ��������16λ : 5 6 5
	s3c_lcd->fix.type = FB_TYPE_PACKED_PIXELS;
//	s3c_lcd->fix.type_aux = xxx;//�����ƽ��Ļ�����һ�����ͣ����ﲻ��Ҫ
	s3c_lcd->fix.visual = FB_VISUAL_TRUECOLOR;/* TFT */
//	s3c_lcd->fix.xpanstep = FB_VISUAL_;
//	s3c_lcd->fix.ypanstep = FB_VISUAL_;
//	s3c_lcd->fix.ywrapstep = FB_VISUAL_;
	s3c_lcd->fix.line_length = X_RESOLUTION*2;/* һ����240�����أ�ÿ��16λ��Ҳ����2���ֽ� */
	#if 0
	//���ҪӦ�ó���ȥ���ʣ���Ҫ���á�
	s3c_lcd->fix.mmio_start = FB_VISUAL_;
	s3c_lcd->fix.mmio_len = FB_VISUAL_;
	s3c_lcd->fix.accel = FB_VISUAL_;
	s3c_lcd->fix.reserved = xxx;
	#endif
	
/* 2.2 ���ÿɱ�Ĳ��� */
	s3c_lcd->var.xres = X_RESOLUTION;//����ֱ���
	s3c_lcd->var.yres = Y_RESOLUTION;
	s3c_lcd->var.xres_virtual = X_RESOLUTION;//����ֱ���
	s3c_lcd->var.yres_virtual = Y_RESOLUTION;
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
	s3c_lcd->screen_size = X_RESOLUTION*Y_RESOLUTION*16/8;
	s3c_lcd->pseudo_palette = pseudo_palette;//�ٵĵ�ɫ��

/* 3. Ӳ����ص����� */
/* 3.1 ����ԭ��ͼ����GPIO����LCD */
	gpbcon = ioremap(0x56000010, 8);//Ϊʲô����ӳ��8���ֽڣ�������ӳ��4���ֽ�
	gpbdat = gpbcon + 1;

	gpccon = ioremap(0x56000020, 4);
	gpdcon = ioremap(0x56000030, 4);
	gpgcon = ioremap(0x56000060, 4);
	/* GPIO�ܽ�����VD[7..0],LCDVF[2..0],VM,VFRAME,VLINE,VCLK,LEND  */
	*gpccon = 0xaaaaaaaa;
	*gpdcon = 0xaaaaaaaa;/* GPIO�ܽ�����VD[23..8]  */
	*gpbcon &= ~(3);/* GPB0������Ϊ������ţ����Ʊ��� */
	*gpbcon |= 1;
	*gpbdat &= ~(1<<0);/* ����͵�ƽ������������ */

	*gpgcon |= (3<<8);/* GPG4����LCD_PWREN */
		
/* 3.2 ����LCD�ֲ�����LCD������������VCLK��Ƶ�ʵȵ� */
	lcd_regs = ioremap(0x4D000000, sizeof(struct lcd_regs));

/*LCDCON1
 bit[17:8]:CLKVAL����������VCLK�ģ�LCD�ֲ�P14����Ϊ100ns;
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
//��ֱ�����ʱ�����
 bit[31:24]:VBPD,VSYNC֮���ٹ��೤ʱ����ܷ�����1�����ݣ�
            LCD�ֲ� T0 - T2 - T1 = 4��
            VBPD = 3��
 bit[23:14]:�����У�320�У�����LINEVAL = 320 - 1 = 319��
 bit[13:6] :VFPD���������һ������֮���ٹ��೤ʱ��ŷ���VSYNC��
 		   LCD�ֲ�T2-T5 = 322 - 320 =2,����VFPD = 2 - 1 = 1��
 bit[5:0]  :VSPW��VSYNC�źŵ������ȣ�LCD�ֲ�T1 = 1, ����VSPW = 1 - 1 =0��
 */
	lcd_regs->lcdcon2 = (3<<24) | ((Y_RESOLUTION - 1)<<14) | (1<<6) | (0<<0);

/*
LCDCON3
//ˮƽ�����ʱ�����
 bit[25:19]:HBPD,HSYNC֮���ٹ��೤ʱ����ܷ�����1�����ݣ�
            LCD�ֲ� T6 - T7 - T8 = 17��
            HBPD = 16��
 bit[18:8]:�����У�240�У�����HOZVAL = 240 - 1 = 239��
 bit[7:0] :HFPD���������һ������֮���ٹ��೤ʱ��ŷ���HSYNC��
           LCD�ֲ� T8 - T11 = 251 - 240 = 11,
           ����HFPD = 11 - 1 = 10;
 */
	lcd_regs->lcdcon3 = (16<<19) | ((X_RESOLUTION - 1)<<8) | (10<<0);

/*
 LCDCON4
 ˮƽ�����ͬ���ź�
 bit[7:0]: HSPW, HSYNC�źŵ������ȣ�LCD�ֲ� T7 = 5,����HSPW = 5 - 1 = 4��	
 */
	lcd_regs->lcdcon4 = 4;

/*
 LCDCON5
 �źŵļ���
 bit[11] FRM565   : 1 = 565 format
 bit[10] INVVCLK  : 0 = The video data is fetched at VCLK falling edge, LCD�ֲ�ʱ��ͼ���½���ȡ����
 bit[ 9] INVVLINE : 1��HSYNC�ź�Ҫ��ת�����͵�ƽ��Ч
 bit[8] INVVFRAME : 1��VSYNCҪ��ת
 bit[7] INVVD     : 0��video data��Ҫ��ת
 bit[6] INVVDEN   : 0��VDEN��Ҫ��ת
 bit[5] INVPWREN  : 0��PWREN��Ҫ��ת

 bit[3] PWREN     : 0�� LCD_PWREN ����͵�ƽ
 bit[1] BSWP      : 0�� 
 bit[0] HWSWP     : 1�� 2440�ֲ�P413
*/
	lcd_regs->lcdcon5 = (1<<11) | (0<<10) | (1<<9) | (1<<8) | (1<<0);

/* 3.3 �����Դ�(framebuffer),���ѵ�ַ����LCD������ */
	//s3c_lcd->fix.mmio_start = xxx;//�Դ�������ַ
	//�����ַ
	s3c_lcd->screen_base = dma_alloc_writecombine(NULL, s3c_lcd->fix.smem_len, &s3c_lcd->fix.smem_start, GFP_KERNEL);
	
	lcd_regs->lcdsaddr1 = (s3c_lcd->fix.smem_start >> 1) & (~(3<<30));//��ʼ��ַ��ֻ��29-0λ��Ч//�Դ�������ַ
	lcd_regs->lcdsaddr2 = ((s3c_lcd->fix.smem_start + s3c_lcd->fix.smem_len)>>1)&(0x1fffff);//������ַ��ֻ��21-0λ��Ч
	lcd_regs->lcdsaddr3 = (X_RESOLUTION*16/16);//һ�еĳ���( ��λ��:���֣�2�ֽ� )

/* ����LCD */
	lcd_regs->lcdcon1 |= (1<<0);/* ʹ��LCD������ */
	lcd_regs->lcdcon5 |= (1<<3);/* ʹ��LCD���� */
	*gpbdat |= 1; 				/* ����ߵ�ƽ��ʹ�ܱ���� */

/* 4. ע��  */
	register_framebuffer(s3c_lcd);

	return 0;
	
}

//���ں���
static void lcd_exit(void)
{
	/* ж��ʱ�ر�LCD,�ص����� */
	unregister_framebuffer(s3c_lcd);
	lcd_regs->lcdcon1 &= ~(1<<0);
	*gpbdat &= ~1; 
	//�ͷ��ڴ�
	dma_free_writecombine(NULL,s3c_lcd->fix.smem_len,s3c_lcd->screen_base,s3c_lcd->fix.smem_start);
	//�����ַӳ�䣬��������:��ӳ��ĺ�������������©
	iounmap(lcd_regs);
	iounmap(gpbcon);
	iounmap(gpccon);
	iounmap(gpdcon);
	iounmap(gpgcon);
	//�ͷŽṹ��
	framebuffer_release(s3c_lcd);	
}

module_init(lcd_init);
module_exit(lcd_exit);
MODULE_LICENSE("GPL");//Э��



