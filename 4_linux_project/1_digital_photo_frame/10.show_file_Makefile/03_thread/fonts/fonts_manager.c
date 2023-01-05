#include <config.h>
#include <fonts_manager.h>
#include <string.h>

static PT_FontOpr g_ptFontOprHead = NULL;

int 
RegisterFontOpr(PT_FontOpr ptFontOpr)
{
	PT_FontOpr ptTemp;

	if(NULL == g_ptFontOprHead)	{
		g_ptFontOprHead   = ptFontOpr;
		ptFontOpr->ptNext = NULL;
	}else{
		ptTemp = g_ptFontOprHead;
		
		//找到为空的地址
		while(ptTemp->ptNext){
			ptTemp = ptTemp->ptNext;
		}
		
		//把当前变量地址赋给空地址
		ptTemp->ptNext    = ptFontOpr;
		ptFontOpr->ptNext = NULL;
	}
	
	return 0;
}

void ShowFontOpr(void)
{
	int i = 0;
	PT_FontOpr ptTmp = g_ptFontOprHead;

	while (ptTmp){
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}

PT_FontOpr GetFontOpr(char *pcName)
{
	PT_FontOpr ptTmp = g_ptFontOprHead;
	
	while (ptTmp)
	{
		if (strcmp(ptTmp->name, pcName) == 0)
		{
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}
	return NULL;
}


int FontsInit(void)
{
	int iError;
	
	iError = ASCIIInit();
	if (iError)
	{
		DBG_PRINTF("ASCIIInit error!\n");
		return -1;
	}

	iError = GBKInit();
	if (iError)
	{
		DBG_PRINTF("GBKInit error!\n");
		return -1;
	}
	
	iError = FreeTypeInit();
	if (iError)
	{
		DBG_PRINTF("FreeTypeInit error!\n");
		return -1;
	}

	return 0;
}

