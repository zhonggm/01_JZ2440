#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <config.h>
#include <encoding_manager.h>
#include <fonts_manager.h>
#include <disp_manager.h>
#include <input_manager.h>
#include <pic_operation.h>
#include <render.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

/* digitpic <bmp_file> */
int main(int argc, char **argv)
{
	int iFdBmp;
	int iRet;
	unsigned char *pucBMPmem;
	struct stat tBMPstat;
		
	PT_DispOpr ptDispOpr;

	extern T_PicFileParser g_tBMPParser;

	T_PixelDatas tPixelDatas;     //BMP�ļ��������������
	T_PixelDatas tPixelDatasSmall;//���ź������
	T_PixelDatas tPixelDatasFB;   //framebuffer������

	if (argc != 2){
		printf("%s <bmp_file>\n", argv[0]);
		return -1;
	}

	DebugInit();
	InitDebugChanel();

	DisplayInit();
	
	/* �����Ϊ"fb"���豸�ڵ� */
	ptDispOpr = GetDispOpr("fb");
	ptDispOpr->DeviceInit();
	ptDispOpr->CleanScreen(0);
	
	/* ��BMP�ļ� */
	iFdBmp = open(argv[1], O_RDWR);
	if (iFdBmp == -1){
		DBG_PRINTF("can't open %s\n", argv[1]);
	}

	fstat(iFdBmp, &tBMPstat);
	pucBMPmem = (unsigned char *)mmap(NULL , tBMPstat.st_size, \
		PROT_READ | PROT_WRITE, MAP_SHARED, iFdBmp, 0);
	if (pucBMPmem == (unsigned char *)-1){
		DBG_PRINTF("mmap error!\n");
		return -1;
	}

	/* ��ȡBMP�ļ���RGB����, ����, ��LCD����ʾ���� */
	iRet = g_tBMPParser.IsSupport(pucBMPmem);
	if (iRet == 0){
		DBG_PRINTF("%s is not bmp file\n", argv[1]);
		return -1;		
	}

	tPixelDatas.iBpp = ptDispOpr->iBpp;//LCD��Bpp����Ŀ��framebuffer���ݵ�Bpp
	iRet = g_tBMPParser.GetPixelDatas(pucBMPmem, &tPixelDatas);
	if (iRet){
		DBG_PRINTF("GetPixelDatas error!\n");
		return -1;		
	}
	//����framebuffer����
	tPixelDatasFB.iWidth        = ptDispOpr->iXres;
	tPixelDatasFB.iHeight       = ptDispOpr->iYres;
	tPixelDatasFB.iBpp          = ptDispOpr->iBpp;
	tPixelDatasFB.iLineBytes    = ptDispOpr->iXres * ptDispOpr->iBpp / 8; 
	tPixelDatasFB.aucPixelDatas = ptDispOpr->pucDispMem;
	//LCD����ʾԴͼƬ��С��ͼƬ
	PicMerge(0, 0, &tPixelDatas, &tPixelDatasFB);

	//����Ϊһ���С
	tPixelDatasSmall.iWidth  = tPixelDatas.iWidth/2;
	tPixelDatasSmall.iHeight = tPixelDatas.iHeight/2;
	tPixelDatasSmall.iBpp    = tPixelDatas.iBpp;
	tPixelDatasSmall.iLineBytes = tPixelDatasSmall.iWidth * tPixelDatasSmall.iBpp / 8;
	tPixelDatasSmall.aucPixelDatas = malloc(tPixelDatasSmall.iLineBytes * tPixelDatasSmall.iHeight);
	PicZoom(&tPixelDatas, &tPixelDatasSmall);
	
	//LCD����ʾԴͼƬ��Сһ���ͼƬ	
	PicMerge(128, 128, &tPixelDatasSmall, &tPixelDatasFB);

	return 0;
}

