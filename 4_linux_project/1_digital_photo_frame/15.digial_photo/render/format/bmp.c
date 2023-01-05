
#include <config.h>
#include <pic_operation.h>
#include <stdlib.h>
#include <string.h>

/* ���µ�pack������Ϊ�ṹ����� */
#pragma pack(push) /* ����ǰpack����ѹջ���� */
#pragma pack(1)    /* ָ����1�ֽڶ��룬�����ڽṹ�嶨��֮ǰʹ�� */
/* �ļ���Ϣͷ */
typedef struct tagBITMAPFILEHEADER { /* bmfh */
	unsigned short bfType; 
	unsigned long  bfSize;
	unsigned short bfReserved1;
	unsigned short bfReserved2;
	unsigned long  bfOffBits;
} BITMAPFILEHEADER;

/* λͼ��Ϣͷ */
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
#pragma pack(pop) /* �ָ���ǰ��pack���� */

static int IsBMPFormat(unsigned char *aFileHead);
static int GetPixelDatasFrmBMP(unsigned char *aFileHead, PT_PixelDatas ptPixelDatas);
static int FreePixelDatasForBMP(PT_PixelDatas ptPixelDatas);

T_PicFileParser g_tBMPParser = {
	.name           = "bmp",
	.IsSupport      = IsBMPFormat,
	.GetPixelDatas  = GetPixelDatasFrmBMP,
	.FreePixelDatas = FreePixelDatasForBMP,	
};

/*�ж��ļ���ʽ�Ƿ�ΪBMP��ʽ*/
static int IsBMPFormat(unsigned char *aFileHead)
{
	if (aFileHead[0] != 0x42 || aFileHead[1] != 0x4d)
		return 0;
	else
		return 1;
}

/* ת��һ��bmp�ļ�������ΪRGB��ʽ���ݣ����洢��pudDstDatasλ�� */
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
	
	//���˵���24λbpp��Դ�ļ�
	if (iSrcBpp != 24)
		return -1;
	
	/* Ŀ��bpp��24λ�Ļ���ֱ�ӿ���Ϊ24λRGB888��ʽ */
	if (iDstBpp == 24)
		memcpy(pudDstDatas, pudSrcDatas, iWidth * 3);
	else{
		
		//������ش���
		for (i = 0; i < iWidth; i++){

			/* �����ԭʼRGB */
			dwBlue  = pudSrcDatas[pos++];
			dwGreen = pudSrcDatas[pos++];
			dwRed   = pudSrcDatas[pos++];
			
			if (iDstBpp == 32){
				
				/* ת��Ϊ 32λRGB888��ʽ */
				dwColor = (dwRed << 16) | (dwGreen << 8) | dwBlue;
				*pwDstDatas32bpp = dwColor;//����Ŀ��
				pwDstDatas32bpp++;
				
			}else if (iDstBpp == 16){
			
				/* ת��Ϊ 565RGB��ʽ */
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
 * ptPixelDatas->iBpp ������Ĳ���, ��������BMP�õ�������Ҫת��Ϊ�ø�ʽ
 */
static int GetPixelDatasFrmBMP(unsigned char *aFileHead, PT_PixelDatas ptPixelDatas)
{
	BITMAPFILEHEADER *ptBITMAPFILEHEADER;/* �ļ�ͷ��Ϣ�洢λ�� */
	BITMAPINFOHEADER *ptBITMAPINFOHEADER;/* λͼ��Ϣͷ�洢λ�� */

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

	/* ���Դ�ļ�����24λbpp */
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
	
	iLineWidthReal  = iWidth * iBMPBpp / 8;       /* Դ�ļ�ÿ��������ʵ���ֽ��� */
	iLineWidthAlign = (iLineWidthReal + 3) & ~0x3;/* ��4ȡ����ÿ�����ݵ��ֽ���  */

	//Դ��ַ������Դ�ļ����ݵ��׵�ַ��������ʵ��Դ�ļ�(ͼƬ)�����½ǵĵ�һ�����ص���λ��
	pucSrc = aFileHead + ptBITMAPFILEHEADER->bfOffBits;
	pucSrc = pucSrc + (iHeight - 1) * iLineWidthAlign;
	
	//Ŀ�ĵ�ַ
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



