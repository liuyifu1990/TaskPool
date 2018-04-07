#ifndef _ALL_H_
#define _ALL_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>


#include "queue.h"
#include "utils.h"
//#include "core.h"
#include "log.h"
#include "timer.h"
#include "task.h"


#pragma pack(1)

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
};


#endif
