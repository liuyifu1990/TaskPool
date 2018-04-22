#include "all.h"

extern TaskItem_T TaskItems[MAX_TASK_NUM];

#pragma pack(1)

typedef struct
{
	UINT8 szProcPath[MAX_FILE_PATH_LEN + 1];
	UINT8 szProcName[MAX_FILE_NAME_LEN + 1];
	BOOL  isMyself;
}procPathInfo_T;

typedef struct
{
	procPathInfo_T pathName[MAX_PROC_COUNT];
	INT32 iProcCnt;
	INT32 msqId;
	
	INT32 shmId;//proc info
	
	pthread_mutex_t *pProcMutex;
}mutiProcManager_T;

typedef struct
{
	pid_t   pid;
	UINT8 	szProcPath[MAX_FILE_PATH_LEN + 1];
	INT32 	iTaskCnt;
	UINT32  szTnoArr[MAX_TASK_NUM];
}processShmBuf_T;

static INT8 g_ProcCfgFilePath[MAX_FILE_PATH_LEN + 1] = {0};

static INT8 g_SelfProcPath[MAX_FILE_PATH_LEN + 1];
static INT8 g_SelfProcName[MAX_FILE_NAME_LEN + 1];
static INT8 g_ProcCount = 0;

static mutiProcManager_T ProcManager = {0};
static INT32 gInitSleepTime = 5;
//static pthread_mutex_t *pProcMutex = NULL;


static void getProcNamebyPath(INT8 *szPath, INT8 *szName, INT32 len )
{
	INT8 *pEnd = NULL;
	INT8 *pIdx = NULL;

	if ( szPath == NULL || szName == NULL 
		|| len <= 0 || strstr(szPath, "/") == NULL )
		return;

	pEnd = &szPath[strlen(szPath)];

	pIdx = pEnd;
	
	while( *pIdx != '/' )
	{
		pIdx --;
	}

	snprintf(szName, len -1, "%s", pIdx + 1);
}

//read and verrify Process.ini
static INT32 ipcHandleProcCfg()
{
	INT32 iCnt = 0;
	INT8 *pHome = NULL;
	pHome = getenv("HOME");
	INT32 iFor = 0;
	INT8  szTitle[MAX_STR32 + 1] = {0};
	procPathInfo_T *pProInfo = NULL;
	static INT8 flag = 0;
	
	if ( pHome == NULL )
	{
		return RESULT_OPER_SYS_ERR;
	}

	snprintf( g_ProcCfgFilePath, MAX_FILE_PATH_LEN, "%s/etc/%s", pHome, PROC_CFG_FILE_NAME  );

	getCurProcFullpath(g_SelfProcPath, sizeof(g_SelfProcPath) );
	getProcNamebyPath(g_SelfProcPath, g_SelfProcName, sizeof(g_SelfProcName) );
	
	iCnt = GetIniKeyInt("CFG", "modulecount", g_ProcCfgFilePath );

	if ( iCnt <=0 || iCnt > MAX_PROC_COUNT )
	{
		sysLog_E("ipcHandleProcCfg: bad process count[%d], cfgfile[%s]", iCnt, g_ProcCfgFilePath );
		return RESULT_CFG_ITEM_ERR;
	}

	for ( iFor = 0; iFor < iCnt; iFor ++ )
	{
		pProInfo = &ProcManager.pathName[iFor];
		
		snprintf(szTitle, MAX_STR32, "MODULE_%02d", iFor);
		
		GetIniKeyString(szTitle, "modulename", pProInfo->szProcName, MAX_FILE_NAME_LEN, g_ProcCfgFilePath);
		if ( pProInfo->szProcName[0] == '\0' )
		{
			sysLog_E("ipcHandleProcCfg: module %d config err, cfgitem %s", iFor, szTitle);
			return RESULT_CFG_ITEM_ERR;
		}

		snprintf(pProInfo->szProcPath, MAX_FILE_PATH_LEN, "%s/bin/%s"
				, pHome, pProInfo->szProcName );

		if ( strcmp(pProInfo->szProcPath, g_SelfProcPath) == 0 
			&& strcmp(pProInfo->szProcName, g_SelfProcName) == 0 )
		{
			if ( flag == 1 )
			{
				sysLog_E("ipcHandleProcCfg: two same cfg[%s] in proc cfg file", pProInfo->szProcName);
				return RESULT_CFG_ITEM_ERR;	
			}
			
			pProInfo->isMyself = TRUE;
			flag = 1;
		}

		sysLog_D("ipcHandleProcCfg: proc cfg item[%d] is [%s]", iFor, pProInfo->szProcName );
		
	}

	if ( iFor != iCnt )
	{
		sysLog_D("ipcHandleProcCfg: proc cfg itemCnt[%d] is [%s]", iFor, pProInfo->szProcName );
		return RESULT_CFG_ITEM_ERR;
	}
	
	ProcManager.iProcCnt = iFor;
	
	return RESULT_OK;
}

