/***************************************************************************************
Similarly, the rough outline of a JPEG decompression operation is:

Allocate and initialize a JPEG decompression object    // ����ͳ�ʼ��һ��decompression�ṹ��
Specify the source of the compressed data (eg, a file) // ָ��Դ�ļ�
Call jpeg_read_header() to obtain image info		      // ��jpeg_read_header���jpgͼ����Ϣ
Set parameters for decompression		               // ���ý�ѹ����,����Ŵ���С
jpeg_start_decompress(...); 			               // ������ѹ��jpeg_start_decompress
while (scan lines remain to be read)
	jpeg_read_scanlines(...);		                   // ѭ������jpeg_read_scanlines
jpeg_finish_decompress(...);			               // jpeg_finish_decompress������ѹ
Release the JPEG decompression object                  // �ͷ�decompression�ṹ��
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
	FBCleanScreen(0);//�����ɺ�ɫ
	
	//����ͳ�ʼ��һ��decompression�ṹ��
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	//ָ��.jpgԴ�ļ�
	if ((infile = fopen(argv[1], "rb")) == NULL) {
		fprintf(stderr, "can't open %s\n", argv[1]);	    
		return -1;	
	}	
	jpeg_stdio_src(&cinfo, infile);

	// ��jpeg_read_header���jpg�ļ���Ϣͷ
	jpeg_read_header(&cinfo, TRUE);
	
	//jpgԴ�ļ���Ϣ
	printf("image_width = %d\n", cinfo.image_width);
	printf("image_height = %d\n", cinfo.image_height);
	printf("num_components = %d\n", cinfo.num_components);
	
	// ���ý�ѹ����,����Ŵ���С
	printf("enter scale M/N:\n");
	scanf("%d/%d", &cinfo.scale_num,  &cinfo.scale_denom);
	printf("scale to : %d/%d\n", cinfo.scale_num, cinfo.scale_denom);
	//cinfo.scale_num    = 1;
	//cinfo.scale_denom  = 2;////����Ϊ1/2
	
	// ������ѹ��jpeg_start_decompress	
	jpeg_start_decompress(&cinfo);

	//�����ͼ���ļ���Ϣ
	printf("output_width = %d\n", cinfo.output_width);
	printf("output_height = %d\n", cinfo.output_height);
	printf("output_components = %d\n", cinfo.output_components);

	//һ�е����ݳ���
	/* JSAMPLEs per row in output buffer */
	row_stride = cinfo.output_width * cinfo.output_components;
	/* Make a one-row-high sample array that will go away when done with image */
	buffer = malloc(row_stride);
	
	// ѭ������jpeg_read_scanlines��һ��һ�еػ�ý�ѹ����
	while(cinfo.output_scanline < cinfo.output_height){
		(void)jpeg_read_scanlines(&cinfo, &buffer, 1);//һ�������ݴ浽buffer
		//����������д��LCDȥ
		FBShowLine(0, cinfo.output_width, cinfo.output_scanline, buffer);
	}

	free(buffer);
	jpeg_finish_decompress(&cinfo);//������ѹ
	jpeg_destroy_decompress(&cinfo);//�ͷŽ�ѹʹ�ýṹ��	

	return 0;
}

static int FBDeviceInit(void)
{
	int ret;
	
	//��fb�豸�ڵ�
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
 
	//LCD  framebuffer �ڴ�ӳ��
	g_dwScreenSize = g_tFbVar.xres * g_tFbVar.yres * g_tFbVar.bits_per_pixel / 8;
	g_pucFbMem     = (unsigned char *)mmap(NULL , g_dwScreenSize, \
		PROT_READ | PROT_WRITE,	MAP_SHARED, g_iFbFd, 0);
	
	if(g_pucFbMem < 0){
		DBG_PRINTF("can't mmap framebuffer\n");
		return -1;
	}

	g_dwLineWidth  = g_tFbVar.xres * g_tFbVar.bits_per_pixel / 8;//һ���������ݵ��ֽ���
	g_dwPixelWidth = g_tFbVar.bits_per_pixel / 8;				 //һ���������ݵ��ֽ���
	
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
				i += 2;//ÿ������ռ2���ֽ�
			}
			break;
		}
		case 32:
		{
			for(i = 0; i < g_dwScreenSize; i++){
				*pdwPen32 = dwColor;
				pdwPen32++;
				i += 4;//ÿ������ռ4���ֽ�
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

