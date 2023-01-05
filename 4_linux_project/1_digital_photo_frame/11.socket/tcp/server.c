
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
��������ϵͳ�ļ�����,����һ�㲻�����ͷ�ļ�,
��ʹ������һ���ṹ(struct sockaddr_in)������
struct sockaddr_in{
	unsigned short		  sin_family;	  
	unsigned short int	  sin_port;
	struct in_addr		  sin_addr;
	unsigned char 		  sin_zero[8];
}
������Ҫʹ��Internet����
        sin_familyһ��ΪAF_INET,
        sin_addr����ΪINADDR_ANY��ʾ���Ժ��κε�����ͨ��,
        sin_port������Ҫ�����Ķ˿ں�.
        sin_zero[8]����������. 
  bind�����صĶ˿�ͬsocket���ص��ļ�������������һ��.
  �ɹ��Ƿ���0,ʧ�ܵ������socketһ�� 
#endif

/* socket
 * bind
 * listen
 * accept
 * send / recv
 */
int main(int argc, char *argv[])
{
	int iSocketServer;/*�������ļ�������*/
	int iSocketClient;/*�ͻ����ļ�������*/
	struct sockaddr_in tSocketServerAddr;/*��������Ϣ*/
	struct sockaddr_in tSocketClientAddr;/*�ͻ�����Ϣ*/
	int iRet;
	int iAddrLen;	

	int iRecvLen;/*�ͻ���ʵ�ʷ��͵����ݳ���*/
	unsigned char ucRecvBuf[RECV_BUF_MAX_SIZE];/*���տͻ����������ݴ洢��*/

	int iClientNum = -1;/*���ӵĿͻ��˼���*/

	/* �ӽ����˳�֮���������̷���һ���źţ�����źŵĴ�������SIG_IGN */
	signal(SIGCHLD, SIG_IGN);
	
	/*====================================  1  =============================================*/
	/* �������˿�ʼ����socket������ */
	/*
	AF_INET    : ˵����������������ڵ��������õ�ͨѶЭ����AF_INET��
	SOCK_STREAM: ���������õ���TCPЭ�顣
	0          : ��������ָ����type,��������ط�����һ��ֻҪ��0������Ϳ�����,
	             socketΪ����ͨѶ��������׼��.
	*/
	iSocketServer = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == iSocketServer){
		printf("socket error!\n");
		return -1;
	}
	
	/*====================================  2  =============================================*/
	/* ����������� struct sockaddr_in �ṹ  */ 
	tSocketServerAddr.sin_family      = AF_INET;
	tSocketServerAddr.sin_port        = htons(SERVER_PORT);/*host to net, short*/
	tSocketServerAddr.sin_addr.s_addr = INADDR_ANY;/*��ʾ���Ժ��κε�����ͨ��*/
	memset(tSocketServerAddr.sin_zero, 0, 8);
	
	/*====================================  3  =============================================*/
	 /* ����sockfd������  */ 
	iRet = bind(iSocketServer, (const struct sockaddr *)&tSocketServerAddr, \ 
		sizeof(struct sockaddr));
	if(-1 == iRet){
		printf("bind error!\n");
		return -1;
	}
	/*====================================  4  =============================================*/
	/* ����sockfd������  */
	iRet = listen(iSocketServer, BACKLOG);
	if (-1 == iRet)
	{
		printf("listen error!\n");
		return -1;
	}
	/*====================================  5  =============================================*/
	/* ����������,ֱ���ͻ�����������  */
	while(1){
		iAddrLen = sizeof(struct sockaddr);
		iSocketClient= accept(iSocketServer, (struct sockaddr *)&tSocketClientAddr, \
			&iAddrLen);
	/*======================================  6  ===========================================*/
	/* ���ӳɹ���ʾ */
		if(-1 != iSocketClient){
			iClientNum++;
			
			printf("Get connection from client %d : %s\n", iClientNum, \
				inet_ntoa(tSocketClientAddr.sin_addr));

	/*===================================  7  ==============================================*/
	/* ���ӳɹ�һ���ͻ��ˣ��ʹ���һ���ӽ�������������ͻ��˷��������� */
			if(!fork()){
				/* �ӽ��̿ռ� */
				while(1){
					/* ���տͻ��˷��������ݲ���ӡ��ʾ���� */
					iRecvLen = recv(iSocketClient, ucRecvBuf, RECV_BUF_MAX_SIZE - 1, 0);
					if(iRecvLen <= 0){
						close(iSocketClient);/* ���ͨѶ�Ѿ�����     */
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
	close(iSocketServer);/* ���ͨѶ�Ѿ�����     */
	
	return 0;
}

