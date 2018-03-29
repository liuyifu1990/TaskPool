#include "all.h"

static TaskDesc_T g_taskManager[MAX_TASK_NUM];
static INT32 g_iTaskNum = 0;
INT32  g_iTaskQueSize = 1000;
INT32  g_CurNodeNo;

static pthread_key_t  p_key;
static pthread_once_t once = PTHREAD_ONCE_INIT; 


static INT32 taskCreateByCfgsArr();
static INT32 taskCreatByCfgItem(TaskDesc_T *pDesc);
THREAD_ENTRY static void *TaskThread( void *arg );
static TaskDesc_T*getTaskDesc(INT32 iTno);


INT32 taskInit(const TaskItem_T *szTaskItems)
{
	INT32 idx = 0;
	const TaskItem_T *pItem = NULL;
	TaskDesc_T *pDesc = NULL;
	INT32 rc = 0;

	if ( szTaskItems == NULL )
	{
		sysLog_E("taskInit: szTaskItems is NULL");
		return RESULT_PARA_ERR;

	}
	
	for (idx = 0; idx < MAX_TASK_NUM; idx ++ )
	{
		pItem = &szTaskItems[idx];
		
		if (   pItem->iTno >= 0 
			&& pItem->entry != NULL 
			&& pItem->iStacksize > 0 
			&& pItem->iMaxtime > 0 
			&& pItem->iTno < MAX_TASK_NUM )
		{
			if (g_taskManager[pItem->iTno].taskItem.entry != NULL )//已经存在一个了
			{
				sysLog_E("taskInit: overlap Tno[%d] in cfg arr", pItem->iTno);
				return RESULT_PARA_ERR;
			}
			
			memcpy( &g_taskManager[pItem->iTno].taskItem, pItem, sizeof(TaskItem_T) );

			g_iTaskNum ++;
		}
		else if (pItem->iTno == 0 && pItem->entry == NULL
			  && pItem->iStacksize == 0 && pItem->iMaxtime == 0 )
		{
			break;
		}
		else
		{
			sysLog_E("taskInit: cfg taskItem is error, iTno[%d] istacksize[%d]" \
					 " iMaxtime[%d] taskEntry[%p] MAX_TASK_NUM is[%d]"
					 , pItem->iTno, pItem->iStacksize
					 , pItem->iMaxtime, pItem->entry, MAX_TASK_NUM);
			return RESULT_PARA_ERR;
		}
	}

	if ( g_iTaskNum == 0 )
	{
		sysLog_E("taskInit: empty cfg taskitems");
		return RESULT_PARA_ERR;
	}

	return taskCreateByCfgsArr();
}

static INT32 taskCreateByCfgItem(TaskDesc_T *pDesc)
{
	INT32 rc = 0;
	pthread_attr_t attr = {0};
	pthread_t      id = {0};

	if ( pDesc == NULL )
	{
		return RESULT_PARA_ERR;
	}

 	rc =  queueCreat(&pDesc->task_queue, g_iTaskQueSize );
	if ( rc != RESULT_OK )
	{
		return rc;
	}

	rc = pthread_mutex_init(&pDesc->task_mutex, NULL);
	if ( rc != 0 )
	{
		return RESULT_OPER_SYS_ERR;
	}

	rc = pthread_cond_init(&pDesc->task_cond, NULL);
	if ( rc != 0 )
	{
		return RESULT_OPER_SYS_ERR;
	}

	//创建任务线程，参数为tno
	pthread_attr_init(&attr);
	pthread_attr_setstacksize( &attr, pDesc->taskItem.iStacksize * 1024);

	if ( pthread_create( &id, &attr, TaskThread, (void*)&pDesc->taskItem.iTno ) != 0 )
	{
		sysLog_E("taskCreateByCfgItem: create task[%d] error", pDesc->taskItem.iTno );
		return RESULT_OPER_SYS_ERR;
	}
	
	pthread_attr_destroy(&attr);
	return RESULT_OK;
}

static INT32 taskCreateByCfgsArr()
{
	INT32 idx = 0;
	TaskDesc_T *pDesc = NULL;
	INT32 rc = 0;
	
	if ( g_iTaskNum == 0 )
	{
		return RESULT_OK;
	}

	for (idx = 0; idx < MAX_TASK_NUM; idx ++)
	{
		pDesc = &g_taskManager[idx];
		if ( pDesc->taskItem.iTno != 0 &&  pDesc->taskItem.entry != NULL )
		{
			rc = taskCreateByCfgItem( pDesc);
			if ( rc != RESULT_OK )
			{
				sysLog_E("taskCreateByCfgsArr: create task[%d] error, rc[%d]", pDesc->taskItem.iTno, rc);
				return rc;
			}
		}
	}

	sysLog_E("taskCreateByCfgsArr: create %d task succ!", g_iTaskNum);
	
	return RESULT_OK;
}

void getSelfTid(TID_T *pTID)
{
	if ( pTID == NULL )
		return;
	
	pTID->iTno =  *(INT16*)pthread_getspecific(p_key);
	pTID->iNodeNo = g_CurNodeNo;
}

static void taskInitEvent()
{
	TaskDesc_T *pDesc = NULL;
	INT16 iTno = *(INT16*)pthread_getspecific(p_key);
	
	pDesc = &g_taskManager[iTno];
	
	pthread_mutex_lock(&pDesc->task_mutex);
	
	if ( pDesc->taskItem.entry != NULL )
	{
		sysLog_E("taskInitEvent: send init event to task[%d]", iTno );
		pDesc->taskItem.entry(INIT_TASK_EVENT, NULL, 0);
	}
	
	pthread_mutex_unlock(&pDesc->task_mutex);
}

