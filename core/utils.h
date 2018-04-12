#ifndef _CORE_H_
#define _CORE_H_
//#include "all.h"

#pragma pack(1)

#define snprintf snprintf

#define TRUE  1
#define FALSE 0
#define CONF_FILE_PATH  "Config.ini"  

#define THREAD_ENTRY
#define MAX_TASK_NUM 	50

#define MAX_STR16 		16
#define MAX_STR32 		32
#define MAX_STR64 		64
#define MAX_STR128 		128
#define MAX_STR256 		256
#define MAX_STR512 		512
#define MAX_STR1024 	1024
#define MAX_STR2048 	2048

typedef int 		 			INT32;
typedef unsigned int 			UINT32;
typedef char 					INT8;
typedef unsigned char 			UINT8;
typedef unsigned long long int 	UINT64;
typedef long long int 			INT64;
typedef short int               INT16;
typedef unsigned short int      UINT16;
typedef void*               	HANDLE;
typedef pthread_mutex_t MUTEX;
typedef int						TIMERID; 			

#define BOOL int

enum errorCode
{
	RESULT_OK 				= 0,
	RESULT_ALLOC_ERR 		= 1,
	RESULT_PARA_ERR 		= 2,
	RESULT_UNKNOWN_ERR 		= 3,
	RESULT_QUE_FULL 		= 4,
	RESULT_MUTEX_ERR 		= 5,
	RESULT_FILE_ERR         = 6,
	RESULT_OPER_SYS_ERR     = 7,
	RESULT_INNER_ASEND_ERR  = 8,
	RESULT_TIMER_ERR		= 9,
};
	

void * GET_MEM(INT32 size);
void   RET_MEM(void *pData);


void  GetIniKeyString(INT8 *szTitle, INT8 *szKey, INT8 *szBuf, INT32 BufLen, INT8 *szFilePath );
INT32 GetIniKeyInt(INT8 *szTitle, INT8 *szKey, INT8 *szFilePath);
INT32 InitCfgPath();
void  TrimLeft(INT8 *s);
void  Trim(INT8 *s);
void  TrimRight(INT8 *s);
void  thread_sleep_ms(UINT32 iCnt_ms);
void  thread_sleep_s(UINT32 iCnt_s);
void   getCurTimeStr(INT8 *szTime, INT32 len );


#endif
