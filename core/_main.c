#include "all.h"

extern CirQueue_T LogQueue;




int main()
{
	pid_t pid;
	INT32 i= 0;

	setsid();
	
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
	
	printf("starting test...\n");
	
	logInit();
	logRegister("test.log",    3, LOG_TAG_APP00);

	

	while( i==0 )
	{
		i ++;
		sleep(1000000000);
		i --;
	}
}



