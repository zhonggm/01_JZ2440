#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

/*******
ÓÃ·¨
firstdrv_test on //µÆÁÁ
firstdrv_test off//µÆÃğ

*******/

int main(int argc, char **argv)
{
	int fd;
	int val  = 1;
	fd = open("/dev/xyz", O_RDWR);
	if(fd < 0)
		printf("can't open!\n");

	//ÌáÊ¾ÓÃ»§
	if(argc != 2)
		{
			printf("Usage :\n");
			printf("%s <on | off>\n", argv[0]);
			return 0;
		}
	//ÅĞ¶ÏÓÃ»§ÊäÈëµÄÊÇon»¹ÊÇoffå?
	if(strcmp(argv[1],"on")==0)
		{
			val = 1;//Ğ´1£¬GPIOÊä³öµÍ
		}
	else
		{
			val = 0;//Ğ´0£¬GPIOÊä³ö¸ß
		}
	
	write(fd, &val ,4);
	return 0;
}


