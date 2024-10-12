#ifndef __MONITOR_H
#define __MONITOR_H
#include "stm32f1xx_hal.h"
#include <stdio.h>
#include  "thread.h"

typedef struct TaskWatchdog {
    TaskControlBlock *task;   // ��ص�����
    uint32_t lastKickTime;    // ��һ�θ�λʱ��
    uint32_t timeout;         // ��ʱʱ��
    struct TaskWatchdog *next; // ����ָ��
} TaskWatchdog;



uint32_t getSysTime(void);
uint32_t getTaskSwitchDelay(void);
void monitorTaskWatchdog(void);
void kickTaskWatchdog(TaskControlBlock *task);
void initTaskWatchdog(TaskControlBlock *task, uint32_t timeout);

#endif