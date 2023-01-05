/***************************************************************************************
Similarly, the rough outline of a JPEG decompression operation is:

Allocate and initialize a JPEG decompression object    // 分配和初始化一个decompression结构体
Specify the source of the compressed data (eg, a file) // 指定源文件
Call jpeg_read_header() to obtain image info		      // 用jpeg_read_header获得jpg图像信息
Set parameters for decompression		               // 设置解压参数,比如放大、缩小
jpeg_start_decompress(...); 			               // 启动解压：jpeg_start_decompress
while (scan lines remain to be read)
	jpeg_read_scanlines(...);		                   // 循环调用jpeg_read_scanlines
jpeg_finish_decompress(...);			               // jpeg_finish_decompress结束解压
Release the JPEG decompression object                  // 释放decompression结构体
***************************************************************************************/
#include <stdio.h>
#include "jpeglib.h"
#include <setjmp.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <string.h>

#include <stdlib.h>


#define FB_DEVICE_NAME "/dev/fb0"
#define DBG_PRINTF printf

static int g_iFbFd;

static struct fb_var_screeninfo g_tFbVar;
static struct fb_fix_screeninfo g_tFbFix;			
static unsigned char *g_pucFbMem;
static unsigned int g_dwScreenSize;

static unsigned int g_dwLineWidth;
static unsigned int g_dwPixelWidth;

static int FBDeviceInit(void);
static int FBShowPixel(int iX, int iY, unsigned int dwColor);
static int FBCleanScreen(unsigned int dwColor);
static int FBShowLine(int iXStart, int iXEnd, int iY, unsigned char *pucRGBArray);


/* Uage : jpg2rgb <jpg_file>
 */
int main(int argc, char *argv[])
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE *infile;
	
	unsigned char *buffer;	/* Output row buffer */
	int row_stride;		/* physical row width in image buffer */

	if(FBDeviceInit()){
		return -1;
	}
	FBCleanScreen(0);//清屏成黑色
	
	//分配和初始化一个decompression结构体
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	//指定.jpg源文件
	if ((infile = fopen(argv[1], "rb")) == NULL) {
		fprintf(stderr, "can't open %s\n", argv[1]);	    
		return -1;	
	}	
	jpeg_stdio_src(&cinfo, infile);

	// 用jpeg_read_header获得jpg文件信息头
	jpeg_read_header(&cinfo, TRUE);
	
	//jpg源文件信息
	printf("image_width = %d\n", cinfo.image_width);
	printf("image_height = %d\n", cinfo.image_height);
	printf("num_components = %d\n", cinfo.num_components);
	
	// 设置解压参数,比如放大、缩小
	printf("enter scale M/N:\n");
	scanf("%d/%d", &cinfo.scale_num,  &cinfo.scale_denom);
	printf("scale to : %d/%d\n", cinfo.scale_num, cinfo.scale_denom);
	//cinfo.scale_num    = 1;
	//cinfo.scale_denom  = 2;////比例为1/2
	
	// 启动解压：jpeg_start_decompress	
	jpeg_start_decompress(&cinfo);

	//输出的图像文件信息
	printf("output_width = %d\n", cinfo.output_width);
	printf("output_height = %d\n", cinfo.output_height);
	printf("output_components = %d\n", cinfo.output_components);

	//一行的数据长度
	/* JSAMPLEs per row in output buffer */
	row_stride = cinfo.output_width * cinfo.output_components;
	/* Make a one-row-high sample array that will go away when done with image */
	buffer = malloc(row_stride);
	
	// 循环调用jpeg_read_scanlines来一行一行地获得解压数据
	while(cinfo.output_scanline < cinfo.output_height){
		(void)jpeg_read_scanlines(&cinfo, &buffer, 1);//一行数据暂存到buffer
		//把这行数据写到LCD去
		FBShowLine(0, cinfo.output_width, cinfo.output_scanline, buffer);
	}

	free(buffer);
	jpeg_finish_decompress(&cinfo);//结束解压
	jpeg_destroy_decompress(&cinfo);//释放解压使用结构体	

	return 0;
}

static int FBDeviceInit(void)
{
	int ret;
	
	//打开fb设备节点
	g_iFbFd = open(FB_DEVICE_NAME, O_RDWR);
	if(g_iFbFd < 0){
		DBG_PRINTF("can't open %s\n", FB_DEVICE_NAME);
		return -1;
	}

	ret = ioctl(g_iFbFd, FBIOGET_VSCREENINFO, &g_tFbVar);
	if(ret < 0){
		DBG_PRINTF("can't get fb's var\n");
		return -1;
	}
	
	ret = ioctl(g_iFbFd, FBIOGET_FSCREENINFO, &g_tFbFix);
	if(ret < 0){
		DBG_PRINTF("can't get fb's fix\n");
		return -1;
	}      
 
	//LCD  framebuffer 内存映射
	g_dwScreenSize = g_tFbVar.xres * g_tFbVar.yres * g_tFbVar.bits_per_pixel / 8;
	g_pucFbMem     = (unsigned char *)mmap(NULL , g_dwScreenSize, \
		PROT_READ | PROT_WRITE,	MAP_SHARED, g_iFbFd, 0);
	
	if(g_pucFbMem < 0){
		DBG_PRINTF("can't mmap framebuffer\n");
		return -1;
	}

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

static int FBCleanScreen(unsigned int dwColor)
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

static int FBShowLine(int iXStart, int iXEnd, int iY, unsigned char *pucRGBArray)
{
	int i = iXStart * 3;
	int iX;
	unsigned int dwColor;

	if (iY >= g_tFbVar.yres)
		return -1;

	if (iXStart >= g_tFbVar.xres)
		return -1;

	if (iXEnd >= g_tFbVar.xres)
		iXEnd = g_tFbVar.xres;		
	
	for (iX = iXStart; iX < iXEnd; iX++){
		/* 0xRRGGBB */
		dwColor = (pucRGBArray[i]<<16) + (pucRGBArray[i+1]<<8) + (pucRGBArray[i+2]<<0);
		i += 3;
		FBShowPixel(iX, iY, dwColor);
	}
	
	return 0;
}

