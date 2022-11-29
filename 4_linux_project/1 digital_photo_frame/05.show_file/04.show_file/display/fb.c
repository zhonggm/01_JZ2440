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
static int FBCleanScreen(unsigned int dwBackColor);


static int g_iFBFd;

static struct fb_var_screeninfo g_tFbVar;
static struct fb_fix_screeninfo g_tFbFix;			
static unsigned char *g_pucFbMem;
static unsigned int g_dwScreenSize;

static unsigned int g_dwLineWidth;
static unsigned int g_dwPixelWidth;

/* 构造、设置、注册一个结构体 */
static T_DispOpr g_tFbOpr = {
	.name        = "fb",
	.DeviceInit  = FBDeviceInit,
	.ShowPixel   = FBShowPixel,
	.CleanScreen = FBCleanScreen,
};

static int FBDeviceInit(void)
{
	//打开fb设备节点
	g_iFBFd = open(FB_DEVICE_NAME, O_RDWR);
	if(g_iFBFd < 0){
		DBG_PRINTF("can't open %s\n", FB_DEVICE_NAME);
		return -1;
	}

	if(ioctl(g_iFBFd, FBIOGET_VSCREENINFO, &g_tFbVar)){
		DBG_PRINTF("can't get var\n");
		return -1;
	}

	if(ioctl(g_iFBFd, FBIOGET_FSCREENINFO, &g_tFbFix)){
		DBG_PRINTF("can't get fix\n");
		return -1;
	}      

	//LCD  framebuffer内存映射
	g_dwScreenSize = g_tFbVar.xres * g_tFbVar.yres * g_tFbVar.bits_per_pixel / 8;
	g_pucFbMem = (unsigned char *)mmap(NULL , g_dwScreenSize, PROT_READ | PROT_WRITE, \
								MAP_SHARED, g_iFBFd, 0);
	if(g_pucFbMem == (unsigned char *)-1){
		DBG_PRINTF("can't mmap framebuffer\n");
		return -1;
	}

	g_tFbOpr.iXres       = g_tFbVar.xres;
	g_tFbOpr.iYres       = g_tFbVar.yres;
	g_tFbOpr.iBpp        = g_tFbVar.bits_per_pixel;

	g_dwLineWidth  = g_tFbVar.xres * g_tFbVar.bits_per_pixel / 8;//一行像素数据的字节数
	g_dwPixelWidth = g_tFbVar.bits_per_pixel / 8;				 //一个像素数据的字节数
	
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

	if ((iX >= g_tFbVar.xres) || (iY >= g_tFbVar.yres)){
		DBG_PRINTF("out of region\n");
		return -1;
	}

	pucFB      = g_pucFbMem + g_dwLineWidth * iY + g_dwPixelWidth * iX;
	pwFB16bpp  = (unsigned short *)pucFB;
	pdwFB32bpp = (unsigned int *)pucFB;
	
	switch (g_tFbVar.bits_per_pixel){
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
			DBG_PRINTF("can't support %d bpp\n", g_tFbVar.bits_per_pixel);
			return -1;
		}
	}

	return 0;
}

static int 
FBCleanScreen(unsigned int dwColor)
{
	unsigned char  *pucPen8;
	unsigned short *pwPen16;
	unsigned int   *pdwPen32;
	unsigned short wColor16bpp; /* 565 */

	unsigned int dwRed, dwGreen, dwBlue;
	int i = 0;
	
	pucPen8 = g_pucFbMem;
	pwPen16  = (unsigned short *)pucPen8;
	pdwPen32 = (unsigned int *)pucPen8;

	switch(g_tFbVar.bits_per_pixel){
		case 8:
		{
			memset(g_pucFbMem, dwColor, g_dwScreenSize);
			break;
		}
		case 16:
		{
			dwRed   = (dwColor >> (16+3)) & 0x1f;
			dwGreen = (dwColor >> (8+2)) & 0x3f;
			dwBlue  = (dwColor >> 3) & 0x1f;
			wColor16bpp = (dwRed << 11) | (dwGreen << 5) | dwBlue;
			while (i < g_dwScreenSize){
				*pwPen16 = wColor16bpp;
				pwPen16++;
				i += 2;//每个像素占2个字节
			}
			break;
		}
		case 32:
		{
			for(i = 0; i < g_dwScreenSize; i++){
				*pdwPen32 = dwColor;
				pdwPen32++;
				i += 4;//每个像素占4个字节
			}
			break;
		}
		default:
		{
			DBG_PRINTF("can't surport %dbpp\n", g_tFbVar.bits_per_pixel);
			return -1;
			break;
		}
	}

	return 0;
}



int FBInit(void)
{
	return RegisterDispOpr(&g_tFbOpr);
}


