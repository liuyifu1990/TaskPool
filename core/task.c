#include "all.h"

static TaskDesc_T g_taskManager[MAX_TASK_NUM];
static INT32 g_iTaskNum = 0;
INT32  g_iTaskQueSize = 1000;
INT32  g_CurNodeNo;
INT32  g_iEventTimeoutSec = 2;

extern INT8  g_szCfgFilePath[MAX_FILE_PATH_LEN];

static pthread_key_t  p_key;
static pthread_once_t once = PTHREAD_ONCE_INIT; 
static pthread_t g_timeoutId = {0};

static INT32 taskCreateByCfgsArr();
static INT32 taskCreatByCfgItem(TaskDesc_T *pDesc);
THREAD_ENTRY static void *TaskThread( void *arg );
static TaskDesc_T*getTaskDesc(INT32 iTno);
THREAD_ENTRY static void *TaskInfoDetector( void *arg );
static void taskRuntimeDetect();

static void taskReadCoreCfg()
{
	//TBD
	g_iEventTimeoutSec = GetIniKeyInt("TASK", "MAX_EVENT_TIMEOUT", g_szCfgFilePath);
	if ( g_iEventTimeoutSec <=0 || g_iEventTimeoutSec > 10 )
	{
		g_iEventTimeoutSec = 2;
	}

	sysLog_D("taskReadCoreCfg: g_iEventTimeoutSec[%d]", g_iEventTimeoutSec);

	return;
}


INT32 taskInit(const TaskItem_T *szTaskItems)
{
	INT32 idx = 0;
	const TaskItem_T *pItem = NULL;
	TaskDesc_T *pDesc = NULL;
	INT32 rc = 0;
	pthread_t id = 0;
	
	taskReadCoreCfg();
	
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
	pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

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
	
	pTID->iTno =  (INT32)(long)pthread_getspecific(p_key);
	pTID->iNodeNo = g_CurNodeNo;

	//sysLog_D("getSelfTid: key=%d, iTno=%d", p_key, pTID->iTno );
}

static void sendInitEvent(INT32 iTno)
{
	TaskDesc_T *pDesc = NULL;

	pDesc = &g_taskManager[iTno];

	if ( pDesc->taskItem.entry != NULL )
	{
		sysLog_E("taskInitEvent: send init event to task[%d]", iTno );
		pDesc->taskItem.entry(INIT_TASK_EVENT, NULL, 0);
	}
}

static void taskInitOnce()
{
	pthread_key_create(&p_key, NULL); 
}

static void taskQuitEvent( )
{
	TaskDesc_T *pDesc = NULL;
	INT32 iTno = (INT32)(long)pthread_getspecific(p_key);
	
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

	iCurTno = *(INT32*)arg;
	pthread_once(&once, taskInitOnce);
	
	pthread_setspecific(p_key, (void*)(long)iCurTno );
	
	sysLog_E("TaskThread: init task %d...", iCurTno );

	sendInitEvent(iCurTno);
	
	pDesc = &g_taskManager[iCurTno];
	
	while(TRUE)
	{
		pthread_mutex_lock(&pDesc->task_mutex);
		
		while( pDesc->task_queue.count <= 0 )
		{
			pthread_cond_wait(&pDesc->task_cond, &pDesc->task_mutex);
		}

		pMsgItem = queueGet(&pDesc->task_queue, QUEUE_DEL_NO );
		if ( pMsgItem == NULL )
		{
			sysLog_E("TaskThread: msg in queue in NULL, tno[%d]", iCurTno);
			exit(1);
		}

		pthread_mutex_unlock(&pDesc->task_mutex);

		pDesc->status = TASK_STATUS_RUNNING;
		time(&pDesc->iTick);
		
		if (pDesc->taskItem.entry != NULL)
		{
			pDesc->taskItem.entry( pMsgItem->iEvent, pMsgItem->pMsg, pMsgItem->iMsgLen );
		}

		pDesc->iTick = 0;
		pDesc->status = TASK_STATUS_WAITING;
		pDesc->iTick = 0;
		
		pMsgItem = queueGet(&pDesc->task_queue, QUEUE_DEL_YES );
		if ( pMsgItem != NULL  )
		{
			free( (void*)pMsgItem );
			pMsgItem = NULL;
		}
		
	}
}

