#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

/*******
�÷�
firstdrv_test on //����
firstdrv_test off//����

*******/

int main(int argc, char **argv)
{
	int fd;
	int val  = 1;
	fd = open("/dev/xyz", O_RDWR);
	if(fd < 0)
		printf("can't open!\n");

	//��ʾ�û�
	if(argc != 2)
		{
			printf("Usage :\n");
			printf("%s <on | off>\n", argv[0]);
			return 0;
		}
	//�ж��û��������on����off�?
	if(strcmp(argv[1],"on")==0)
		{
			val = 1;//д1��GPIO�����
		}
	else
		{
			val = 0;//д0��GPIO�����
		}
	
	write(fd, &val ,4);
	return 0;
}


