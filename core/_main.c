#include "all.h"

extern INT8 g_szCfgFilePath[MAX_FILE_PATH_LEN];

TaskItem_T TaskItems[MAX_TASK_NUM] = {
		{1, "testTask", NULL, 1024, 10},
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
	
	while( i==0 )
	{
		i ++;
		sleep(100000000);
		i --;
	}
}

