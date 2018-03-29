#include "all.h"

static LogManager_T g_LogManager[MAX_LOG_TAG_NUM] = {0};
static pthread_mutex_t log_mutex;
static pthread_cond_t  log_cond;
CirQueue_T LogQueue = {0};
CirQueue_T *pLogQueue = NULL;

INT32 g_logLevel = LOG_LVL_DEBUG;

static void LogAppendFile(LogELem_T *pElem)
{
	LogManager_T *pManager = NULL;

	while (1)
	{
		if ( pElem == NULL
			|| g_LogManager[pElem->iLogTag].useFlag != TRUE
			|| pElem->iLen <= 0
			|| pElem->pLogStr == NULL )
		{
			break;
		}
		
		pManager = &g_LogManager[pElem->iLogTag];
		
		if (pManager->pFile == NULL)
		{
			break;
		}
		
		if ( pManager->iFileSize > MAX_FILE_SIZE * 1024 * 1024 )
		{
			fclose(pManager->pFile);
			remove(pManager->szCurPath);
			
			pManager->iSeq = pManager->iSeq % pManager->iSplitNum;
			snprintf(pManager->szCurPath, MAX_FILE_PATH_LEN, "%s/log/%s%02d%s"
					, getenv("HOME"), pManager->szFileName
					, pManager->iSeq, pManager->szFileSuffix );
			
			pManager->pFile =fopen(pManager->szCurPath, "wb+");
			if ( pManager->pFile == NULL )
			{
				break;
			}
			fputs(pElem->pLogStr, pManager->pFile);
			fputc('\n', pManager->pFile);
			fflush(pManager->pFile);
			pManager->iFileSize = pElem->iLen + 1;	
		}
		else
		{
			fputs(pElem->pLogStr, pManager->pFile);
			fputc('\n', pManager->pFile);
			fflush(pManager->pFile);
			pManager->iFileSize = pManager->iFileSize + pElem->iLen + 1;
			break;
		}

	}

	if ( pElem->pLogStr != NULL )
	{
		free(pElem->pLogStr);
		pElem->pLogStr = NULL;
		free(pElem);
	}
}

THREAD_ENTRY static void *LogThread( void *arg )
{
	LogELem_T *pElem = NULL;
	
	
	while(TRUE)
	{
		pthread_mutex_lock( &log_mutex );
		while( pLogQueue->count <= 0 )
		{
			pthread_cond_wait( &log_cond, &log_mutex );
		}

		pElem = queueGet(pLogQueue, QUEUE_DEL_YES );//取数据，写文件

		LogAppendFile( pElem);

		pthread_mutex_unlock( &log_mutex );
		
	}
}

INT32 logInit( )
{
	INT32 idx = 0;
	pthread_t threadId = 0;
	pthread_attr_t attr = {0};

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED );

	if ( pthread_mutex_init(&log_mutex, NULL) != 0 )
	{
		return RESULT_MUTEX_ERR;
	}

	if ( pthread_cond_init(&log_cond, NULL) != 0 )
	{
		return RESULT_MUTEX_ERR;
	}

	if ( queueCreat(&LogQueue, MAX_LOG_QUEUE_LEN ) != RESULT_OK )
	{
		return RESULT_UNKNOWN_ERR;
	}
	
	pLogQueue = &LogQueue;

	if (pthread_create(&threadId, &attr, LogThread, NULL) != 0 )
	{
		printf("create log thread err.\n");
		exit(1);
	}
	else
	{
		pthread_attr_destroy(&attr);
		printf("log init ok!\n");
	}
}

