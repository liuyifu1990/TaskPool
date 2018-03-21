#ifndef _QUEUE_H_
#define _QUEUE_H_
#include "core.h"

#pragma pack(1)

#define QUEUE_STATUS_NORMAL    0
#define QUEUE_STATUS_NEAR_FULL 1
#define QUEUE_FULL_PERCENT     90


typedef struct
{
	INT32 head;
	INT32 tail;
	INT32 count;
	INT32 total;
	INT8  status;	
	void **data;
	MUTEX mutex;
}CirQueue_T;

INT32       queueCreat(CirQueue_T *pQueue, INT32 iQueSize);
INT32       queueInsert(CirQueue_T *pQueue, void *pItem);
void*		queueGet(CirQueue_T *pQueue);
void  		queueDestory(CirQueue_T *pQueue);
INT32 		queueCnt(CirQueue_T *pQueue );

#endif