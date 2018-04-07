#include "all.h"

typedef struct
{
	BOOL   usedFlag;
	
	UINT32 iTaskNo;
	time_t iSetTick;
	UINT32 iSet100msVal;	

	UINT32 iCbkEvt;
	INT8  *pCbkData;
	UINT32 iCbkLen;
}TimerItem_T;

typedef struct
{
	TimerItem_T *pTimerItemArr;
	UINT32 		iArrCnt;
	INT32 		iEmptyIdx;
	
	MUTEX  		mutex;
}TimerManager_T;

THREAD_ENTRY static void *TimerThread( void *arg )
{
	
}

INT32 timerInit(INT32 iMaxTimerCnt)
{
	if ( iMaxTimerCnt > 2000 || iMaxTimerCnt < 20 )
	{
		return RESULT_PARA_ERR;
	}

	
}

INT32 setTimer( INT32 iCnt100ms, UINT16 event, void *pCbkData )
{
	
}

INT32 setLpTimer( INT32 iCnt100ms, UINT16 event, void *pCbkData )
{
	
}


void killTimer(INT32 iTimerId )
{
	
}
