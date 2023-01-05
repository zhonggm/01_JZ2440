#ifndef _PIC_OPERATION_H
#define _PIC_OPERATION_H

/* 像素信息 */
typedef struct PixelDatas {
	int iWidth; //X分辨率
	int iHeight;//Y分辨率
	int iBpp;   //像素深度
	int iLineBytes;//一行数据所占字节数
	unsigned char *aucPixelDatas;
}T_PixelDatas, *PT_PixelDatas;

/* 记录图片解析后的信息 */
typedef struct PicFileParser{
	char *name;
	int (*IsSupport)(unsigned char *aFileHead);
	int (*GetPixelDatas)(unsigned char *aFileHead, PT_PixelDatas tPixelDatas);
	int (*FreePixelDatas)(PT_PixelDatas tPixelDatas);
}T_PicFileParser, *PT_PicFileParser;

#endif /* _PIC_OPERATION_H */
