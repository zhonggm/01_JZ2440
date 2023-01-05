
#include <pic_operation.h>
#include <stdlib.h>
#include <string.h>

/* 缩放图片 
*ptOriginPic : 源图片数据
*ptZoomPic   : 目的图片数据
*/
int PicZoom(PT_PixelDatas ptOriginPic, PT_PixelDatas ptZoomPic)
{
    unsigned long dwDstWidth = ptZoomPic->iWidth;
    unsigned long *pdwSrcXTable = malloc(sizeof(unsigned long) * dwDstWidth);
	unsigned long x;
	unsigned long y;
	unsigned long dwSrcY;
	unsigned char *pucDest;
	unsigned char *pucSrc;
	unsigned long dwPixelBytes = ptOriginPic->iBpp/8;

	//为简单，只支持同bpp的图片缩放
	if (ptOriginPic->iBpp != ptZoomPic->iBpp)
		return -1;
	
    for (x = 0; x < dwDstWidth; x++)//生成表 pdwSrcXTable
        pdwSrcXTable[x] = (x*ptOriginPic->iWidth / ptZoomPic->iWidth);

    for (y = 0; y < ptZoomPic->iHeight; y++){			
        dwSrcY = (y * ptOriginPic->iHeight / ptZoomPic->iHeight);

		pucDest = ptZoomPic->aucPixelDatas + y*ptZoomPic->iLineBytes;
		pucSrc  = ptOriginPic->aucPixelDatas + dwSrcY*ptOriginPic->iLineBytes;
		
        for (x = 0; x <dwDstWidth; x++){
           /* 原图座标: pdwSrcXTable[x]，srcy
             * 缩放座标: x, y
			*/
			memcpy(pucDest+x*dwPixelBytes, pucSrc+pdwSrcXTable[x]*dwPixelBytes, dwPixelBytes);
        }
    }

    free(pdwSrcXTable);
	
	return 0;
}

