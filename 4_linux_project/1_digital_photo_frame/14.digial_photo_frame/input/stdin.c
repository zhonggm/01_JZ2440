
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
	char c;

	ptInputEvent->iType = INPUT_TYPE_STDIN;
	
	/*�ȶ�ȡ����*/
	c = fgetc(stdin);/* ����Ϣֱ�������� */
	/* ��¼��׼���뷢��ʱ�� */
	gettimeofday(&ptInputEvent->tTime, NULL);
	/*�ٴ�������*/
	switch(c){
		case 'u': ptInputEvent->iVal = INPUT_VALUE_UP;      break; /* �Ϸ� */
		case 'n': ptInputEvent->iVal = INPUT_VALUE_DOWN;    break; /* �·� */
		case 'q': ptInputEvent->iVal = INPUT_VALUE_EXIT;    break; /* �˳� */
		default : ptInputEvent->iVal = INPUT_VALUE_UNKNOWN; break; /* ��Ч */
	}
	
	return 0;	
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