static BOOL ipcIsOverlapProcName(procPathInfo_T *pPathInfoLow, procPathInfo_T *pPathInfoUp )
{
	if ( pPathInfoLow == NULL || pPathInfoUp == NULL )
	{
		return FALSE;
	}

	if ( pPathInfoLow->szProcPath[0] == '\0' || pPathInfoUp->szProcPath[0] == '\0' )
	{
		return FALSE;
	}

	if ( strcmp(pPathInfoLow->szProcPath, pPathInfoUp->szProcPath) == 0 )
	{
		sysLog_E("ipcIsOverlapTaskId: overlap proc name: %s-%s", pPathInfoLow->szProcPath, pPathInfoUp->szProcPath);
		return TRUE;
	}

	return FALSE;
}

INT32  ipcMoniVerifyProcCfgInfo()
{
	procPathInfo_T *pPathInfoLow = NULL;
	procPathInfo_T *pPathInfoUp  = NULL;
	INT32 iFor = 0;
	INT32 jFor = 0;

	if ( ProcManager.iProcCnt == 1 )
	{
		return RESULT_OK;	
	}

	for (iFor = 0; iFor < ProcManager.iProcCnt; iFor ++)
	{
		for ( jFor = iFor + 1; jFor < ProcManager.iProcCnt; jFor ++ )
		{
			pPathInfoLow = &ProcManager.pathName[iFor];
			pPathInfoUp = &ProcManager.pathName[jFor];	

			if ( TRUE == ipcIsOverlapProcName( pPathInfoLow,  pPathInfoUp) )
			{
				return RESULT_CFG_ITEM_ERR;
			}
		}
		

	}
	
	return RESULT_OK;
}

static void *ipcMoniGetShmProcInfoByPath(const UINT8 *szPath)
{
	key_t shm_key = 0;
	INT32 shm_id = 0;
	void *pRet = NULL;

	if (szPath == NULL || szPath[0] == '\0' )
	{
		return NULL;
	}
	
	shm_key = ftok( szPath, IPC_KEY_SHMBUF );
	shm_id = shmget(shm_key, sizeof(processShmBuf_T), 0 );
	if ( -1 == shm_id)
	{
		sysLog_E("ipcMoniGetShmProcInfoByPath: get shmbuf by path[%s] err, errno[%d]", szPath, errno );
		return NULL;
	}

	pRet = shmat(shm_id, NULL, SHM_RDONLY );
	if ( pRet == NULL )
	{
		sysLog_E("ipcMoniGetShmProcInfoByPath: attach to shmbuf by path[%s] err, errno[%d]", szPath, errno );
		return NULL;
	}

	return pRet;
}

