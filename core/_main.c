#include "all.h"

extern INT8 g_szCfgFilePath[MAX_FILE_PATH_LEN];

void testTask1( INT16 event, INT8 *pMsg, INT32 iLen )
{
	switch(event)
	{
		case INIT_TASK_EVENT:
		{
			sysLog_D("testTask1: recv INIT_TASK_EVENT" );
			break;
		}
		case QUIT_TASK_EVENT:
		{
			sysLog_D("testTask1: recv QUIT_TASK_EVENT" );
			break;
		}
		default:
		{
			sysLog_D("testTask1: recv known event-%d", event );
			break;
		}
	}

	return;
}

TaskItem_T TaskItems[MAX_TASK_NUM] = {
		{1, "testTask1", testTask1, 1024, 10},
};


int main()
{
	pid_t pid;
	INT32 i= 0;
	
	pid = fork(); 
	if (pid < 0) 
	{
		return -1;
	}
	//parent退出
	if (pid > 0)
	{
		_exit(0);
	}
	//子进程成为守护进程
	setsid();
	
	printf("starting test...\n");
	
	InitCfgPath();
	logInit();
	logRegister("syslog.log", 3, LOG_TAG_APP00);

	if ( RESULT_OK != taskInit(TaskItems) )
	{
		sysLog_D("taskInit fail." );
		exit(1);
	}
	
	while( i==0 )
	{
		i ++;
		sleep(100000000);
		i --;
	}
}

