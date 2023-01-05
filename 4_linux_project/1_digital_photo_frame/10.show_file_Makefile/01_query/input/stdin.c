
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
    /* ��������ݾͶ�ȡ����������
     * ���û�����ݾ����̷��أ����ȴ���
     */

	/* select, poll ���Բο�unix�����߼����*/
	
	struct timeval tTV;
	fd_set tFds;
	char c;

	tTV.tv_sec  = 0;
	tTV.tv_usec = 0;
	FD_ZERO(&tFds);
	FD_SET(STDIN_FILENO, &tFds); //STDIN_FILENO is 0
	select(STDIN_FILENO + 1, &tFds, NULL, NULL, &tTV);

	/* ��� tFds ���ĳһλ�������ˣ�
	   �ͱ����������ˣ�
	   ��ȡ���ݣ�
	*/
	if(FD_ISSET(STDIN_FILENO, &tFds)){
		/* ��¼��׼���뷢��ʱ�� */
		ptInputEvent->iType = INPUT_TYPE_STDIN;
		gettimeofday(&ptInputEvent->tTime, NULL);
		/*�ȶ�ȡ����*/
		c = fgetc(stdin);
		/*�ٴ�������*/
		switch(c){
			case 'u': ptInputEvent->iVal = INPUT_VALUE_UP;      break; /* �Ϸ� */
			case 'n': ptInputEvent->iVal = INPUT_VALUE_DOWN;    break; /* �·� */
			case 'q': ptInputEvent->iVal = INPUT_VALUE_EXIT;    break; /* �˳� */
			default : ptInputEvent->iVal = INPUT_VALUE_UNKNOWN; break; /* ��Ч */
		}
		return 0;
	}else{
		return -1;  //û�����ݾ�ֱ�ӷ���-1
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

