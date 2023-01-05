
#include <config.h>
#include <debug_manager.h>

#include <config.h>
#include <input_manager.h>
#include <string.h>

#include <stdio.h>
#include <stdarg.h>

static PT_DebugOpr g_ptDebugOprHead;

static int g_iDbgLevelLimit = 8;

/*注册所有的打印方式到链表*/
int RegisterDebugOpr(PT_DebugOpr ptDebugOpr)
{
	PT_DebugOpr ptTmp;

	if (!g_ptDebugOprHead){
		g_ptDebugOprHead   = ptDebugOpr;
		ptDebugOpr->ptNext = NULL;
	}else{
		ptTmp = g_ptDebugOprHead;
		while (ptTmp->ptNext)
			ptTmp = ptTmp->ptNext;
		
		ptTmp->ptNext	   = ptDebugOpr;
		ptDebugOpr->ptNext = NULL;
	}
     
	return 0;
}

/* 列举所有的调试打印方式名称 */
void ShowDebugOpr(void)
{
	int i = 0;
	PT_DebugOpr ptTmp = g_ptDebugOprHead;

	while (ptTmp){
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}

/* strBuf = "dbglevel=6" */
int SetDbgLevel(char * strBuf)
{
	g_iDbgLevelLimit = strBuf[9] - '0';

	return 0;
}

//从链表里面根据名字找到底层具体注册的操作结构体
PT_DebugOpr GetDebugOpr(char *pcName)
{
	PT_DebugOpr ptTmp = g_ptDebugOprHead;

	while(ptTmp){
		if(0 == strcmp(ptTmp->name, pcName))
			return ptTmp;
		
		ptTmp = ptTmp->ptNext;
	}

	return NULL;
}

/*
 * stdout=0   : 关闭stdout打印
 * stdout=1	 : 打开stdout打印
 * netprint=0	 : 关闭netprint打印
 * netprint=1	 : 打开netprint打印
 */
int SetDbgChanel(char *strBuf)
{
	char *pStrTmp;
	char strName[100];
	PT_DebugOpr ptTmp;

	pStrTmp = strchr(strBuf, '=');
	if (!pStrTmp){
		return -1;
	}else{
	
		//通过名称找到打印方式结构体
		strncpy(strName, strBuf, pStrTmp-strBuf);
		strName[pStrTmp-strBuf] = '\0';
		ptTmp = GetDebugOpr(strName);
		if (!ptTmp)
			return -1;
		
		//根据命令来设置该方式是否可用
		if (pStrTmp[1] == '0')
			ptTmp->isCanUse = 0;
		else
			ptTmp->isCanUse = 1;
		
		return 0;
	}
}

/* 调试打印函数 */
int DebugPrint(const char *pcFormat, ...)
{
	char strTmpBuf[1000];
	char *pcTmp;
	
	va_list tArg;
	int iNum;
	PT_DebugOpr ptTmp = g_ptDebugOprHead;
	int dbglevel = DEFAULT_DBGLEVEL;
	
	va_start (tArg, pcFormat);
	iNum = vsprintf (strTmpBuf, pcFormat, tArg);
	va_end (tArg);
	strTmpBuf[iNum] = '\0';

	pcTmp = strTmpBuf;
	
	/* 根据打印级别决定是否打印 */
	if ((strTmpBuf[0] == '<') && (strTmpBuf[2] == '>')){
		dbglevel = strTmpBuf[1] - '0';
		if (dbglevel >= 0 && dbglevel <= 9)
			pcTmp = strTmpBuf + 3;
		else
			dbglevel = DEFAULT_DBGLEVEL;
	}

	if (dbglevel > g_iDbgLevelLimit)
		return -1;
	

	/* 调用链表中所有isCanUse为1的结构体的DebugPrint函数,把信息打印出来。*/
	while (ptTmp){
		
		if (ptTmp->isCanUse)
			ptTmp->DebugPrint(pcTmp);
		
		ptTmp = ptTmp->ptNext;
	}

	return 0;
}

//用于注册stdout.c和netprint.c里的结构体。
int DebugInit(void)
{
	int iError;

	iError  = StdoutInit();
	iError |= NetPrintInit();

	return iError;
}

//启动使用我们定义的DebugPrint
int InitDebugChanel(void)
{
	PT_DebugOpr ptTmp = g_ptDebugOprHead;
	
	while (ptTmp){
		if (ptTmp->isCanUse && ptTmp->DebugInit)
			ptTmp->DebugInit();
		
		ptTmp = ptTmp->ptNext;
	}

	return 0;
}

