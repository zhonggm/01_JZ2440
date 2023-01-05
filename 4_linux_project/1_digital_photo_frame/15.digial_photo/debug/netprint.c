
#include <config.h>
#include <debug_manager.h>

#include <input_manager.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <signal.h>

#if 0
��������ϵͳ�ļ�����,����һ�㲻�����ͷ�ļ�,
��ʹ������һ���ṹ(struct sockaddr_in)������
	struct sockaddr_in{
		unsigned short		  sin_family;	  
		unsigned short int	  sin_port;
		struct in_addr		  sin_addr;
		unsigned char		  sin_zero[8];
	};


������Ҫʹ��Internet����
        sin_familyһ��ΪAF_INET,
        sin_addr����ΪINADDR_ANY��ʾ���Ժ��κε�����ͨ��,
        sin_port������Ҫ�����Ķ˿ں�.
        sin_zero[8]����������. 
  bind�����صĶ˿�ͬsocket���ص��ļ�������������һ��.
  �ɹ��Ƿ���0,ʧ�ܵ������socketһ�� 
#endif

/* ������UDP���ѿ����嵱��server�ˡ�*/
#define SERVER_PORT 	    5678
#define PRINT_BUF_MAX_SIZE (16*1024)

static int g_iSocketServer;
static struct sockaddr_in g_tSocketServerAddr;
static struct sockaddr_in g_tSocketClientAddr;

static char *g_pcNetPrintBuf;
static int g_iWritePos = 0;
static int g_iReadPos  = 0;

static int g_iHaveConnected = 0;/* �����ӱ�־ */

static pthread_t g_tSendTreadID;//�����߳�ID
static pthread_t g_tRecvTreadID;//�����߳�ID

static pthread_mutex_t g_tNetDbgSendMutex  = PTHREAD_MUTEX_INITIALIZER;//������
static pthread_cond_t  g_tNetDbgSendConVar = PTHREAD_COND_INITIALIZER; //��������

static int IsEmpty(void);
static int GetData(char *pcVal);

/* �����̴߳����� */
static void *NetDbgSendThreadFunction(void *pVoid)
{
	char strTmpBuf[512];/* �ѻ����������ݶ�ȡ�����������Ա����ڷ��� */
	char cVal;
	int i;
	int iAddrLen;
	int iSendLen;
	
	while (1){
		/* ƽʱ����,�ȴ��������� g_tNetDbgSendConVar �����͹��� */
		pthread_mutex_lock(&g_tNetDbgSendMutex);/* ��û����� */
		pthread_cond_wait(&g_tNetDbgSendConVar, &g_tNetDbgSendMutex);	
		pthread_mutex_unlock(&g_tNetDbgSendMutex);/* �ͷŻ����� */

		while (g_iHaveConnected && !IsEmpty()){
			i = 0;

			/* �ѻ��λ�����������ȡ����,���ȡ512�ֽ� */
			while ((i < 512) && (0 == GetData(&cVal))){
				strTmpBuf[i] = cVal;
				i++;
			}
			
			/* ִ�е�����, ��ʾ������ */
			/* ��sendto�������ʹ�ӡ��Ϣ�������ӵĿͻ��� */
			iAddrLen = sizeof(struct sockaddr);
			iSendLen = sendto(g_iSocketServer, strTmpBuf, i, 0,
		        (const struct sockaddr *)&g_tSocketClientAddr, iAddrLen);
		}
	}
	
	return NULL;
}

/* �����̴߳����� */
static void *NetDbgRecvThreadFunction(void *pVoid)
{
	socklen_t iAddrLen;
	int iRecvLen;
	char ucRecvBuf[1000];
	struct sockaddr_in tSocketClientAddr;

	while (1){
		iAddrLen = sizeof(struct sockaddr);
		DBG_PRINTF("in NetDbgRecvTreadFunction\n");
		iRecvLen = recvfrom(g_iSocketServer, ucRecvBuf, 999, 0, \
			(struct sockaddr *)&tSocketClientAddr, &iAddrLen);
		
		if (iRecvLen > 0){
			ucRecvBuf[iRecvLen] = '\0';
			DBG_PRINTF("netprint.c get msg: %s\n", ucRecvBuf);
			
			/* ��������:
			 * setclient            : ���ý��մ�ӡ��Ϣ�Ŀͻ���
			 * dbglevel=0,1,2...    : �޸Ĵ�ӡ����
			 * stdout=0             : �ر�stdout��ӡ
			 * stdout=1             : ��stdout��ӡ
			 * netprint=0           : �ر�netprint��ӡ
			 * netprint=1           : ��netprint��ӡ
			 */
			if (0 == strcmp(ucRecvBuf, "setclient")){
				g_tSocketClientAddr = tSocketClientAddr;//���¿ͻ�����Ϣ
				g_iHaveConnected    = 1;
			}else if (0 == strncmp(ucRecvBuf, "dbglevel=", 9)){
				SetDbgLevel(ucRecvBuf);//���´�ӡ����
			}else{
				SetDbgChanel(ucRecvBuf);//���´�ӡ��ʽ����
			}
		}
	}
	
	return NULL;
}

