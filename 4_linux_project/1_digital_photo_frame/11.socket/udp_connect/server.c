
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
	
	/*====================================  1  =============================================*/
	/* �������˿�ʼ����socket������ */
	/*
	AF_INET    : ˵����������������ڵ��������õ�ͨѶЭ����AF_INET��
	SOCK_DGRAM: ���������õ���UDPЭ�顣
	0          : ��������ָ����type,��������ط�����һ��ֻҪ��0������Ϳ�����,
	             socketΪ����ͨѶ��������׼��.
	*/
	iSocketServer = socket(AF_INET, SOCK_DGRAM, 0);
	if(-1 == iSocketServer){
		printf("socket error!\n");
		return -1;
	}
	
	/*====================================  2  =============================================*/
	/* ����������� struct sockaddr_in �ṹ */ 
	tSocketServerAddr.sin_family      = AF_INET;
	tSocketServerAddr.sin_port        = htons(SERVER_PORT);/*host to net, short*/
	tSocketServerAddr.sin_addr.s_addr = INADDR_ANY;/*��ʾ���Ժ��κε�����ͨ��*/
	memset(tSocketServerAddr.sin_zero, 0, 8);
	
	/*====================================  3  =============================================*/
	 /* ����sockfd������  */ 
	iRet = bind(iSocketServer, (const struct sockaddr *)&tSocketServerAddr, sizeof(struct sockaddr));
	if(-1 == iRet){
		printf("bind error!\n");
		return -1;
	}

	/*====================================  5  =============================================*/
	/* ����������,ֱ���ͻ�����������  */
	while(1){
		iAddrLen = sizeof(struct sockaddr);
		iRecvLen = recvfrom(iSocketServer, ucRecvBuf, RECV_BUF_MAX_SIZE - 1, 0, (struct sockaddr *)&tSocketClientAddr, &iAddrLen);
		if(iRecvLen > 0){
			ucRecvBuf[iRecvLen] = '\0';
			printf("Get Msg From %s : %s\n", inet_ntoa(tSocketClientAddr.sin_addr), ucRecvBuf);
		}
	}
	
	/*======================================  8  ===========================================*/
	close(iSocketServer);/* ���ͨѶ�Ѿ����� */
	
	return 0;
}

