#ifndef _PIC_OPERATION_H
#define _PIC_OPERATION_H

/* ������Ϣ */
typedef struct PixelDatas {
	int iWidth; //X�ֱ���
	int iHeight;//Y�ֱ���
	int iBpp;   //�������
	int iLineBytes;//һ��������ռ�ֽ���
	unsigned char *aucPixelDatas;
}T_PixelDatas, *PT_PixelDatas;

/* ��¼ͼƬ���������Ϣ */
typedef struct PicFileParser{
	char *name;
	int (*IsSupport)(unsigned char *aFileHead);
	int (*GetPixelDatas)(unsigned char *aFileHead, PT_PixelDatas tPixelDatas);
	int (*FreePixelDatas)(PT_PixelDatas tPixelDatas);
}T_PicFileParser, *PT_PicFileParser;

#endif /* _PIC_OPERATION_H */
