#ifndef _TIMER_
#define _TIMER_

#define MAX_TIME_STR_LEN 128

void   getCurTimeStr(INT8 *szTime, INT32 len );
INT32  setTimer( INT32 iCnt100ms,    UINT16 event, void *clbkData );
void   killTimer(INT32 timerId );
#endif
