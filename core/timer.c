#include "all.h"

void getCurTimeStr(INT8 *szTime, INT32 len )
{
	if ( szTime == NULL )
		return;
	
	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	snprintf(szTime, len, "%s", asctime (timeinfo));

	return;
}