static BOOL ipcIsOverlapTaskid(processShmBuf_T *shmBuf1, processShmBuf_T *shmBuf2)
{
	INT32 iFor = 0;
	INT32 jFor = 0;

	if ( shmBuf1 == NULL || shmBuf2 == NULL )
	{
		return FALSE;
	}

	for(iFor =0; iFor < shmBuf1->iTaskCnt; iFor ++ )
	{
		for(jFor = 0; jFor < shmBuf2->iTaskCnt; jFor ++ )
		{
			if ( shmBuf1->szTnoArr[iFor] == shmBuf2->szTnoArr[iFor] 
				|| ( shmBuf1->szTnoArr[iFor] == 0 || shmBuf2->szTnoArr[iFor] == 0 ) )
			{
				sysLog_E("ipcIsOverlapTaskid: overlap or err taskid [path1-%s, taskid-%d] [path2-%s, taskid-%d]"
						, shmBuf1->szProcPath, shmBuf1->szTnoArr[iFor]
						, shmBuf2->szProcPath, shmBuf2->szTnoArr[jFor] );
				return TRUE;
			}
		} 
	}
	
	return FALSE;
}


INT32  ipcMoniVerifyProcTaskInfo()
{
	UINT8 *pPathLow = NULL;
	UINT8 *pPathUp  = NULL;
	INT32 iFor = 0;
	INT32 jFor = 0;
	processShmBuf_T *pBufLow = NULL;
	processShmBuf_T *pBufUp = NULL;

	if ( ProcManager.iProcCnt == 1 )
	{
		return RESULT_OK;	
	}

	//sysLog_D("ipcMoniVerifyProcTaskInfo: procCnt[%d], procPath[%s]", ProcManager.iProcCnt, ProcManager.pathName[0].szProcPath );

	for (iFor = 0; iFor < ProcManager.iProcCnt; iFor ++)
	{
		for ( jFor = iFor + 1; jFor < ProcManager.iProcCnt; jFor ++ )
		{
			pPathLow =   ProcManager.pathName[iFor].szProcPath;
			pPathUp = ProcManager.pathName[jFor].szProcPath;

			pBufLow = ipcMoniGetShmProcInfoByPath(pPathLow );
			pBufUp  = ipcMoniGetShmProcInfoByPath(pPathUp );

			if ( pBufLow == NULL || pBufUp == NULL )
			{
				return RESULT_CFG_ITEM_ERR;
			}
			
			if ( TRUE == ipcIsOverlapTaskid( pBufLow,  pBufUp) )
			{
				return RESULT_CFG_ITEM_ERR;
			}
		}
		

	}
	
	return RESULT_OK;
}


static INT32 ipcCreateComShmBuf( INT32 len        )
{
	
	return RESULT_OK;
}

static INT32 ipcCreateMsgQue()
{
	key_t  mq_key = 0;
	INT32  mq_id  = 0;
	INT8 *pProcMqKey = g_SelfProcPath;

	mq_key = ftok( pProcMqKey, IPC_KEY_MSGQUE );
	mq_id = msgget(mq_key, IPC_CREAT|0666 );
	
	if ( mq_id == -1 )
	{
		sysLog_E("ipcCreateMsgQue: msgget return -1, errno[%d]", errno );
		return RESULT_OPER_SYS_ERR;
	}
	ProcManager.msqId = mq_id;

	sysLog_E("ipcCreateMsgQue: create mq succ, procName-%s", g_SelfProcName);
	return RESULT_OK;
}


