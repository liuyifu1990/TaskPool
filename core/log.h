#ifndef _LOG_H_
#define _LOG_H_

#include "all.h"

#pragma pack(1)

#define MAX_FILE_PATH_LEN     1024
#define MAX_FILE_NAME_LEN     256
#define MAX_FILE_SUFFIX_LEN   32
#define MAX_FILE_SIZE         5 //文件大小，MB
#define MAX_LOG_QUEUE_LEN     1000
#define MAX_LOG_STR_LINE_LEN  1024

INT32 logInit();
INT32 logRegister(INT8 *szFileName, INT32 iSplitNum, INT8 iLogTag );
void  logWrite(INT8 iLogTag, INT8 *pLogBuf, INT32 iBufLen);
void WriteLogAPP00(INT32 level, INT8 *filename, INT8 *line, const INT8 *szFMT, ... );


typedef struct
{
	INT32 iLogTag;
	INT8 *pLogStr;
	INT32 iLen;
}LogELem_T;

typedef struct
{
	INT8  szFileName[MAX_FILE_NAME_LEN + 1];     //文件名
	INT8  szFileSuffix[MAX_FILE_SUFFIX_LEN + 1]; //文件后缀
	INT8  szCurPath[MAX_FILE_PATH_LEN + 1];      //当前日志文件路径
	INT32 iSeq;
	INT32 iSplitNum;
	FILE  *pFile;
	INT32 iFileSize;
	BOOL  useFlag;
}LogManager_T;

#define MAX_LOG_TAG_NUM 10

#define LOG_TAG_APP00 0
#define LOG_TAG_APP01 1
#define LOG_TAG_APP02 2
#define LOG_TAG_APP03 3
#define LOG_TAG_APP04 4
#define LOG_TAG_APP05 5
#define LOG_TAG_APP06 6
#define LOG_TAG_APP07 7
#define LOG_TAG_APP08 8
#define LOG_TAG_APP09 9

#define LOG_LVL_ERROR  0
#define LOG_LVL_NOTICE 1
#define LOG_LVL_INFO   2
#define LOG_LVL_DEBUG  3


#define Log_D(szFMT, ...)  WriteLogAPP00(LOG_LVL_DEBUG,  __FILE__, __LINE__, szFMT, __VA_ARGS__)
#define Log_I(szFMT, ...)  WriteLogAPP00(LOG_LVL_INFO,   __FILE__, __LINE__, szFMT, __VA_ARGS__)
#define Log_N(szFMT, ...)  WriteLogAPP00(LOG_LVL_NOTICE, __FILE__, __LINE__, szFMT, __VA_ARGS__)
#define Log_E(szFMT, ...)  WriteLogAPP00(LOG_LVL_ERROR,  __FILE__, __LINE__, szFMT, __VA_ARGS__)





#endif

