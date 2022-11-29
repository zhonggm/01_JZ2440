#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

/*******
用法
firstdrv_test on //灯亮
firstdrv_test off//灯灭

*******/

int main(int argc, char **argv)
{
	int fd;
	int val  = 1;
	fd = open("/dev/xyz", O_RDWR);
	if(fd < 0)
		printf("can't open!\n");

	//提示用户
	if(argc != 2)
		{
			printf("Usage :\n");
			printf("%s <on | off>\n", argv[0]);
			return 0;
		}
	//判断用户输入的是on还是off�?
	if(strcmp(argv[1],"on")==0)
		{
			val = 1;//写1，GPIO输出低
		}
	else
		{
			val = 0;//写0，GPIO输出高
		}
	
	write(fd, &val ,4);
	return 0;
}