static INT32 ipcinitTaskInfoShmBuf()
{
	INT8 *pProcInfoShmKey = g_SelfProcPath;
	key_t shm_key = 0;
	INT32 shm_id = 0;
	processShmBuf_T *pInfoBuf = NULL;
	INT32 	idx = 0;
	
	if ( pProcInfoShmKey[0] == '\0' )
	{
		return RESULT_PARA_ERR;
	}

	shm_key = ftok( pProcInfoShmKey, IPC_KEY_SHMBUF );
	shm_id = shmget(shm_key, sizeof(processShmBuf_T), IPC_CREAT|0777  );
	if ( shm_id == -1 )
	{
		sysLog_E("ipcinitTaskInfoShmBuf: shmget return -1, errno[%d]", errno );
		return RESULT_OPER_SYS_ERR;
	}
	ProcManager.shmId = shm_id;

	pInfoBuf = shmat( shm_id, NULL, 0);
	if ( pInfoBuf == NULL )
	{
		sysLog_E("ipcinitTaskInfoShmBuf: shmat return -1, errno[%d]", errno );
		return RESULT_OPER_SYS_ERR;
	}
	sysLog_E("ipcinitTaskInfoShmBuf: shmid[%d] shmaddr[%p] ", shm_id, pInfoBuf );

	memset(pInfoBuf, 0x0, sizeof(processShmBuf_T) );
	
	pInfoBuf->pid = getpid();
	strncpy(pInfoBuf->szProcPath, g_SelfProcPath, MAX_FILE_PATH_LEN );
	while(idx < MAX_TASK_NUM )
	{
		if ( TaskItems[idx].iTno != 0 )
		{
			pInfoBuf->szTnoArr[idx] = TaskItems[idx].iTno;
			pInfoBuf->iTaskCnt ++;
		}
		else
		{
			idx ++;
			continue;
		}
		idx ++;
	}
	
	if (pInfoBuf->iTaskCnt == 0)
	{
		sysLog_E("ipcinitTaskInfoShmBuf: proc[%s] task num is 0", g_SelfProcName );
		shmctl(shm_id, IPC_RMID, NULL);
		return RESULT_PARA_ERR;
	}
	
	sysLog_E("ipcinitTaskInfoShmBuf: write to shm buf suc, procName-%s, taskCnt-%d", g_SelfProcPath, pInfoBuf->iTaskCnt );
	pInfoBuf = NULL;

	return RESULT_OK;
}

static INT32 ipcInitProcMutex()
{
	INT8 *pProcMutexShmKey = ProcManager.pathName[0].szProcPath;
	key_t shm_key = 0;
	INT32 shm_id = 0;
	pthread_mutexattr_t  attr = {0};

	if ( pProcMutexShmKey[0] == '\0' )
	{
		return RESULT_PARA_ERR;
	}

	shm_key = ftok( pProcMutexShmKey, IPC_KEY_MUTEX );
	shm_id = shmget(shm_key, sizeof(pthread_mutex_t), 0 );

	if ( shm_id == -1 )
	{
		sysLog_E("ipcMoniCreateProcMutex: shmget return -1, errno[%d]", errno );
		return RESULT_OPER_SYS_ERR;
	}

	ProcManager.pProcMutex = shmat( shm_id, NULL, 0);
	if ( ProcManager.pProcMutex == NULL )
	{
		sysLog_E("ipcinitTaskInfoShmBuf: shmat return -1, errno[%d]", errno );
		return RESULT_OPER_SYS_ERR;
	}

	return RESULT_OK;
}

static INT32 ipcMoniInitProcMutex()
{
	INT8 *pProcMutexShmKey = ProcManager.pathName[0].szProcPath;
	key_t shm_key = 0;
	INT32 shm_id = 0;
	pthread_mutexattr_t  attr = {0};

	if ( pProcMutexShmKey[0] == '\0' )
	{
		return RESULT_PARA_ERR;
	}

	shm_key = ftok( pProcMutexShmKey, IPC_KEY_MUTEX );
	shm_id = shmget(shm_key, sizeof(pthread_mutex_t), IPC_CREAT|0777 );

	if ( shm_id == -1 )
	{
		sysLog_E("ipcMoniCreateProcMutex: shmget return -1, errno[%d]", errno );
		return RESULT_OPER_SYS_ERR;
	}

	ProcManager.pProcMutex = shmat( shm_id, NULL, 0);
	if ( ProcManager.pProcMutex == NULL )
	{
		sysLog_E("ipcinitTaskInfoShmBuf: shmat return -1, errno[%d]", errno );
		return RESULT_OPER_SYS_ERR;
	}

	memset(ProcManager.pProcMutex, 0x0, sizeof(pthread_mutex_t) );

	sysLog_D("ipcinitTaskInfoShmBuf: process mutex in shm at[%p]", ProcManager.pProcMutex);

	pthread_mutexattr_init(&attr); 
	pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED); 

	pthread_mutex_init(ProcManager.pProcMutex, &attr );

	pthread_mutexattr_destroy(&attr);
	
	return RESULT_OK;
}

