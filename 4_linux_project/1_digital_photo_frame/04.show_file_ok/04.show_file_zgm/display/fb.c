#include <config.h>
#include <disp_manager.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <string.h>

static int FBDeviceInit(void);
static int FBShowPixel(int iX, int iY, unsigned int dwColor);
static int FBCleanScreen(void);


static int g_fd;

static struct fb_var_screeninfo g_tFBVar;
static struct fb_fix_screeninfo g_tFBFix;			
static unsigned char *g_pucFBMem;
static unsigned int g_dwScreenSize;

static unsigned int g_dwLineWidth;
static unsigned int g_dwPixelWidth;

static T_DispOpr g_tFBOpr = {
	.name        = "fb",
	.DeviceInit  = FBDeviceInit,
	.ShowPixel   = FBShowPixel,
	.CleanScreen = FBCleanScreen,
};

static int FBDeviceInit(void)
{
	int ret;
	
	g_fd = open(FB_DEVICE_NAME, O_RDWR);
	if (0 > g_fd)
	{
		DBG_PRINTF("can't open %s\n", FB_DEVICE_NAME);
	}

	ret = ioctl(g_fd, FBIOGET_VSCREENINFO, &g_tFBVar);
	if (ret < 0)
	{
		DBG_PRINTF("can't get fb's var\n");
		return -1;
	}

	ret = ioctl(g_fd, FBIOGET_FSCREENINFO, &g_tFBFix);
	if (ret < 0)
	{
		DBG_PRINTF("can't get fb's fix\n");
		return -1;
	}
	DBG_PRINTF("g_tFBVar.bits_per_pixel = %d\n", g_tFBVar.bits_per_pixel);
	g_dwScreenSize = g_tFBVar.xres * g_tFBVar.yres * g_tFBVar.bits_per_pixel / 8;
	g_pucFBMem = (unsigned char *)mmap(NULL , g_dwScreenSize, \
					PROT_READ | PROT_WRITE, MAP_SHARED, g_fd, 0);
	if (0 > g_pucFBMem)	
	{
		DBG_PRINTF("can't mmap\n");
		return -1;
	}

	g_tFBOpr.iXres       = g_tFBVar.xres;
	g_tFBOpr.iYres       = g_tFBVar.yres;
	g_tFBOpr.iBpp        = g_tFBVar.bits_per_pixel;

	g_dwLineWidth  = g_tFBVar.xres * g_tFBVar.bits_per_pixel / 8;
	g_dwPixelWidth = g_tFBVar.bits_per_pixel / 8;
	
	return 0;
}


static int FBShowPixel(int iX, int iY, unsigned int dwColor)
{
	unsigned char *pucFB;
	unsigned short *pwFB16bpp;
	unsigned int *pdwFB32bpp;
	unsigned short wColor16bpp; /* 565 */
	int iRed;
	int iGreen;
	int iBlue;

	if ((iX >= g_tFBVar.xres) || (iY >= g_tFBVar.yres))
	{
		DBG_PRINTF("out of region\n");
		return -1;
	}

	pucFB      = g_pucFBMem + g_dwLineWidth * iY + g_dwPixelWidth * iX;
	pwFB16bpp  = (unsigned short *)pucFB;
	pdwFB32bpp = (unsigned int *)pucFB;
	
	switch (g_tFBVar.bits_per_pixel)
	{
		case 8:
		{
			*pucFB = (unsigned char)dwColor;
			break;
		}
		case 16:
		{
			iRed   = (dwColor >> (16+3)) & 0x1f;
			iGreen = (dwColor >> (8+2)) & 0x3f;
			iBlue  = (dwColor >> 3) & 0x1f;
			wColor16bpp = (iRed << 11) | (iGreen << 5) | iBlue;
			*pwFB16bpp	= wColor16bpp;
			break;
		}
		case 32:
		{
			*pdwFB32bpp = dwColor;
			break;
		}
		default :
		{
			DBG_PRINTF("can't support %d bpp\n", g_tFBVar.bits_per_pixel);
			return -1;
		}
	}

	return 0;
}

static int FBCleanScreen(void)
{
	//ֽɫ24λ 0xE7DBB5
	unsigned int r,g,b,color;
	//ת��Ϊ565��ʽ
	r = ((0xE7DBB5 & 0xff0000) >> 19);
	g = ((0xE7DBB5 & 0x00ff00) >> 10);
	b = ((0xE7DBB5 & 0x0000ff) >> 3);
	
	DBG_PRINTF("r = 0x%x\n", r);
	DBG_PRINTF("g = 0x%x\n", g);
	DBG_PRINTF("b = 0x%x\n", b);
	color = (r<<11) | (g<<5) | (b<<0);
	
	DBG_PRINTF("color = 0x%x\n", color);
	memset(g_pucFBMem, color, g_dwScreenSize);
	return 0;
}

int FBInit(void)
{
	return RegisterDispOpr(&g_tFBOpr);
}

