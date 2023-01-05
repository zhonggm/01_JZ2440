
#include <config.h>
#include <pic_operation.h>
#include <stdlib.h>
#include <string.h>

/* 以下的pack处理是为结构体对齐 */
#pragma pack(push) /* 将当前pack设置压栈保存 */
#pragma pack(1)    /* 指定按1字节对齐，必须在结构体定义之前使用 */
/* 文件信息头 */
typedef struct tagBITMAPFILEHEADER { /* bmfh */
	unsigned short bfType; 
	unsigned long  bfSize;
	unsigned short bfReserved1;
	unsigned short bfReserved2;
	unsigned long  bfOffBits;
} BITMAPFILEHEADER;

/* 位图信息头 */
typedef struct tagBITMAPINFOHEADER { /* bmih */
	unsigned long  biSize;
	unsigned long  biWidth;
	unsigned long  biHeight;
	unsigned short biPlanes;
	unsigned short biBitCount;
	unsigned long  biCompression;
	unsigned long  biSizeImage;
	unsigned long  biXPelsPerMeter;
	unsigned long  biYPelsPerMeter;
	unsigned long  biClrUsed;
	unsigned long  biClrImportant;
} BITMAPINFOHEADER;
#pragma pack(pop) /* 恢复先前的pack设置 */

static int IsBMPFormat(unsigned char *aFileHead);
static int GetPixelDatasFrmBMP(unsigned char *aFileHead, PT_PixelDatas ptPixelDatas);
static int FreePixelDatasForBMP(PT_PixelDatas ptPixelDatas);

T_PicFileParser g_tBMPParser = {
	.name           = "bmp",
	.IsSupport      = IsBMPFormat,
	.GetPixelDatas  = GetPixelDatasFrmBMP,
	.FreePixelDatas = FreePixelDatasForBMP,	
};

/*判断文件格式是否为BMP格式*/
static int IsBMPFormat(unsigned char *aFileHead)
{
	if (aFileHead[0] != 0x42 || aFileHead[1] != 0x4d)
		return 0;
	else
		return 1;
}

/* 转化一行bmp文件的数据为RGB格式数据，并存储到pudDstDatas位置 */
static int CovertOneLine(int iWidth, int iSrcBpp, int iDstBpp, \
	unsigned char *pudSrcDatas, unsigned char *pudDstDatas)
{
	unsigned int dwRed;
	unsigned int dwGreen;
	unsigned int dwBlue;
	
	unsigned int dwColor;

	unsigned short *pwDstDatas16bpp = (unsigned short *)pudDstDatas;
	unsigned int   *pwDstDatas32bpp = (unsigned int *)pudDstDatas;

	int i;
	int pos = 0;
	
	//过滤掉非24位bpp的源文件
	if (iSrcBpp != 24)
		return -1;
	
	/* 目的bpp是24位的话，直接拷贝为24位RGB888格式 */
	if (iDstBpp == 24)
		memcpy(pudDstDatas, pudSrcDatas, iWidth * 3);
	else{
		
		//逐个像素处理
		for (i = 0; i < iWidth; i++){

			/* 分离出原始RGB */
			dwBlue  = pudSrcDatas[pos++];
			dwGreen = pudSrcDatas[pos++];
			dwRed   = pudSrcDatas[pos++];
			
			if (iDstBpp == 32){
				
				/* 转换为 32位RGB888格式 */
				dwColor = (dwRed << 16) | (dwGreen << 8) | dwBlue;
				*pwDstDatas32bpp = dwColor;//赋给目的
				pwDstDatas32bpp++;
				
			}else if (iDstBpp == 16){
			
				/* 转换为 565RGB格式 */
				dwRed   = dwRed   >> 3;
				dwGreen = dwGreen >> 2;
				dwBlue  = dwBlue  >> 3;
				
				dwColor = (dwRed << 11) | (dwGreen << 5) | (dwBlue);
				*pwDstDatas16bpp = dwColor;
				pwDstDatas16bpp++;
				
			}
		}
	}
	
	return 0;
}

/*
 * ptPixelDatas->iBpp 是输入的参数, 它决定从BMP得到的数据要转换为该格式
 */
static int GetPixelDatasFrmBMP(unsigned char *aFileHead, PT_PixelDatas ptPixelDatas)
{
	BITMAPFILEHEADER *ptBITMAPFILEHEADER;/* 文件头信息存储位置 */
	BITMAPINFOHEADER *ptBITMAPINFOHEADER;/* 位图信息头存储位置 */

	int iWidth;
	int iHeight;
	int iBMPBpp;
	int y;

	unsigned char *pucSrc;
	unsigned char *pucDest;
	
	int iLineWidthAlign;
	int iLineWidthReal;
	
	ptBITMAPFILEHEADER = (BITMAPFILEHEADER *)aFileHead;
	ptBITMAPINFOHEADER = (BITMAPINFOHEADER *)(aFileHead + sizeof(BITMAPFILEHEADER));

	iWidth  = ptBITMAPINFOHEADER->biWidth;
	iHeight = ptBITMAPINFOHEADER->biHeight;
	iBMPBpp = ptBITMAPINFOHEADER->biBitCount;

	/* 如果源文件不是24位bpp */
	if (iBMPBpp != 24){
		DBG_PRINTF("iBMPBpp = %d\n", iBMPBpp);
		DBG_PRINTF("sizeof(BITMAPFILEHEADER) = %d\n", sizeof(BITMAPFILEHEADER));
		return -1;
	}

	ptPixelDatas->iWidth  = iWidth;
	ptPixelDatas->iHeight = iHeight;
	//ptPixelDatas->iBpp     = iBpp;
	ptPixelDatas->iLineBytes    = iWidth * ptPixelDatas->iBpp / 8;
	ptPixelDatas->aucPixelDatas = malloc(iWidth * iHeight * ptPixelDatas->iBpp / 8);
	if (NULL == ptPixelDatas->aucPixelDatas){
		return -1;
	}
	
	iLineWidthReal  = iWidth * iBMPBpp / 8;       /* 源文件每行数据真实的字节数 */
	iLineWidthAlign = (iLineWidthReal + 3) & ~0x3;/* 向4取整后每行数据的字节数  */

	//源地址，计算源文件数据的首地址，它是在实际源文件(图片)的左下角的第一个像素点们位置
	pucSrc = aFileHead + ptBITMAPFILEHEADER->bfOffBits;
	pucSrc = pucSrc + (iHeight - 1) * iLineWidthAlign;
	
	//目的地址
	pucDest = ptPixelDatas->aucPixelDatas;
	
	for (y = 0; y < iHeight; y++){		
		//memcpy(pucDest, pucSrc, iLineWidthReal);
		CovertOneLine(iWidth, iBMPBpp, ptPixelDatas->iBpp, pucSrc, pucDest);
		pucSrc  -= iLineWidthAlign;
		pucDest += ptPixelDatas->iLineBytes;
	}
	
	return 0;	
}

static int FreePixelDatasForBMP(PT_PixelDatas ptPixelDatas)
{
	free(ptPixelDatas->aucPixelDatas);
	return 0;
}



