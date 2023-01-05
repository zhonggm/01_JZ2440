
#include <config.h>
#include <input_manager.h>
#include <disp_manager.h>

#include <stdlib.h>
#include <tslib.h>

/* �ο�tslib���ts_print.c */
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
	  * ��Ϊ���������¼�������Ϣ����struct ts_sample���ͱ����
	  * ��x��y���� ������ֵ��
            * Ҫ���������ķֱ��ʱȽ��жϡ�
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
	int bStartPressed = 0;/* һ��Ҫ��ʼ��Ϊ0 */
	int iDelta;

	while(1){
		/* ��Ϊ������������ʽ��������������ݣ������ߡ� */
		iRet = ts_read(g_ptTsDev, &tSamp, 1);/* ���� */
		
		if(1 == iRet){/* ���� */
			
			if((tSamp.pressure > 0)&&(0 == bStartPressed)){
				
				/* ��¼�տ�ʼѹ�µĵ� */
				tSampPressed = tSamp;
				bStartPressed = 1;/*��ʾ��ס��*/		
			}

			if(tSamp.pressure <= 0){

				/* ���º��ɿ� */
				tSampReleased = tSamp;
				
				/* �������� */
				if (!bStartPressed){

					return -1;
					
				}else{
					/* ����X����λ�Ʋ� */
					iDelta = tSampReleased.x - tSampPressed.x;
					ptInputEvent->tTime = tSampReleased.tv;/* ��¼ʱ�� */
					ptInputEvent->iType = INPUT_TYPE_TOUCHSCREEN;
					
					if (iDelta > giXres/5){
						/* �һ���������һҳ */
						ptInputEvent->iVal = INPUT_VALUE_UP;
						
					}else if (iDelta < 0 - giXres/5){
						/* �󻬣�������һҳ */
						ptInputEvent->iVal = INPUT_VALUE_DOWN;
						
					}else{
						ptInputEvent->iVal = INPUT_VALUE_UNKNOWN;
						
					}
					
					return 0;
				}
			}				
		}else{
			/* û���� */
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

