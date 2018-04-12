#ifndef _TIMER_
#define _TIMER_

#define MAX_TIME_STR_LEN 128

#define TIMER_LOOP   1
#define TIMER_ONCE   0

#define TIMER_ID_ERR -1

INT32  timerInit(INT32 iMaxTimerCnt);
INT32  setTimer( INT32 iCnt100ms,    UINT16 event, void *pCbkData, INT32 len );   //普通定时器
INT32  setLpTimer( INT32 iCnt100ms,    UINT16 event, void *pCbkData, INT32 len ); //循环定时器
void   killTimer(INT32 iTimerId );
#endif
