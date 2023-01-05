#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <config.h>
#include <draw.h>
#include <encoding_manager.h>
#include <fonts_manager.h>
#include <disp_manager.h>
#include <string.h>

#include <input_manager.h>

/* ./show_file [-s Size] [-f freetype_font_file] [-h HZK] <text_file> */
int main(int argc, char *argv[])
{
	int iError;
	unsigned int dwFontSize = 16;
	char acHzkFile[128];      //用于存储汉字库名
	char acFreetypeFile[128]; //用于存储freetype字库名
	char acTextFile[128];     //用于存储文本文件名
	char acDisplay[128];      //用于存储显示设备名

	int bList = 0; /* 输入列举命令标志位 */

	T_InputEvent tInputEvent;/* 用于保存输入事件 */

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
				//optarg 指向s 的后面第一个参数
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

	/*optind 会指向非选项的第一个参数，
	    正常optind < argc。
	*/
	if (!bList && (optind >= argc)){
		printf("Usage: %s [-s Size] [-d display] [-f font_file] [-h HZK] <text_file>\n", argv[0]);
		printf("Usage: %s -l\n", argv[0]);
		return -1;
	}
	
	iError = DisplayInit();
	if (iError){
		DBG_PRINTF("DisplayInit error!\n");
		return -1;
	}

	iError = FontsInit();
	if (iError){
		DBG_PRINTF("FontsInit error!\n");
		return -1;
	}

	iError = EncodingInit();
	if (iError){
		DBG_PRINTF("EncodingInit error!\n");
		return -1;
	}
	

	iError = InputInit();
	if (iError){
		DBG_PRINTF("InputInit error!\n");
		return -1;
	}

	if (bList){
		printf("supported display:\n");
		ShowDispOpr();

		printf("supported font:\n");
		ShowFontOpr();

		printf("supported encoding:\n");
		ShowEncodingOpr();

		printf("supported input:\n");
		ShowInputOpr();

		return 0;
	}

	//剩下未解析的字符串会存在argc[optind] 里，只保留128个字节长度。
	strncpy(acTextFile, argv[optind], 128);
	acTextFile[127] = '\0';//添加结束符

	//打开一个要显示的文本文件
	iError = OpenTextFile(acTextFile);
	if (iError){
		DBG_PRINTF("OpenTextFile error!\n");
		return -1;
	}
	

	//设置文件要使用的汉字库或freetype 字库，字的大小。
	iError = SetTextDetail(acHzkFile, acFreetypeFile, dwFontSize);
	if (iError){
		DBG_PRINTF("SetTextDetail error!\n");
		return -1;
	}
	

	//选择显示设备并初始化
	iError = SelectAndInitDisplay(acDisplay);
	if (iError){
		DBG_PRINTF("SelectAndInitDisplay error!\n");
		return -1;
	}
	
	//所有输入设备初始化
	iError = AllInputDevicesInit();	
	if (iError){
		DBG_PRINTF("AllInputDevicesInit error!\n");
		return -1;
	}
	
	iError = ShowNextPage();
	if (iError){
		DBG_PRINTF("Error to show first page\n");
		return -1;
	}
	
	printf("Enter 'n' to show next page, 'u' to show previous page, 'q' to exit: ");
	
	while (1){
		if(0 == GetInputEvent(&tInputEvent)){
			switch(tInputEvent.iVal){
				case INPUT_VALUE_UP:   ShowPrePage();    break;
				case INPUT_VALUE_DOWN: ShowNextPage();   break;
				case INPUT_VALUE_EXIT: return 0;  	     break;
				default: DBG_PRINTF("Invalid input!\n"); break;
			}
		}
	}
	
	return 0;
}



