#ifndef _TASK_H
#define _TASK_H

#pragma pack(1)

#define TASK_STATUS_RUNNING 1
#define TASK_STATUS_WAITING 0

#define INIT_TASK_EVENT   1000
#define QUIT_TASK_EVENT   1001


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

typedef void (*TASK_ENTRY_FUNC)(INT16 iEvent, INT8 *pMsg, INT32 iMsgLen);

typedef struct
{
	INT16 iTno;				//task的Tno
	UINT8 *pNameStr;		//线程的名字
	TASK_ENTRY_FUNC entry;	//task处理函数
	INT32 iStacksize;		//线程栈空间大小，单位kb
	INT32 iMaxtime;			//最大单事件处理最大运行时间，单位s
}TaskItem_T;

typedef struct
{
	TaskItem_T taskItem;
	
	pthread_mutex_t task_mutex;
	pthread_cond_t  task_cond;
	CirQueue_T 		task_queue;

	UINT8 status;
}TaskDesc_T;

INT32 taskInit(const TaskItem_T *szTaskItems);
INT32 ASend(INT16    iEvent, INT8 *pMsg, INT32 iLen, TID_T *pReceiver);
void  getSelfTid();
INT16 CurEvent();
void  CurSender(TID_T *pTID );
#endif