static void taskQuitEvent( )
{
	TaskDesc_T *pDesc = NULL;
	INT16 iTno = *(INT16*)pthread_getspecific(p_key);
	
	pDesc = &g_taskManager[iTno];
	pthread_mutex_lock(&pDesc->task_mutex);
	
	if ( pDesc->taskItem.entry != NULL )
	{
		sysLog_E("taskQuitEvent: send quit event to task[%d]", iTno );
		pDesc->taskItem.entry(QUIT_TASK_EVENT, NULL, 0);
	}
	pthread_mutex_unlock(&pDesc->task_mutex);
}

THREAD_ENTRY static void *TaskThread( void *arg )
{
	INT32 iCurTno = 0;
	TaskDesc_T *pDesc = NULL;
	MessageItem_T *pMsgItem = NULL;
	
	

	iCurTno = *(INT16*)arg;
	
	pthread_key_create(&p_key,NULL); 
	pthread_setspecific(p_key, arg);
	
	sysLog_E("TaskThread: init task %d...", iCurTno );
	pDesc = &g_taskManager[iCurTno];

	pthread_once(&once, taskInitEvent);
	
	while(TRUE)
	{
		pthread_mutex_lock(&pDesc->task_mutex);
		while( pDesc->task_queue.count <= 0 )
		{
			pDesc->status = TASK_STATUS_WAITING;
			pthread_cond_wait(&pDesc->task_cond, &pDesc->task_mutex);
		}

		pMsgItem = queueGet(&pDesc->task_queue, QUEUE_DEL_NO );
		if ( pMsgItem == NULL )
		{
			sysLog_E("TaskThread: msg in queue in NULL, tno[%d]", iCurTno);
			exit(1);
		}
		pDesc->status = TASK_STATUS_RUNNING;
		/*
		* 设置执行时长定时器
		*/
		if (pDesc->taskItem.entry != NULL)
		{
			pDesc->taskItem.entry( pMsgItem->iEvent, pMsgItem->pMsg, pMsgItem->iMsgLen );
		}

		pMsgItem = queueGet(&pDesc->task_queue, QUEUE_DEL_YES );
		free( (void*)pMsgItem );
		pMsgItem = NULL;
		
		pthread_mutex_unlock(&pDesc->task_mutex);
	}
}

INT16 CurEvent()
{
	TaskDesc_T *pDesc = NULL;
	MessageItem_T *pMsgItem = NULL;
	INT16 iTno = *(INT16*)pthread_getspecific(p_key);

	pDesc = &g_taskManager[iTno];
	if ( pDesc->status == TASK_STATUS_RUNNING )
	{
		pMsgItem = queueGet(&pDesc->task_queue, QUEUE_DEL_NO);
		if ( pMsgItem == NULL )
		{
			return -1;
		}
		
		return pMsgItem->iEvent;
	}
	
	return -1;
}

void CurSender(TID_T *pTID)
{
	TaskDesc_T *pDesc = NULL;
	MessageItem_T *pMsgItem = NULL;
	INT16 iTno = *(INT16*)pthread_getspecific(p_key);
	if ( pTID == NULL )
	{
		return;
	}
	
	pDesc = &g_taskManager[iTno];
	if ( pDesc->status == TASK_STATUS_RUNNING )
	{
		pMsgItem = queueGet(&pDesc->task_queue, QUEUE_DEL_NO);
		if ( pMsgItem == NULL )
		{
			return;
		}
		
		memcpy( pTID, &pMsgItem->Sender, sizeof( TID_T ) );
	}

	return;
}

INT32 ASend(INT16 iEvent, INT8 *pMsg, INT32 iLen, TID_T *pReceiver)
{
	TID_T TID = {0};
	MessageItem_T * pMsgItem = NULL;
	TaskDesc_T *pDesc = NULL;
	INT32 ret = 0;
	
	if ( iEvent<0 || pMsg == NULL || iLen <= 0 || pReceiver == NULL )
	{
		return RESULT_PARA_ERR;
	}
	
	pMsgItem = malloc(sizeof(MessageItem_T) );
	if ( pMsgItem == NULL )
	{
		return RESULT_ALLOC_ERR;
	}
	memset( pMsgItem, 0x0, sizeof(MessageItem_T) );

	pMsgItem->pMsg = malloc(iLen);
	if ( pMsgItem->pMsg == NULL )
	{
		free(pMsgItem);
		return RESULT_ALLOC_ERR;
	}
	memcpy(pMsgItem->pMsg, pMsg, iLen );
	
	pMsgItem->iEvent = iEvent;
	pMsgItem->iMsgLen = iLen;
	memcpy(&pMsgItem->Sender, pReceiver, sizeof(TID_T) );
	
	getSelfTid(&TID);

	pDesc = getTaskDesc(pReceiver->iTno);
	if (pDesc == NULL )
	{
		return RESULT_PARA_ERR;
	}
	
	pthread_mutex_lock(&pDesc->task_mutex);

	ret = queueInsert(&pDesc->task_queue, (void*) pMsgItem );
	
	if ( ret != RESULT_OK )
	{
		pthread_mutex_unlock(&pDesc->task_mutex);
		return ret;
	}
	
	pthread_mutex_unlock(&pDesc->task_mutex);

	pthread_cond_signal(&pDesc->task_cond );
}

static TaskDesc_T*getTaskDesc(INT32 iTno)
{
	if ( iTno < 0 || iTno > MAX_TASK_NUM )
	{
		return NULL;
	}

	return &g_taskManager[iTno];
}