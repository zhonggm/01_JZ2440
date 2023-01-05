
#include <config.h>
#include <input_manager.h>
#include <string.h>

static PT_InputOpr g_ptInputOprHead;

/* ����һ��ȫ�ֱ�������2�������̰߳�ֵ���ȥ�����߳̾�ȥ�����ֵ */
static T_InputEvent g_tInputEvent;

/* ���岢��ʼ�������� */
static pthread_mutex_t g_tMutex = PTHREAD_MUTEX_INITIALIZER; 
/* ���岢��ʼ���������� */
static pthread_cond_t g_tConVar = PTHREAD_COND_INITIALIZER; 

int RegisterInputOpr(PT_InputOpr ptInputOpr)
{
	PT_InputOpr ptTmp;

	if (!g_ptInputOprHead){
		g_ptInputOprHead = ptInputOpr;
		ptInputOpr->ptNext = NULL;
	}else{
		ptTmp = g_ptInputOprHead;
		while (ptTmp->ptNext)
			ptTmp = ptTmp->ptNext;
		
		ptTmp->ptNext	   = ptInputOpr;
		ptInputOpr->ptNext = NULL;
	}
     
	return 0;
}

void ShowInputOpr(void)
{
	int i = 0;
	PT_InputOpr ptTmp = g_ptInputOprHead;

	while (ptTmp){
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}

/* �̴߳����� */
static void *InputEventThreadFunction(void *pVoid)
{
	T_InputEvent tInputEvent;
	int iError;
	
	/* ���庯��ָ�� */
	int (*GetInputEvent)(PT_InputEvent ptInputEvent);
	GetInputEvent = (int (*)(PT_InputEvent))pVoid;

	while(1){
		if(0 == GetInputEvent(&tInputEvent)){
			
			/* �����ٽ���Դǰ���Ȼ�û����� */
			pthread_mutex_lock(&g_tMutex);

				/* �������߳�ǰ���ѻ�õ�tInputEvent
				   ��ֵ����һ��ȫ�ֱ���g_tInputEvent */
				g_tInputEvent = tInputEvent;

				/* ���������������������߳� */
				pthread_cond_signal(&g_tConVar);

			/* �����ٽ���Դ����ͷŻ����� */
			pthread_mutex_unlock(&g_tMutex);
		}
	}

	return NULL;
}

/* ���������豸��ʼ�� */
int AllInputDevicesInit(void)
{
	PT_InputOpr ptTmp = g_ptInputOprHead;
	int iError = -1;

	while(ptTmp){
		if(0 == ptTmp->DeviceInit()){
			/* �������߳� */
			pthread_create(&ptTmp->tThreadID, NULL,
				InputEventThreadFunction, ptTmp->GetInputEvent);
			iError = 0;
		}
		ptTmp = ptTmp->ptNext;
	}

	return iError;
}

int GetInputEvent(PT_InputEvent ptInputEvent)
{
	/* ���� */
	pthread_mutex_lock(&g_tMutex);
	pthread_cond_wait(&g_tConVar, &g_tMutex);

	/* �����Ѻ󣬷������� */
	*ptInputEvent = g_tInputEvent;
	pthread_mutex_unlock(&g_tMutex);	
	
	return 0;
}

int InputInit(void)
{
	int iError;

	iError = StdinInit();
	iError |= TouchScreenInit();

	return iError;
}

