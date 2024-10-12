#ifndef __MONITOR_H
#define __MONITOR_H
#include "stm32f1xx_hal.h"
#include <stdio.h>
#include  "thread.h"

typedef struct TaskWatchdog {
    TaskControlBlock *task;   // 监控的任务
    uint32_t lastKickTime;    // 上一次复位时间
    uint32_t timeout;         // 超时时间
    struct TaskWatchdog *next; // 链表指针
} TaskWatchdog;



uint32_t getSysTime(void);
uint32_t getTaskSwitchDelay(void);
void monitorTaskWatchdog(void);
void kickTaskWatchdog(TaskControlBlock *task);
void initTaskWatchdog(TaskControlBlock *task, uint32_t timeout);

#endif