#ifndef _CORE_H_
#define _CORE_H_
#include "all.h"

#pragma pack(1)

#define snprintf snprintf


void * GET_MEM(INT32 size);
void   RET_MEM(void *pData);


void  GetIniKeyString(INT8 *szTitle, INT8 *szKey, INT8 *szBuf, INT32 BufLen, INT8 *szFilePath );
INT32 GetIniKeyInt(INT8 *szTitle, INT8 *szKey, INT8 *szFilePath);
INT32 InitCfgPath();
void  TrimLeft(INT8 *s);
void  Trim(INT8 *s);
void  TrimRight(INT8 *s);


#endif