INT32 logRegister(INT8 *szFileName, INT32 iSplitNum, INT8 iLogTag )
{
	INT8 *pHome = NULL;
	INT8 *pTag  = NULL;
	
	if ( szFileName[0] == '\0' || iLogTag > MAX_LOG_TAG_NUM || iSplitNum <= 0 )
	{
		return RESULT_PARA_ERR;
	}

	if (g_LogManager[iLogTag].szFileName[0] != '\0' )
	{
		return RESULT_PARA_ERR;
	}

	pHome = getenv("HOME");
	
	if ( pHome == NULL )
	{
		return RESULT_PARA_ERR;
	}

	pTag = strstr(szFileName, ".");
	if ( pTag != NULL )
	{
		strncpy(g_LogManager[iLogTag].szFileSuffix, pTag,       szFileName + strlen(szFileName) - pTag ); 
		strncpy(g_LogManager[iLogTag].szFileName,   szFileName, pTag - szFileName );
		snprintf(g_LogManager[iLogTag].szCurPath, MAX_FILE_PATH_LEN, "%s/log/%s%02d%s"
				, pHome, g_LogManager[iLogTag].szFileName, 0, g_LogManager[iLogTag].szFileSuffix  );
	}
	else
	{
		snprintf(g_LogManager[iLogTag].szFileName, MAX_FILE_NAME_LEN, "%s", szFileName );
		snprintf(g_LogManager[iLogTag].szCurPath, MAX_FILE_PATH_LEN, "%s/log/%s%02d"
				, pHome, g_LogManager[iLogTag].szFileName, 0 );

	}
	
	g_LogManager[iLogTag].iSeq = 0;
	g_LogManager[iLogTag].iSplitNum = iSplitNum;
	g_LogManager[iLogTag].pFile = fopen( g_LogManager[iLogTag].szCurPath, "a" );
	if ( g_LogManager[iLogTag].pFile == NULL )
	{
		return RESULT_FILE_ERR;
	}

	g_LogManager[iLogTag].useFlag = TRUE;
	return RESULT_OK;
}

void logWrite(INT8 iLogTag, INT8 *pLogBuf, INT32 iBufLen)
{
	LogELem_T *pElem = NULL;

	if ( pLogBuf == NULL || iBufLen <= 0 || iLogTag >= MAX_LOG_TAG_NUM
		|| g_LogManager[iLogTag].szCurPath[0] == '\0' )
	{
		printf("para in error, log fullpath[%s]", g_LogManager[iLogTag].szCurPath );
		return;
	}
	
	pElem = malloc( sizeof(LogELem_T) );
	if ( pElem == NULL )
	{
		return;
	}

	pElem->pLogStr = malloc(iBufLen + 1);
	if ( pElem->pLogStr == NULL )
	{
		if ( pElem != NULL )
		{
			free(pElem);
		}
		return;
	}

	pElem->iLen = iBufLen;
	pElem->iLogTag = iLogTag;
	
    snprintf(pElem->pLogStr, iBufLen + 1, "%s", pLogBuf );

	pthread_mutex_lock(&log_mutex);
	queueInsert(pLogQueue, (void*) pElem );
	pthread_mutex_unlock(&log_mutex);

	pthread_cond_signal(&log_cond);
}

void WriteLogAPP00(INT32 level, INT8 *filename, INT32 line, const INT8 *szFMT, ... )
{
	INT8 szLogbuf[MAX_LOG_STR_LINE_LEN + 1] = {0};
	INT8 szInbuf[MAX_LOG_STR_LINE_LEN + 1] = {0};
	INT8 szTimeStr[MAX_TIME_STR_LEN + 1] = {0};

	if ( g_logLevel < level )
	{
		return;
	}

	getCurTimeStr( szTimeStr, MAX_TIME_STR_LEN );
	
	va_list args; 
	va_start(args, szFMT);
	vsnprintf(szLogbuf, MAX_LOG_STR_LINE_LEN, szFMT, args); 
	va_end(args); 

	if ( szLogbuf[0] == '\0' )
		return;

	snprintf(szInbuf, MAX_LOG_STR_LINE_LEN, "%s %s:%d %s", szTimeStr, filename, line, szLogbuf );
	
	logWrite(LOG_TAG_APP00, szInbuf, strlen(szInbuf) );
}

