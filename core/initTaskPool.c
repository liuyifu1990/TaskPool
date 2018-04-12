#include "all.h"

INT32 ReadTaskCfg()
{

	return RESULT_OK;
}

INT32 InitTaskPool(const TaskItem_T *szTaskItems)
{
	INT32 ret = 0;
	
	do 
	{
		ret = InitCfgPath();
		if ( ret  != RESULT_OK )
		{
			sysLog_D("InitTaskPool: InitCfgPath returns err[%d]", ret );
			break;
		}

		ret = logInit();
		if ( ret  != RESULT_OK )
		{
			sysLog_D("InitTaskPool: logInit returns err[%d]", ret );
			break;
		}

		ret = logRegister("syslog.log", 3, LOG_TAG_APP00);
		if ( ret  != RESULT_OK )
		{
			sysLog_D("InitTaskPool: logRegister syslog returns err[%d]", ret );
			break;
		}

		ret = timerInit(500);
		if ( ret  != RESULT_OK )
		{
			sysLog_D("InitTaskPool: timerInit returns err[%d]", ret );
			break;
		}

		ret = initTaskSingleManage();
		if ( ret  != RESULT_OK )
		{
			sysLog_D("InitTaskPool: initTaskSingleManage returns err[%d]", ret );
			break;
		}

		ret = taskInit(szTaskItems);
		if ( ret  != RESULT_OK )
		{
			sysLog_D("InitTaskPool: taskInit returns err[%d]", ret );
			break;
		}

	}while(0);

	if ( ret == RESULT_OK )
	{
		sysLog_E("taskPool init ok...");
	}
	
	return ret;
}
