#include "all.h"

static TaskDesc_T g_taskManager[MAX_TASK_NUM];
static INT32 g_iTaskNum = 0;
INT32  g_iTaskQueSize = 0;
TaskItem_T TaskItems[MAX_TASK_NUM];

static INT32 taskCreateByCfgsArr();
static INT32 taskCreatByCfgItem(TaskDesc_T *pDesc);
THREAD_ENTRY static void *TaskThread( void *arg );


INT32 taskInit()
{
	INT32 idx = 0;
	TaskItem_T *pItem = NULL;
	TaskDesc_T *pDesc = NULL;
	INT32 rc = 0;
	
	for (idx = 0; idx < MAX_TASK_NUM; idx ++ )
	{
		pItem = &TaskItems[idx];
		pDesc = &g_taskManager[idx];
		
		if (   pItem->iTno >= 0 
			&& pItem->entry != NULL 
			&& pItem->iStacksize <=0 
			&& pItem->iMaxtime <= 0 
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

THREAD_ENTRY static void *TaskThread( void *arg )
{
	
}

