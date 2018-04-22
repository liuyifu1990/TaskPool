#ifndef _IPC_H
#define _IPC_H

#pragma pack(1)

#define IPC_KEY_SHMBUF   0
#define IPC_KEY_MSGQUE   1
#define IPC_KEY_MUTEX	 2

INT32 initIpc(); //for normal process
INT32 initMoniIpc(); //for monitor process


#endif
