#include "all.h"


void * GET_MEM(INT32 size)
{
	if ( size <= 0 )  
		return NULL;  
	else		
		return malloc(size);
}			    
							
void RET_MEM(void *pData)
{
	if (pData == NULL) 
		return; 
	else 
		free(pData); 
	return; 
}

void WriteLogAPP00( const INT8 *szFMT, ... )
{
	INT8 szLogbuf[MAX_LOG_STR_LINE_LEN + 1] = {0};
	/*
	*TODO:添加日志产生的文件，函数信息，以及时间戳、日志分级宏定义
	*/
	va_list args; 
	va_start(args, szFMT);
	snprintf(szLogbuf, MAX_LOG_STR_LINE_LEN, szFMT, args); 
	va_end(args); 

	if ( szLogbuf[0] == '\0' )
		return;

	logWrite(LOG_TAG_APP00, szLogbuf, strlen(szLogbuf) );
}

