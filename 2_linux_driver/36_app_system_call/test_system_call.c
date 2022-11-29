

#include <errno.h>
#include <unistd.h>

#define __NR_SYSCALL_BASE	0x900000

void hello (char *buf, int count)
{
  asm ("mov r0, %0\n"	/* save the argment in r0 */
  	   "mov r1, %1\n"	/* save the argment in r1 */
       "swi %2\n"	/* do the system call */
       : 
       : "r"(buf),"r"(count), "i" (__NR_SYSCALL_BASE + 352)
       : "r0", "r1");
}

int main(int argc, char **argv)
{
	printf("in app, call hello\n");
	hello("alex zhong", sizeof("alex zhong"));//
//	hello("www.100ask.net", 15);

	return 0;
}

