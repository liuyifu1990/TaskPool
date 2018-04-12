#include "all.h"

extern INT8 g_szCfgFilePath[MAX_FILE_PATH_LEN];
extern INT32 InitTaskPool(const TaskItem_T *szTaskItems);

#define TASK_TEST_EVENT 1003
void testTask1( UINT32 event, INT8 *pMsg, INT32 iLen )
{
	TID_T SelfTID = {0};
	TID_T SenderTID = {0};
	static INT32 icnt = 0;
	static TIMERID timerId = 0;
	
	getSelfTid(&SelfTID);
	CurSender(&SenderTID);	
	TID_T rec = {0};
	
	switch(event)
	{
		case INIT_TASK_EVENT:
		{
			INT32 ret = 0;

			rec.iTno = 1;

			//ret = ASend(TASK_TEST_EVENT, "testMsg", 7, &rec);
			timerId = setLpTimer(50, TASK_TEST_EVENT, NULL, 0 );
			break;
		}
		case TASK_TEST_EVENT:
		{
			INT32 ret = 0;
			icnt ++;
			
			sysLog_D("testTask1: recv TASK_TEST_EVENT event");

			if (icnt == 5)
			{
				sysLog_D("testTask1: kill timer id=%d", timerId);
				killTimer(timerId);
			}
			break;
		}
		case TIMER_1s_EVENT:
		{
			//sysLog_D("testTask1: recv timer 1s event");
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
		//{2, "testTask2", testTask1, 1024, 10},
		//{3, "testTask3", testTask1, 1024, 10},
		//{4, "testTask4", testTask1, 1024, 10},
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
	
	if ( RESULT_OK != InitTaskPool(TaskItems) )
	{
		exit(1);
	}

	printf("%s\n", "taskPool init ok..." );
	
	while( i==0 )
	{
		i ++;
		sleep(100000000);
		i --;
	}
}

