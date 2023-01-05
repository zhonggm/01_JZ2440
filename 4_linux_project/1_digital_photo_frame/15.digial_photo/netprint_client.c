#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <stdio.h>

#define SERVER_PORT 	  5678

/*  
 * ./netprint_client <server_ip> dbglevel=<0-9> 
 * ./netprint_client <server_ip> stdout=0|1
 * ./netprint_client <server_ip> netprint=0|1
 * ./netprint_client <server_ip> show //setclient, ���ҽ��մ�ӡ��Ϣ
 */
int main(int argc, char *argv[])
{
	int iSocketClient;/*�ͻ����ļ�������*/
	struct sockaddr_in tSocketServerAddr;/*��������Ϣ*/

	int iRet;
	unsigned char ucRecvBuf[1000];/* ���ͻ��˽����������ݴ洢�� */

	int iSendLen;
	int iRecvLen;
	socklen_t iAddrLen;
	
	if(argc != 2){
		printf("Usage:\n");
		printf("%s <server_ip> dbglevel=<0-9>\n", argv[0]);	
		printf("%s <server_ip> stdout=0|1\n",     argv[0]);	
		printf("%s <server_ip> netprint=0|1\n",   argv[0]);	
		printf("%s <server_ip> show\n",           argv[0]);	
		
		return -1;
	}
	/*=======================================  1  ==========================================*/
	/* �ͻ�����ʼ���� sockfd ������  */
	iSocketClient = socket(AF_INET, SOCK_DGRAM, 0);

	/*=======================================  2  ==========================================*/
	/* �ͻ�����������˵�����  */
	tSocketServerAddr.sin_family      = AF_INET;
	tSocketServerAddr.sin_port        = htons(SERVER_PORT);/*host to net, short*/
//	tSocketServerAddr.sin_addr.s_addr = INADDR_ANY;/* ��ʾ���Ժ��κε�����ͨ�� */
	if(0 == inet_aton(argv[1], &tSocketServerAddr.sin_addr)){
		printf("invalid server_ip\n");
		return -1;
	}
	memset(tSocketServerAddr.sin_zero, 0, 8);
	
	if(0 == strcmp(argv[2], "show")){
		/* �������� */
		iAddrLen = sizeof(struct sockaddr);
		iSendLen = sendto(iSocketClient, "setclient", 9, 0,
			(const struct sockaddr *)&tSocketServerAddr, iAddrLen);
		while(1){
			/* ѭ��: ����������ݣ���ӡ���� */
			iAddrLen = sizeof(struct sockaddr);
			iRecvLen = recvfrom(iSocketClient, ucRecvBuf, 999, 0, \
				(struct sockaddr *)&tSocketServerAddr, &iAddrLen);
			if(iRecvLen > 0){
				ucRecvBuf[iRecvLen] = '\0';
				printf("%s\n", ucRecvBuf);
			}
		}
	}else{
		/* �������� */
		iAddrLen = sizeof(struct sockaddr);
		iSendLen = sendto(iSocketClient, argv[2], strlen(argv[2]), 0,\
			(const struct sockaddr *)&tSocketServerAddr, iAddrLen);
	}
	
	return 0;
}

