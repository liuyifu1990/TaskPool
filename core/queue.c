#include "all.h"

INT32 queueCreat(CirQueue_T *pQueue, INT32 iQueSize)
{
	if (pQueue == NULL || iQueSize <= 0 )
	{
		return RESULT_PARA_ERR;
	}

	if ( pthread_mutex_init(&pQueue->mutex, NULL) != 0 )
	{
		return RESULT_MUTEX_ERR;
	}	

	pQueue->data = (void**)GET_MEM(iQueSize * sizeof(void *) );
	if (pQueue->data == NULL)
	{
		return RESULT_ALLOC_ERR;
	}
	
	memset(pQueue->data, 0x0, iQueSize * sizeof(void *) );
	
	pQueue->count = 0;
	pQueue->head = pQueue->tail = 0;
	pQueue->status = QUEUE_STATUS_NORMAL;
	pQueue->total = iQueSize;
	
	return RESULT_OK;
}

INT32 queueInsert(CirQueue_T *pQueue, void *pItem)
{
	if ( pQueue == NULL || pItem == NULL )
	{
		return RESULT_PARA_ERR;
	}

	if ( pQueue->status != QUEUE_STATUS_NORMAL )
	{	
		return RESULT_QUE_FULL;
	}

	if ( pthread_mutex_lock(&pQueue->mutex) != 0 )
	{
		return RESULT_MUTEX_ERR;
	}
	
	pQueue->data[pQueue->tail] = pItem;
	pQueue->tail = (pQueue->tail + 1) % pQueue->total;

	pQueue->count ++;
	
	if ( (float)pQueue->count/(float)pQueue->total * 100 >= (float)QUEUE_FULL_PERCENT )
	{
		pQueue->status = QUEUE_STATUS_NEAR_FULL;
	}

	pthread_mutex_unlock(&pQueue->mutex);

	
	return RESULT_OK;
}

void *queueGet(CirQueue_T *pQueue, INT32 oper)
{
	void *pRet = NULL;
	
	if ( pQueue == NULL 
		|| (pQueue->head == pQueue->tail) )
		return pRet;
	
	if (pthread_mutex_lock(&pQueue->mutex) != 0 )
	{
		return NULL;
	}
	
	pRet = pQueue->data[pQueue->head];
	
	if ( oper == QUEUE_DEL_YES )
	{
		pQueue->data[pQueue->head] = NULL;
		
		pQueue->head = (pQueue->head + 1) % pQueue->total;
		pQueue->count --;
	}
	
	pthread_mutex_unlock(&pQueue->mutex);
	
	return pRet;
}


void queueDestory(CirQueue_T *pQueue)
{
	INT32 idx = 0;
	
	if ( pQueue == NULL )
		return;
	pthread_mutex_lock(&pQueue->mutex);
	
	for( idx =0; idx < pQueue->total; idx ++)
	{
		if ( pQueue->data[idx] != NULL )
		{
			RET_MEM(pQueue->data[idx]);
			pQueue->data[idx] = NULL;
		}
	}
	
	RET_MEM(pQueue->data);
	pQueue->count = 0;
	pthread_mutex_unlock(&pQueue->mutex);
	pthread_mutex_destroy(&pQueue->mutex);
	return;
}

INT32 queueCnt(CirQueue_T *pQueue )
{
	INT32 cnt = 0;
	
	if ( pQueue == NULL )
	{
		return -1;
	}


	pthread_mutex_lock(&pQueue->mutex);
	cnt = pQueue->count;
	pthread_mutex_unlock(&pQueue->mutex);
	
	return cnt;
}

