#ifndef _CORE_H_
#define _CORE_H_
#include "all.h"

#pragma pack(1)

#define snprintf snprintf


void * GET_MEM(INT32 size);
void   RET_MEM(void *pData);

void WriteLogAPP00( const INT8 *szFMT, ... );


#endif
