
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
不过由于系统的兼容性,我们一般不用这个头文件,
而使用另外一个结构(struct sockaddr_in)来代替
	struct sockaddr_in{
		unsigned short		  sin_family;	  
		unsigned short int	  sin_port;
		struct in_addr		  sin_addr;
		unsigned char		  sin_zero[8];
	};


我们主要使用Internet所以
        sin_family一般为AF_INET,
        sin_addr设置为INADDR_ANY表示可以和任何的主机通信,
        sin_port是我们要监听的端口号.
        sin_zero[8]是用来填充的. 
  bind将本地的端口同socket返回的文件描述符捆绑在一起.
  成功是返回0,失败的情况和socket一样 
#endif

/* 我们用UDP，把开发板当成server端。*/
#define SERVER_PORT 	    5678
#define PRINT_BUF_MAX_SIZE (16*1024)

static int g_iSocketServer;
static struct sockaddr_in g_tSocketServerAddr;
static struct sockaddr_in g_tSocketClientAddr;

static char *g_pcNetPrintBuf;
static int g_iWritePos = 0;
static int g_iReadPos  = 0;

static int g_iHaveConnected = 0;/* 已连接标志 */

static pthread_t g_tSendTreadID;//发送线程ID
static pthread_t g_tRecvTreadID;//接收线程ID

static pthread_mutex_t g_tNetDbgSendMutex  = PTHREAD_MUTEX_INITIALIZER;//互诉锁
static pthread_cond_t  g_tNetDbgSendConVar = PTHREAD_COND_INITIALIZER; //条件变量

static int IsEmpty(void);
static int GetData(char *pcVal);

/* 发送线程处理函数 */
static void *NetDbgSendThreadFunction(void *pVoid)
{
	char strTmpBuf[512];/* 把缓冲区里数据读取并存在这里以便用于发送 */
	char cVal;
	int i;
	int iAddrLen;
	int iSendLen;
	
	while (1){
		/* 平时休眠,等待条件变量 g_tNetDbgSendConVar 被发送过来 */
		pthread_mutex_lock(&g_tNetDbgSendMutex);/* 获得互诉锁 */
		pthread_cond_wait(&g_tNetDbgSendConVar, &g_tNetDbgSendMutex);	
		pthread_mutex_unlock(&g_tNetDbgSendMutex);/* 释放互诉锁 */

		while (g_iHaveConnected && !IsEmpty()){
			i = 0;

			/* 把环形缓冲区的数据取出来,最多取512字节 */
			while ((i < 512) && (0 == GetData(&cVal))){
				strTmpBuf[i] = cVal;
				i++;
			}
			
			/* 执行到这里, 表示被唤醒 */
			/* 用sendto函数发送打印信息给已连接的客户端 */
			iAddrLen = sizeof(struct sockaddr);
			iSendLen = sendto(g_iSocketServer, strTmpBuf, i, 0,
		        (const struct sockaddr *)&g_tSocketClientAddr, iAddrLen);
		}
	}
	
	return NULL;
}

/* 接收线程处理函数 */
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
			
			/* 解析数据:
			 * setclient            : 设置接收打印信息的客户端
			 * dbglevel=0,1,2...    : 修改打印级别
			 * stdout=0             : 关闭stdout打印
			 * stdout=1             : 打开stdout打印
			 * netprint=0           : 关闭netprint打印
			 * netprint=1           : 打开netprint打印
			 */
			if (0 == strcmp(ucRecvBuf, "setclient")){
				g_tSocketClientAddr = tSocketClientAddr;//更新客户端信息
				g_iHaveConnected    = 1;
			}else if (0 == strncmp(ucRecvBuf, "dbglevel=", 9)){
				SetDbgLevel(ucRecvBuf);//更新打印级别
			}else{
				SetDbgChanel(ucRecvBuf);//更新打印方式开关
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
	g_tSocketServerAddr.sin_addr.s_addr = INADDR_ANY;/*表示可以和任何的主机通信*/
	memset(g_tSocketServerAddr.sin_zero, 0, 8);

	iRet = bind(g_iSocketServer,(const struct sockaddr *)&g_tSocketServerAddr,\
		sizeof(struct sockaddr));
	if(-1 == iRet){
		printf("bind error!\n");
		return -1;
	}

	/* bind 成功后再分配打印缓冲使用的空间 */
	g_pcNetPrintBuf = malloc(PRINT_BUF_MAX_SIZE);
	if(NULL == g_pcNetPrintBuf){
		close(g_iSocketServer);
		return -1;
	}

	/* 创建netprint发送线程: 它用来发送打印信息给客户端 */
	pthread_create(&g_tSendTreadID, NULL, NetDbgSendThreadFunction, NULL);			
	
	/* 创建netprint接收线否: 用来接收控制信息,比如修改打印级别,打开/关闭打印 */
	pthread_create(&g_tRecvTreadID, NULL, NetDbgRecvThreadFunction, NULL);		
	
	return 0;
}

static int NetDbgExit(void)
{
	close(g_iSocketServer);
	free(g_pcNetPrintBuf);

	return 0;
}

/* 打印缓冲区为满 */
static int IsFull(void)
{
	return (g_iReadPos == ((g_iWritePos + 1) % PRINT_BUF_MAX_SIZE));
}

/* 打印缓冲区为空 */
static int IsEmpty(void)
{
	return (g_iReadPos == g_iWritePos);
}

/* 打印缓冲区数据填充*/
/* 返回 0 填充成功。
 * 返回-1 填充失败。
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

/* 打印缓冲区数据获取 */
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
	/* 把数据放入环形缓冲区 */
	int i;

	for(i = 0; i < strlen(strData); i++){
		if(-1 == PutData(strData[i]))
			break;
	}

	/* 如果已经有客户端连接了本服务器端，就把数据通过网络发送给客户端 */
	/* 唤醒netprint的发送线程 */
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


