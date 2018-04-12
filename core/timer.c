#include "all.h"

extern TaskDesc_T g_taskManager[MAX_TASK_NUM];
extern void getSelfTid(TID_T *pTID);

typedef struct
{
	BOOL   usedFlag;
	BOOL   isLpTimer;  //是否是循环定时器
	
	UINT32 iTaskId;    //设置定时器的task
	UINT64 iSetTick;   //设置定时器时的时间（100ms）
	UINT32 iTimer100ms;	//定时器时长

	UINT32 iCbkEvt;
	INT8  *pCbkData;
	UINT32 iCbkLen;
}TimerItem_T;

typedef struct
{
	TimerItem_T *pTimerItemArr;
	UINT32 		iArrCnt;
	BOOL        isFull;
	INT32 		iEmptyIdx;
	
	MUTEX  		mutex;
}TimerManager_T;

static TimerManager_T gTimerManager;

static void timerSend1sEvent2Tasks()
{
	INT32 idx = 0;
	TaskDesc_T *pDesc = NULL;
	TID_T recvTid = {0};
	
	for (idx = 0; idx < MAX_TASK_NUM; idx ++ )
	{
		pDesc = &g_taskManager[idx];

		if ( pDesc->taskItem.entry == NULL )
		{
			continue;
		}
		else
		{
			recvTid.iTno = pDesc->taskItem.iTno;
			ASend(TIMER_1s_EVENT, NULL, 0, &recvTid);
		}
	}
}

THREAD_ENTRY static void *TimerThread( void *arg )
{
	INT32 idx = 0;
	TimerItem_T *pItem = NULL;
	struct timeval tv;
	static UINT64 cur100ms = 0;
	TID_T revTid = {0};

	static UINT64 cur50ms = 0;
	
	while(1)
	{
		thread_sleep_ms(50);
		cur50ms ++;

		if ( cur50ms == 20 )
		{
			timerSend1sEvent2Tasks();
			cur50ms = 0;
		}
		
		gettimeofday(&tv,NULL);
		cur100ms = tv.tv_sec*10 + tv.tv_usec/100000;
		//sysLog_D("%ld-%ld-%ld", tv.tv_sec, tv.tv_usec, cur100ms);
		
		pthread_mutex_lock(&gTimerManager.mutex );
		for (idx = 0; idx < gTimerManager.iArrCnt; idx ++)
		{
			pItem = &gTimerManager.pTimerItemArr[idx];
			if ( pItem->usedFlag == FALSE )
				continue;
			
			if ( pItem->isLpTimer == TRUE )
			{
				if ( cur100ms - pItem->iSetTick >= pItem->iTimer100ms )
				{
					pItem->iSetTick = cur100ms;
					revTid.iTno = pItem->iTaskId;
					ASend(pItem->iCbkEvt, pItem->pCbkData, pItem->iCbkLen, &revTid);
				}
			}
			else
			{
				if ( (UINT32)(cur100ms - pItem->iSetTick) >= pItem->iTimer100ms )
				{
					//sysLog_D("cur100ms=%ld, iSetTick=%ld, iTimer100ms=%ld", cur100ms, pItem->iSetTick, pItem->iTimer100ms);
					pItem->iSetTick = cur100ms;
					revTid.iTno = pItem->iTaskId;
					ASend(pItem->iCbkEvt, pItem->pCbkData, pItem->iCbkLen, &revTid);
					if(pItem->pCbkData != NULL )
					{
						free(pItem->pCbkData);
					}
					memset(pItem, 0x0, sizeof(TimerItem_T) );
				}
			}
		}
		pthread_mutex_unlock(&gTimerManager.mutex );
	}
}

static INT32 findFreeTimerPos()
{ 	//simply loop to find empty pos
	INT32 idx = 0;
	TimerItem_T *pItem = NULL;
	
	pthread_mutex_lock(&gTimerManager.mutex );
	
	if ( gTimerManager.isFull == TRUE )
	{
		return -1;
	}

	for ( idx = 0; idx < gTimerManager.iArrCnt; idx ++ )
	{
		pItem = &gTimerManager.pTimerItemArr[idx];
		if ( pItem->usedFlag == TRUE )
		{
			continue;
		}
		else
		{
			pthread_mutex_unlock(&gTimerManager.mutex );
			gTimerManager.isFull = FALSE;
			return idx;
		}
	}
	
	sysLog_E("findFreeTimerPos: timer pos is full...");
	
	gTimerManager.isFull = TRUE;
	pthread_mutex_unlock(&gTimerManager.mutex );
	
	return -1;
}

