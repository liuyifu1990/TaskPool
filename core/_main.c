#include "all.h"

extern CirQueue_T LogQueue;
extern INT8 g_szCfgFilePath[MAX_FILE_PATH_LEN];
INT8 szVar1[128] = {0};
INT32 iVar2 = 0;

INT8 szVar3[128] = {0};
INT32 iVar4 = 0;

static void ReadSysCfg()
{
	GetIniKeyString("TEST1", "var1", szVar1, sizeof(szVar1), g_szCfgFilePath );
	iVar2 = GetIniKeyInt("TEST1", "var2", g_szCfgFilePath);

	iVar4 = GetIniKeyInt("TEST2", "var1", g_szCfgFilePath);
	GetIniKeyString("TEST2", "var2", szVar3, sizeof(szVar3), g_szCfgFilePath );
	
	printf("szVar1-%s\n", szVar1);
	printf("iVar2-%d\n", iVar2);
	printf("%s\n", g_szCfgFilePath);
	
	WriteLogAPP00( "file-%s, szVar1-%s, iVar2-%d, iVar4-%d, szVar3-%s"
					, g_szCfgFilePath
					, szVar1
					, iVar2
					, iVar4
					, szVar3 );
}



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
	logRegister("test.log",    3, LOG_TAG_APP00);

	ReadSysCfg();
	
	while( i==0 )
	{
		i ++;
		sleep(10);
		GetIniKeyString("TEST1", "var1", szVar1, sizeof(szVar1), g_szCfgFilePath );
		i --;
	}
}

