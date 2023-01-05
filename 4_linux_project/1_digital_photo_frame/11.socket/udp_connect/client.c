#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <stdio.h>

#define SERVER_PORT 	  8888
#define SEND_BUF_MAX_SIZE 1024

/* socket
 * connect
 * send / recv
 */
int main(int argc, char *argv[])
{
	int iSocketClient;/*客户端文件描述符*/
	struct sockaddr_in tSocketServerAddr;/*服务器信息*/

	int iRet;
	unsigned char ucSendBuf[SEND_BUF_MAX_SIZE];/*本客户端发送数据内容存储区*/

	int iSendLen;
	
	if(argc != 2){
		printf("Usage:\n");
		printf("%s <server_ip>\n", argv[0]);		
		return -1;
	}
	/*=======================================  1  ==========================================*/
	/* 客户程序开始建立 sockfd描述符  */
	iSocketClient = socket(AF_INET, SOCK_DGRAM, 0);

	/*=======================================  2  ==========================================*/
	/* 客户程序填充服务端的资料  */
	tSocketServerAddr.sin_family      = AF_INET;
	tSocketServerAddr.sin_port        = htons(SERVER_PORT);/*host to net, short*/
//	tSocketServerAddr.sin_addr.s_addr = INADDR_ANY;/* 表示可以和任何的主机通信 */
	if(0 == inet_aton(argv[1], &tSocketServerAddr.sin_addr)){
		printf("invalid server_ip\n");
		return -1;
	}
	memset(tSocketServerAddr.sin_zero, 0, 8);

	/*=======================================  3  ==========================================*/
	/* 客户程序发起连接请求 */ 
	iRet = connect(iSocketClient, (const struct sockaddr *)&tSocketServerAddr, sizeof(struct sockaddr));
	if(-1 == iRet){
		printf("connect error!\n");
		return -1;
	}

	/*=======================================  4  ==========================================*/
	/* 客户程序发送数据 */ 
	while(1){
		if(fgets(ucSendBuf, SEND_BUF_MAX_SIZE - 1, stdin)){
			
			iSendLen = send(iSocketClient, ucSendBuf, strlen(ucSendBuf), 0);
			if(iSendLen <= 0){
				close(iSocketClient);
				return -1;
			}
		}
	}

	/*=======================================  5  ==========================================*/
	close(iSocketClient);
	
	return 0;
}