static INT32 ipcMoniExecProcess()
{
	pid_t pid = 0;
	INT32 iFor = 0;
	INT32 iExeRet = 0;
	
	if ( (pid = vfork()) < 0 )
	{
		sysLog_E("ipcMoniExecProcess: vfork error, errno[%d]", errno );
		return RESULT_OPER_SYS_ERR;
	}
	else if ( pid == 0 )//child
	{
		for(iFor =0; iFor < ProcManager.iProcCnt; iFor ++)
		{
			if ( ProcManager.pathName[iFor].szProcPath[0] != '\0' 
				&& ProcManager.pathName[iFor].isMyself != TRUE )
			{
				iExeRet = execl(ProcManager.pathName[iFor].szProcPath
								, ProcManager.pathName[iFor].szProcName, (char*)0);

				if ( iExeRet <= 0 )
				{
					sysLog_E("ipcMoniExecProcess: execute %s fail, errno[%d]", ProcManager.pathName[iFor].szProcPath, errno );
					return RESULT_OPER_SYS_ERR;
				}
				else
				{
					sysLog_E("ipcMoniExecProcess: excute %s succ", ProcManager.pathName[iFor].szProcPath);
				}
			}
		}
	}

	return RESULT_OK;
}

//monitor process
INT32 initMoniIpc()
{
	INT32 ret = RESULT_OK;
	
	
	do
	{
		ret = ipcHandleProcCfg();
		if ( ret != RESULT_OK )
		{
			break;
		}

		ret = ipcMoniVerifyProcCfgInfo();
		if ( ret != RESULT_OK )
		{
			break;
		}

		ret = ipcMoniInitProcMutex();
		if ( ret != RESULT_OK )
		{
			break;
		}
		

		if ( 0 != pthread_mutex_lock(ProcManager.pProcMutex ) )
		{
			ret = RESULT_OPER_SYS_ERR;
			break;
		}

		sysLog_D("initMoniIpc: lock process mutex");
		
		if ( RESULT_OK != ipcMoniExecProcess() )
		{
			ret = RESULT_OPER_SYS_ERR;
			break;
		}
		
		/*killllllllllllllll*/
		sysLog_D("initMoniIpc: sleep for other process");
		sleep(gInitSleepTime);
		
		ret = ipcMoniVerifyProcTaskInfo();
		if ( ret != RESULT_OK )
		{
			/*killllllllllllllll*/
			pthread_mutex_unlock(ProcManager.pProcMutex);
			break;
		}

		sysLog_D("initMoniIpc: unlock process mutex");
		pthread_mutex_unlock(ProcManager.pProcMutex);
		
	}while (0);

	return ret;
	
}

//normal process
INT32 initIpc()
{
	INT32 ret = RESULT_OK;
	
	do
	{
		ret = ipcHandleProcCfg();
		if ( ret != RESULT_OK )
		{
			break;
		}

		ret = ipcInitProcMutex();
		if ( ret != RESULT_OK )
		{
			break;
		}
		
		ret = ipcinitTaskInfoShmBuf();
		if ( ret != RESULT_OK )
		{
			break;
		}

		sysLog_D("initIpc: lock process mutex");

		if ( 0 != pthread_mutex_lock(ProcManager.pProcMutex ) )
		{
			ret = RESULT_OPER_SYS_ERR;
			break;
		}
		sysLog_D("initIpc: unlock process mutex");
		pthread_mutex_unlock(ProcManager.pProcMutex);

		ret = ipcCreateMsgQue();
		if ( ret != RESULT_OK )
		{
			break;
		}

	}while (0);
		
	return ret;
}


