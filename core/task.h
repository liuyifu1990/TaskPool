#ifndef _TASK_H
#define _TASK_H

#pragma pack(1)

typedef struct
{
	INT32 iNodeNo;
	INT32 iTno;
}TID_T;

typedef struct
{
	INT16 iEvent;
	INT8  *pMsg;
	INT32 iMsgLen;
	TID_T Sender;
}MessageItem_T;

typedef void (*TASK_ENTRY_FUNC)(MessageItem_T *pMsg);

typedef struct
{
	INT16 iTno;
	TASK_ENTRY_FUNC entry;
	INT32 iStacksize;
	INT32 iMaxtime;
}TaskItem_T;

typedef struct
{
	TaskItem_T taskItem;
	
	pthread_mutex_t task_mutex;
	pthread_cond_t  task_cond;
	CirQueue_T 		task_queue;
}TaskDesc_T;

INT32 taskInit();
INT32 ASend( INT8 *pMsg, INT32 iLen, TID_T *pReceiver, TID_T *pSender );

#endif
