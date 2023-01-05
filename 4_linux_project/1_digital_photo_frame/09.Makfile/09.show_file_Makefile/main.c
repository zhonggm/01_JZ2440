#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <config.h>
#include <draw.h>
#include <encoding_manager.h>
#include <fonts_manager.h>
#include <disp_manager.h>
#include <string.h>

/* ./show_file [-s Size] [-f freetype_font_file] [-h HZK] <text_file> */
int main(int argc, char *argv[])
{
	int iError;
	unsigned int dwFontSize = 16;
	char acHzkFile[128];     //�洢���ֿ���
	char acFreetypeFile[128];//�洢freetype�ֿ���
	char acTextFile[128];    //�洢�ı��ļ���

	char acDisplay[128];     //�洢��ʾ�豸��

	char cOpr;
	int bList = 0;

	acHzkFile[0]  	  = '\0';
	acFreetypeFile[0] = '\0';
	acTextFile[0] 	  = '\0';

	strcpy(acDisplay, "fb");
	
	while ((iError = getopt(argc, argv, "ls:f:h:d:")) != -1){
		switch(iError){
			case 'l':
			{
				bList = 1;
				break;
			}
			case 's':
			{
				//optarg ָ��s �ĺ����һ������
				dwFontSize = strtoul(optarg, NULL, 0);
				break;
			}
			case 'f':
			{
				strncpy(acFreetypeFile, optarg, 128);
				acFreetypeFile[127] = '\0';
				break;
			}
			case 'h':
			{
				strncpy(acHzkFile, optarg, 128);
				acHzkFile[127] = '\0';
				break;
			}
			case 'd':
			{
				strncpy(acDisplay, optarg, 128);
				acDisplay[127] = '\0';
				break;
			}
			default:
			{
				printf("Usage: %s [-s Size] [-d display] [-f font_file] [-h HZK] <text_file>\n", argv[0]);
				printf("Usage: %s -l\n", argv[0]);
				return -1;
				break;
			}
		}
	}

	/*optind ��ָ���ѡ��ĵ�һ��������
	    ����optind < argc��
	*/
	if (!bList && (optind >= argc)){
		printf("Usage: %s [-s Size] [-d display] [-f font_file] [-h HZK] <text_file>\n", argv[0]);
		printf("Usage: %s -l\n", argv[0]);
		return -1;
	}
	
	iError = DisplayInit();
	if (iError)
	{
		DBG_PRINTF("DisplayInit error!\n");
		return -1;
	}

	iError = FontsInit();
	if (iError)
	{
		DBG_PRINTF("FontsInit error!\n");
		return -1;
	}

	iError = EncodingInit();
	if (iError)
	{
		DBG_PRINTF("EncodingInit error!\n");
		return -1;
	}

	if (bList){
		printf("supported display:\n");
		ShowDispOpr();

		printf("supported font:\n");
		ShowFontOpr();

		printf("supported encoding:\n");
		ShowEncodingOpr();
		return 0;
	}

	//ʣ��δ�����Ļ����argc[optind]�ֻ����128���ֽڳ��ȡ�
	strncpy(acTextFile, argv[optind], 128);
	acTextFile[127] = '\0';//��ӽ�����
	
	//��һ��Ҫ��ʾ���ı��ļ�
	iError = OpenTextFile(acTextFile);
	if (iError)
	{
		DBG_PRINTF("OpenTextFile error!\n");
		return -1;
	}
	//�����ļ�Ҫʹ�õĺ��ֿ��freetype�ֿ⣬�ֵĴ�С��
	iError = SetTextDetail(acHzkFile, acFreetypeFile, dwFontSize);
	if (iError)
	{
		DBG_PRINTF("SetTextDetail error!\n");
		return -1;
	}
	
	//ѡ����ʾ�豸����ʼ��
	iError = SelectAndInitDisplay(acDisplay);
	if (iError)
	{
		DBG_PRINTF("SelectAndInitDisplay error!\n");
		return -1;
	}
	
	iError = ShowNextPage();
	if (iError){
		DBG_PRINTF("Error to show first page\n");
		return -1;
	}

	while (1){
		printf("Enter 'n' to show next page, 'u' to show previous page, 'q' to exit: ");

		do {
			cOpr = getchar();			
		} while ((cOpr != 'n') && (cOpr != 'u') && (cOpr != 'q'));

		if (cOpr == 'n')
			ShowNextPage();
		else if (cOpr == 'u')
			ShowPrePage();			
		else 
			return 0;		
	}
	
	return 0;
}

