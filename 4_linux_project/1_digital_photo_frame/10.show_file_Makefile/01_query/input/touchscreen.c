
#include <config.h>
#include <input_manager.h>
#include <stdlib.h>
#include <tslib.h>
#include <draw.h>


/* 参考tslib里的ts_print.c */
#if 0
struct timeval{
	__time_t tv_sec; /* Seconds. */
	__suseconds_t tv_usec; /* Microseconds. */
};

struct ts_sample{
	int		x;
	int		y;
	unsigned int pressure;
	struct timeval tv;
};
#endif

struct tsdev *g_ptTsDev;

static int giXres;
static int giYres;

static int TouchScreenDevInit(void)
{
	char *pcTsDeviceName = NULL;

	if( (pcTsDeviceName = getenv("TSLIB_TSDEVICE")) != NULL ) {
		g_ptTsDev = ts_open(pcTsDeviceName, 1);
	} else {
		g_ptTsDev = ts_open("/dev/event0", 1);
	}

	if (!g_ptTsDev) {
		DBG_PRINTF("ts_open error!\n");
		return -1;
	}

	if (ts_config(g_ptTsDev)) {
		DBG_PRINTF("ts_config error!\n");
		return -1;
	}

	/* 
	  * 因为后续输入事件采样信息存在struct ts_sample类型变量里，
	  * 有x、y方向 的坐标值，
            * 要跟触摸屏的分辨率比较判断。
	  */
	if(GetDispResolution(&giXres, &giYres))
		return -1;

	return 0;
}

static int TouchScreenDevExit(void)
{
	return 0;
}

static int IsOutOf500ms(struct timeval *ptPreTime, struct timeval *ptNowTime)
{
	int iPreMs;
	int iNowMs;

	iPreMs = ptPreTime->tv_sec * 1000 + ptPreTime->tv_usec / 1000;
	iNowMs = ptNowTime->tv_sec * 1000 + ptNowTime->tv_usec / 1000;

	return (iNowMs > iPreMs + 500);
}

static int TouchScreenGetInputEvent(PT_InputEvent ptInputEvent)
{
	struct ts_sample tSamp;	
	static struct timeval tPreTime;
	int iRet;
	
	iRet = ts_read(g_ptTsDev, &tSamp, 1);

	if (iRet < 0) {
		return -1;
	}

	/* 处理数据 */
	if(IsOutOf500ms(&tPreTime, &tSamp.tv)){
		/* 如果此次触摸事件发生的时间，
		  * 距上次事件超过了500ms。 
		  */

		/* 将此次时刻更新为下次的前事件时刻 */
		tPreTime = tSamp.tv;
		ptInputEvent->tTime = tSamp.tv;
		ptInputEvent->iType = INPUT_TYPE_TOUCHSCREEN;
		
		if(tSamp.y < giYres / 3)
			ptInputEvent->iVal = INPUT_VALUE_UP;/* 设置为上翻 */
		else if(tSamp.y > 2 * giYres / 3)
			ptInputEvent->iVal = INPUT_VALUE_DOWN;/* 设置为下翻 */
		else
			ptInputEvent->iVal = INPUT_VALUE_UNKNOWN;/* 设置为无效 */

		return 0;
	}else{
		return -1;
	}
}


static T_InputOpr g_tTouchScreenOpr = {
	.name = "touchscreen",
	.DeviceInit = TouchScreenDevInit,
	.DeviceExit = TouchScreenDevExit,
	.GetInputEvent = TouchScreenGetInputEvent,
};

int TouchScreenInit(void)
{
	return RegisterInputOpr(&g_tTouchScreenOpr);
}


