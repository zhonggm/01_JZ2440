
#include <input_manager.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>

static int StdinDevInit(void)
{
	struct termios tTtyState;
	 
	//get the terminal state
	tcgetattr(STDIN_FILENO, &tTtyState);
 
	//turn off canonical mode
	tTtyState.c_lflag &= ~ICANON;
	//minimum of number input read.
	tTtyState.c_cc[VMIN] = 1;
		
	//set the terminal attributes.
	tcsetattr(STDIN_FILENO, TCSANOW, &tTtyState);

	return 0;
}

static int StdinDevExit(void)
{
	struct termios tTtyState;
	 
	//get the terminal state
	tcgetattr(STDIN_FILENO, &tTtyState);
 
	//turn on canonical mode
	tTtyState.c_lflag |= ICANON;
		
	//set the terminal attributes.
	tcsetattr(STDIN_FILENO, TCSANOW, &tTtyState);

	return 0;
}

static int StdinGetInputEvent(PT_InputEvent ptInputEvent)
{
    /* 如果有数据就读取、处理、返回
     * 如果没有数据就立刻返回，不等待。
     */

	/* select, poll 可以参考unix环境高级编程*/
	
	struct timeval tTV;
	fd_set tFds;
	char c;

	tTV.tv_sec  = 0;
	tTV.tv_usec = 0;
	FD_ZERO(&tFds);
	FD_SET(STDIN_FILENO, &tFds); //STDIN_FILENO is 0
	select(STDIN_FILENO + 1, &tFds, NULL, NULL, &tTV);

	/* 如果 tFds 里的某一位被设置了，
	   就表明有数据了，
	   读取数据，
	*/
	if(FD_ISSET(STDIN_FILENO, &tFds)){
		/* 记录标准输入发生时刻 */
		ptInputEvent->iType = INPUT_TYPE_STDIN;
		gettimeofday(&ptInputEvent->tTime, NULL);
		/*先读取数据*/
		c = fgetc(stdin);
		/*再处理数据*/
		switch(c){
			case 'u': ptInputEvent->iVal = INPUT_VALUE_UP;      break; /* 上翻 */
			case 'n': ptInputEvent->iVal = INPUT_VALUE_DOWN;    break; /* 下翻 */
			case 'q': ptInputEvent->iVal = INPUT_VALUE_EXIT;    break; /* 退出 */
			default : ptInputEvent->iVal = INPUT_VALUE_UNKNOWN; break; /* 无效 */
		}
		return 0;
	}else{
		return -1;  //没有数据就直接返回-1
	}
}

static T_InputOpr g_tStdinOpr = {
	.name          = "stdin",
	.DeviceInit    = StdinDevInit,
	.DeviceExit    = StdinDevExit,
	.GetInputEvent = StdinGetInputEvent,
};

int StdinInit(void)
{
	return RegisterInputOpr(&g_tStdinOpr);
}

