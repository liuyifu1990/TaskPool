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
#include "core.h"
#include "log.h"
#include "timer.h"
#include "task.h"


#pragma pack(1)

#define TRUE  1
#define FALSE 0
#define CONF_FILE_PATH  "Config.ini"  

#define THREAD_ENTRY
#define MAX_TASK_NUM 	50

#endif