INT32 timerInit(INT32 iMaxTimerCnt)
{
	pthread_t   tid;
	pthread_attr_t attr = {0};
	
	if ( iMaxTimerCnt > 2000 || iMaxTimerCnt < 20 )
	{
		return RESULT_PARA_ERR;
	}

	gTimerManager.iArrCnt = iMaxTimerCnt;
	gTimerManager.pTimerItemArr = malloc( iMaxTimerCnt * sizeof(TimerItem_T) );
	if ( gTimerManager.pTimerItemArr == NULL )
	{
		return RESULT_PARA_ERR;
	}
	memset(gTimerManager.pTimerItemArr, 0x0, iMaxTimerCnt * sizeof(TimerItem_T));
	
	gTimerManager.iEmptyIdx = 0;

	pthread_mutex_init( &gTimerManager.mutex, NULL );
	pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

	if ( pthread_create(&tid, &attr, TimerThread, NULL) == 0 )
	{
		sysLog_E("timerInit succ, cnt[%d]...", iMaxTimerCnt );
		pthread_attr_destroy(&attr);
		return RESULT_OK;
	}

	sysLog_E("timerInit error...");
	
	return RESULT_OPER_SYS_ERR;
}

INT32 timerSet( INT32 iCnt100ms, UINT16 event, void *pCbkData, INT32 len, INT8 type )
{
	TIMERID iTimerId = 0;
	TimerItem_T *pItem = NULL;
	TID_T tid = {0};
	struct timeval tv;
	gettimeofday(&tv,NULL);

	if (iCnt100ms < 1 )
	{
		return -1;
	}
	
	iTimerId = findFreeTimerPos();
	if ( iTimerId == -1 )
	{
		return -1;
	}

	getSelfTid(&tid);

	pthread_mutex_lock(&gTimerManager.mutex );
	
	pItem = &gTimerManager.pTimerItemArr[iTimerId];

	if ( pCbkData != NULL )
	{
		pItem->pCbkData = malloc(len );
		if ( pItem == NULL )
		{
			return -1;
		}
		memcpy(pItem->pCbkData, pCbkData, len);
		pItem->iCbkLen = len;
	}
	
	pItem->iCbkEvt = event;
	
	if (type == 1)
	{
		pItem->isLpTimer = FALSE;
	}
	else
	{
		pItem->isLpTimer = TRUE;
	}
	
	pItem->iTimer100ms = iCnt100ms;
	pItem->usedFlag = TRUE;
	pItem->iTaskId = tid.iTno;
	pItem->iSetTick = tv.tv_sec*10 + tv.tv_usec/100000;
	
	pthread_mutex_unlock(&gTimerManager.mutex );
	
	return iTimerId;
}

TIMERID setTimer( INT32 iCnt100ms, UINT16 event, void *pCbkData, INT32 len )
{
	return timerSet( iCnt100ms, event, pCbkData, len, 1);
}

TIMERID setLpTimer( INT32 iCnt100ms, UINT16 event, void *pCbkData, INT32 len )
{
	return timerSet( iCnt100ms, event, pCbkData, len, 0);
}

void killTimer(TIMERID iTimerId )
{
	TimerItem_T *pItem = NULL;
	
	if ( iTimerId < 0 || iTimerId >= gTimerManager.iArrCnt )
	{
		return;
	}
	
	pthread_mutex_lock(&gTimerManager.mutex );
	pItem = &gTimerManager.pTimerItemArr[iTimerId];

	if ( pItem->usedFlag != TRUE)
	{
		pthread_mutex_unlock(&gTimerManager.mutex );
		return;
	}

	if ( pItem->pCbkData != NULL )
	{
		free(pItem->pCbkData);
		pItem->pCbkData = NULL;
	}

	memset(pItem, 0x0, sizeof(TimerItem_T) );
	
	pthread_mutex_unlock(&gTimerManager.mutex );
	
	return;
}
