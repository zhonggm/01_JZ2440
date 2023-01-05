
#include <config.h>
#include <debug_manager.h>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static int StdoutDebugPrint(char *strData)
{
	/* ֱ�Ӱ������Ϣ��print��ӡ���� */
	printf("%s", strData);

	return strlen(strData);	
}

static T_DebugOpr g_tStdoutDbgOpr = {
	.name        = "stdout",
	.isCanUse    = 1,
	.DebugPrint  = StdoutDebugPrint,
};

int StdoutInit(void)
{
	return RegisterDebugOpr(&g_tStdoutDbgOpr);
}

