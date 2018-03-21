#ifndef _CORE_H_
#define _CORE_H_

#pragma pack(1)

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
};


#endif