int NetDbgInit(void)
{
	int iRet;
	
	g_iSocketServer = socket(AF_INET, SOCK_DGRAM, 0);
	if(-1 == g_iSocketServer){
		printf("socket error!\n");
		return -1;
	}

	g_tSocketServerAddr.sin_family      = AF_INET;
	g_tSocketServerAddr.sin_port        = htons(SERVER_PORT);
	g_tSocketServerAddr.sin_addr.s_addr = INADDR_ANY;/*��ʾ���Ժ��κε�����ͨ��*/
	memset(g_tSocketServerAddr.sin_zero, 0, 8);

	iRet = bind(g_iSocketServer,(const struct sockaddr *)&g_tSocketServerAddr,\
		sizeof(struct sockaddr));
	if(-1 == iRet){
		printf("bind error!\n");
		return -1;
	}

	/* bind �ɹ����ٷ����ӡ����ʹ�õĿռ� */
	g_pcNetPrintBuf = malloc(PRINT_BUF_MAX_SIZE);
	if(NULL == g_pcNetPrintBuf){
		close(g_iSocketServer);
		return -1;
	}

	/* ����netprint�����߳�: ���������ʹ�ӡ��Ϣ���ͻ��� */
	pthread_create(&g_tSendTreadID, NULL, NetDbgSendThreadFunction, NULL);			
	
	/* ����netprint�����߷�: �������տ�����Ϣ,�����޸Ĵ�ӡ����,��/�رմ�ӡ */
	pthread_create(&g_tRecvTreadID, NULL, NetDbgRecvThreadFunction, NULL);		
	
	return 0;
}

static int NetDbgExit(void)
{
	close(g_iSocketServer);
	free(g_pcNetPrintBuf);

	return 0;
}

/* ��ӡ������Ϊ�� */
static int IsFull(void)
{
	return (g_iReadPos == ((g_iWritePos + 1) % PRINT_BUF_MAX_SIZE));
}

/* ��ӡ������Ϊ�� */
static int IsEmpty(void)
{
	return (g_iReadPos == g_iWritePos);
}

/* ��ӡ�������������*/
/* ���� 0 ���ɹ���
 * ����-1 ���ʧ�ܡ�
 */
static int PutData(char cVal)
{
	if(IsFull())
		return -1;
	else{
		g_pcNetPrintBuf[g_iWritePos] = cVal;
		g_iWritePos = (g_iWritePos + 1) % PRINT_BUF_MAX_SIZE;
		return 0;
	}
}

/* ��ӡ���������ݻ�ȡ */
static int GetData(char *pcVal)
{
	if(IsEmpty())
		return -1;
	else{
		*pcVal = g_pcNetPrintBuf[g_iReadPos];
		g_iReadPos = (g_iReadPos + 1) % PRINT_BUF_MAX_SIZE;
		return 0;
	}
}

static int NetDbgPrint(char * strData)
{
	/* �����ݷ��뻷�λ����� */
	int i;

	for(i = 0; i < strlen(strData); i++){
		if(-1 == PutData(strData[i]))
			break;
	}

	/* ����Ѿ��пͻ��������˱��������ˣ��Ͱ�����ͨ�����緢�͸��ͻ��� */
	/* ����netprint�ķ����߳� */
	pthread_mutex_lock(&g_tNetDbgSendMutex);
	pthread_cond_signal(&g_tNetDbgSendConVar);
	pthread_mutex_unlock(&g_tNetDbgSendMutex);

	return i;
}

static T_DebugOpr g_tNetDbgOpr = {
	.name       = "netprint",
	.isCanUse   = 1,
	.DebugInit  = NetDbgInit,
	.DebugExit  = NetDbgExit,
	.DebugPrint = NetDbgPrint,
};

int NetPrintInit(void)
{
	return RegisterDebugOpr(&g_tNetDbgOpr);
}


