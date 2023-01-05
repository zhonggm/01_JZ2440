
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <stdio.h>

#include <signal.h>

#define SERVER_PORT 	  8888
#define 	BACKLOG       10
#define RECV_BUF_MAX_SIZE 1024

#if 0
不过由于系统的兼容性,我们一般不用这个头文件,
而使用另外一个结构(struct sockaddr_in)来代替
struct sockaddr_in{
	unsigned short		  sin_family;	  
	unsigned short int	  sin_port;
	struct in_addr		  sin_addr;
	unsigned char 		  sin_zero[8];
}
我们主要使用Internet所以
        sin_family一般为AF_INET,
        sin_addr设置为INADDR_ANY表示可以和任何的主机通信,
        sin_port是我们要监听的端口号.
        sin_zero[8]是用来填充的. 
  bind将本地的端口同socket返回的文件描述符捆绑在一起.
  成功是返回0,失败的情况和socket一样 
#endif

/* socket
 * bind
 * listen
 * accept
 * send / recv
 */
int main(int argc, char *argv[])
{
	int iSocketServer;/*服务器文件描述符*/
	int iSocketClient;/*客户端文件描述符*/
	struct sockaddr_in tSocketServerAddr;/*服务器信息*/
	struct sockaddr_in tSocketClientAddr;/*客户端信息*/
	int iRet;
	int iAddrLen;	

	int iRecvLen;/*客户端实际发送的数据长度*/
	unsigned char ucRecvBuf[RECV_BUF_MAX_SIZE];/*接收客户端数据内容存储区*/

	int iClientNum = -1;/*连接的客户端计数*/

	/* 子进程退出之后会给父进程发送一个信号，这个信号的处理函数是SIG_IGN */
	signal(SIGCHLD, SIG_IGN);
	
	/*====================================  1  =============================================*/
	/* 服务器端开始建立socket描述符 */
	/*
	AF_INET    : 说明我们网络程序所在的主机采用的通讯协族是AF_INET。
	SOCK_STREAM: 表明我们用的是TCP协议。
	0          : 由于我们指定了type,所以这个地方我们一般只要用0来代替就可以了,
	             socket为网络通讯做基本的准备.
	*/
	iSocketServer = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == iSocketServer){
		printf("socket error!\n");
		return -1;
	}
	
	/*====================================  2  =============================================*/
	/* 服务器端填充 struct sockaddr_in 结构  */ 
	tSocketServerAddr.sin_family      = AF_INET;
	tSocketServerAddr.sin_port        = htons(SERVER_PORT);/*host to net, short*/
	tSocketServerAddr.sin_addr.s_addr = INADDR_ANY;/*表示可以和任何的主机通信*/
	memset(tSocketServerAddr.sin_zero, 0, 8);
	
	/*====================================  3  =============================================*/
	 /* 捆绑sockfd描述符  */ 
	iRet = bind(iSocketServer, (const struct sockaddr *)&tSocketServerAddr, \ 
		sizeof(struct sockaddr));
	if(-1 == iRet){
		printf("bind error!\n");
		return -1;
	}
	/*====================================  4  =============================================*/
	/* 监听sockfd描述符  */
	iRet = listen(iSocketServer, BACKLOG);
	if (-1 == iRet)
	{
		printf("listen error!\n");
		return -1;
	}
	/*====================================  5  =============================================*/
	/* 服务器阻塞,直到客户程序建立连接  */
	while(1){
		iAddrLen = sizeof(struct sockaddr);
		iSocketClient= accept(iSocketServer, (struct sockaddr *)&tSocketClientAddr, \
			&iAddrLen);
	/*======================================  6  ===========================================*/
	/* 连接成功提示 */
		if(-1 != iSocketClient){
			iClientNum++;
			
			printf("Get connection from client %d : %s\n", iClientNum, \
				inet_ntoa(tSocketClientAddr.sin_addr));

	/*===================================  7  ==============================================*/
	/* 连接成功一个客户端，就创建一个子进程来处理这个客户端发来的数据 */
			if(!fork()){
				/* 子进程空间 */
				while(1){
					/* 接收客户端发来的数据并打印显示出来 */
					iRecvLen = recv(iSocketClient, ucRecvBuf, RECV_BUF_MAX_SIZE - 1, 0);
					if(iRecvLen <= 0){
						close(iSocketClient);/* 这个通讯已经结束     */
						return -1;
					}else{
						ucRecvBuf[iRecvLen] = '\0';
						printf("Get Msg From Client %d : %s\n", iClientNum, ucRecvBuf);
					}
				}
			}
			
		}
	}
	
	/*======================================  8  ===========================================*/
	close(iSocketServer);/* 这个通讯已经结束     */
	
	return 0;
}