INT16 CurEvent()
{
	TaskDesc_T *pDesc = NULL;
	MessageItem_T *pMsgItem = NULL;
	INT32 iTno =  (INT32)(long)pthread_getspecific(p_key);

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
	INT32 iTno = (INT32)(long)pthread_getspecific(p_key);
	if ( pTID == NULL )
	{
		return;
	}

	//sysLog_D("CurSender: key=%d, iTno=%d", p_key, iTno );
	
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

INT32 ASend(UINT32 iEvent, INT8 *pMsg, INT32 iLen, TID_T *pReceiver)
{	//send to local node only
	TID_T TID = {0};
	MessageItem_T * pMsgItem = NULL;
	TaskDesc_T *pDesc = NULL;
	INT32 ret = 0;
	
	if ( iEvent < 0 || pMsg == NULL || iLen <= 0 || pReceiver == NULL )
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

	getSelfTid(&TID);
	memcpy(&pMsgItem->Sender, &TID, sizeof(TID_T) );
	
	pDesc = getTaskDesc(pReceiver->iTno);
	if (pDesc == NULL || pDesc->taskItem.entry == NULL )
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

static void taskRuntimeDetect()
{
	//to be done: queue print & event process time dectect
	time_t iCurTime = 0;
	TaskDesc_T *pDesc = NULL;
	INT32 idx = 0;
	MessageItem_T *pItem = NULL;
	INT8  szLogBuf[MAX_STR128 + 1] = {0};
	INT32 iBufLen = 0;
	time(&iCurTime);
	
	for (idx = 0; idx < MAX_TASK_NUM; idx ++)
	{
		pDesc = getTaskDesc(idx);

		if ( pDesc == NULL || pDesc->taskItem.entry == NULL )
			continue;

		if ( pDesc->status == TASK_STATUS_RUNNING  )
		{
			if ( iCurTime - pDesc->iTick > g_iEventTimeoutSec )
			{
				pItem = queueGet(&pDesc->task_queue, QUEUE_DEL_NO );
				if ( pItem == NULL )
					continue;
				
				sysLog_E("taskRuntimeDetect: task proc event[%d] to long[%d], program exit!!!", pItem->iEvent, iCurTime - pDesc->iTick );
				sleep(2);
				exit(1);
			}
		}


		iBufLen = strlen(szLogBuf);
		snprintf(szLogBuf + iBufLen, strlen(szLogBuf) - iBufLen, " %s-%d ", pDesc->taskItem.pNameStr, queueCnt(&pDesc->task_queue) );

	}

	if ( strlen(szLogBuf) > 0 )
		sysLog_E("taskRuntimeDetect: taskQueue state:%s", szLogBuf );
	
}

static void almSingleHandle()
{
	alarm(1);
	pthread_kill( g_timeoutId, SIGALRM );
	return;
}

THREAD_ENTRY static void *TaskInfoDetector( void *arg )
{
	INT32 idx = 0;
	TaskDesc_T *pDesc = NULL;
	time_t iCurTime;
	INT32 ret, sigNo = 0;

	sigset_t set = {0};
	sigemptyset(&set);
	sigaddset(&set, SIGALRM);
	
	for (;;)
	{
		ret = sigwait(&set, &sigNo );
		if ( ret != 0 )
			sysLog_D("TaskTimeDetector: sigwait err, ret=%d", ret);

		sysLog_D("TaskTimeDetector: sigwait %d", sigNo);
		taskRuntimeDetect();
	}
}


INT32 initTaskSingleManage()
{
	sigset_t set;
	pthread_attr_t attr = {0};
	
	sigemptyset(&set);
	sigaddset(&set, SIGALRM);

	if ( pthread_sigmask(SIG_SETMASK, &set, NULL) != 0 )
	{
		return RESULT_OPER_SYS_ERR;
	}

	signal(SIGALRM, almSingleHandle);
	alarm(1);
	
	pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
	if ( pthread_create( &g_timeoutId, &attr, TaskInfoDetector, NULL ) != 0 )
	{
		sysLog_D("initTaskSingleManage pthead_create returns error" );
		return RESULT_OPER_SYS_ERR;
	}
	else
	{
		sysLog_D("initTaskSingleManage create suc, pthreadId=%d", g_timeoutId );
	}
	

	return RESULT_OK;

}
