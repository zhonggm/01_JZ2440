
#include <config.h>
#include <input_manager.h>
#include <string.h>

static PT_InputOpr g_ptInputOprHead;

/* 定义一个全局变量，这2个输入线程把值存进去，主线程就去读这个值 */
static T_InputEvent g_tInputEvent;

/* 定义并初始化互斥锁 */
static pthread_mutex_t g_tMutex = PTHREAD_MUTEX_INITIALIZER; 
/* 定义并初始化条件变量 */
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

/* 线程处理函数 */
static void *InputEventThreadFunction(void *pVoid)
{
	T_InputEvent tInputEvent;
	int iError;
	
	/* 定义函数指针 */
	int (*GetInputEvent)(PT_InputEvent ptInputEvent);
	GetInputEvent = (int (*)(PT_InputEvent))pVoid;

	while(1){
		if(0 == GetInputEvent(&tInputEvent)){
			
			/* 访问临界资源前，先获得互斥锁 */
			pthread_mutex_lock(&g_tMutex);

				/* 唤醒主线程前，把获得的tInputEvent
				   的值赋给一个全局变量g_tInputEvent */
				g_tInputEvent = tInputEvent;

				/* 发送条件变量，唤醒主线程 */
				pthread_cond_signal(&g_tConVar);

			/* 访问临界资源完后，释放互斥锁 */
			pthread_mutex_unlock(&g_tMutex);
		}
	}

	return NULL;
}

/* 所有输入设备初始化 */
int AllInputDevicesInit(void)
{
	PT_InputOpr ptTmp = g_ptInputOprHead;
	int iError = -1;

	while(ptTmp){
		if(0 == ptTmp->DeviceInit()){
			/* 创建子线程 */
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
	/* 休眠 */
	pthread_mutex_lock(&g_tMutex);
	pthread_cond_wait(&g_tConVar, &g_tMutex);

	/* 被唤醒后，返回数据 */
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

