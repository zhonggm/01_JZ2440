
#include <config.h>
#include <input_manager.h>
#include <disp_manager.h>

#include <stdlib.h>
#include <tslib.h>

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

static struct tsdev *g_ptTsDev;

static int giXres;
static int giYres;

static int TouchScreenDevInit(void)
{
	char *pcTsDeviceName = NULL;

	if( (pcTsDeviceName = getenv("TSLIB_TSDEVICE")) != NULL ) {
		g_ptTsDev = ts_open(pcTsDeviceName, 0);
	} else {
		g_ptTsDev = ts_open("/dev/event0", 1);
	}

	if (!g_ptTsDev) {
		DBG_PRINTF(APP_ERR"ts_open error!\n");
		return -1;
	}

	if (ts_config(g_ptTsDev)) {
		DBG_PRINTF(APP_ERR"ts_config error!\n");
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


static int TouchScreenGetInputEvent(PT_InputEvent ptInputEvent)
{
	struct ts_sample tSamp;	
	struct ts_sample tSampPressed;
	struct ts_sample tSampReleased;
	
	int iRet;
	int bStartPressed = 0;/* 一定要初始化为0 */
	int iDelta;

	while(1){
		/* 因为打开是用阻塞方式，所以如果无数据，则休眠。 */
		iRet = ts_read(g_ptTsDev, &tSamp, 1);/* 采样 */
		
		if(1 == iRet){/* 按下 */
			
			if((tSamp.pressure > 0)&&(0 == bStartPressed)){
				
				/* 记录刚开始压下的点 */
				tSampPressed = tSamp;
				bStartPressed = 1;/*表示按住了*/		
			}

			if(tSamp.pressure <= 0){

				/* 按下后松开 */
				tSampReleased = tSamp;
				
				/* 处理数据 */
				if (!bStartPressed){

					return -1;
					
				}else{
					/* 计算X坐标位移差 */
					iDelta = tSampReleased.x - tSampPressed.x;
					ptInputEvent->tTime = tSampReleased.tv;/* 记录时间 */
					ptInputEvent->iType = INPUT_TYPE_TOUCHSCREEN;
					
					if (iDelta > giXres/5){
						/* 右滑，翻到上一页 */
						ptInputEvent->iVal = INPUT_VALUE_UP;
						
					}else if (iDelta < 0 - giXres/5){
						/* 左滑，翻到下一页 */
						ptInputEvent->iVal = INPUT_VALUE_DOWN;
						
					}else{
						ptInputEvent->iVal = INPUT_VALUE_UNKNOWN;
						
					}
					
					return 0;
				}
			}				
		}else{
			/* 没按下 */
			return -1;
		}
	}

	return 0;
}

static T_InputOpr g_tTouchScreenOpr = {
	.name          = "touchscreen",
	.DeviceInit    = TouchScreenDevInit,
	.DeviceExit    = TouchScreenDevExit,
	.GetInputEvent = TouchScreenGetInputEvent,
};

int TouchScreenInit(void)
{
	return RegisterInputOpr(&g_tTouchScreenOpr);
}

