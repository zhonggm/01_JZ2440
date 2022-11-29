
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_REPLAY   0
#define INPUT_TAG      1

/* Usage:
 * ./input_replay write <file>
 * ./input_replay replay
 * ./input_repaly tag <string>
 */

void print_usage(char *file)
{
	printf("Usage:\n");
	printf("%s write <file>\n", file);//将测试数据写入驱动
	printf("%s replay\n", file);	  //回放测试数据
	printf("%s tag <string>\n", file);//手动区分开mymsg里要上报的数据，以string为标志
}

int main(int argc, char **argv)
{
	int fd;
	int fd_data;
	int buf[100];
	int len;
	
	if (argc != 2 && argc != 3)
	{
		print_usage(argv[0]);
		return -1;
	}

	fd = open("/dev/input_emu", O_RDWR);
	if (fd < 0)
	{
		printf("can't open /dev/input_emu\n");
		return -1;
	}

	if (strcmp(argv[1], "replay") == 0)
	{
		ioctl(fd, INPUT_REPLAY);
	}
	else if (strcmp(argv[1], "write") == 0)
	{
		if (argc != 3)
		{
			print_usage(argv[0]);
			return -1;
		}

		fd_data = open(argv[2], O_RDONLY);
		if (fd_data < 0)
		{
			printf("can't open %s\n", argv[2]);
			return -1;
		}

		while (1)
		{
			len = read(fd_data, buf, 100);
			if (len == 0)
			{
				printf("write to replay_buf over\n");
				break;
			}
			else
			{
				write(fd, buf, len);				
			}
		}
	}
	else if (strcmp(argv[1], "tag") == 0)
	{
		if (argc != 3)
		{
			print_usage(argv[0]);
			return -1;
		}
		ioctl(fd, INPUT_TAG, argv[2]);
	}
	else
	{
		print_usage(argv[0]);
		return -1;
	}

	return 0;
	
}


