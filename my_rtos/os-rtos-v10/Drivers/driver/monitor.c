#include "monitor.h"
#include "heap_1.h"

TaskWatchdog *watchdogList = NULL; // 任务看门狗链表


/*1. 获取系统当前时间*/
uint32_t getSysTime(void) {
    return HAL_GetTick();  // 使用系统滴答计时器，返回时间（单位：ms）
}


/*2. 任务切换延迟*/
uint32_t getTaskSwitchDelay(void) {
    static uint32_t lastSwitchTime = 0;
    uint32_t currentTime = getSysTime();
    uint32_t switchDelay = currentTime - lastSwitchTime;
    lastSwitchTime = currentTime;
    return switchDelay;
}

// 初始化任务看门狗：
void initTaskWatchdog(TaskControlBlock *task, uint32_t timeout) {
    TaskWatchdog *newWatchdog = (TaskWatchdog *) pvPortMalloc(sizeof(TaskWatchdog));
    if (newWatchdog != NULL) {
        newWatchdog->task = task;
        newWatchdog->lastKickTime = getSysTime();
        newWatchdog->timeout = timeout;
        newWatchdog->next = watchdogList;
        watchdogList = newWatchdog;
    }
}

//复位任务看门狗（Kick the Watchdog）
void kickTaskWatchdog(TaskControlBlock *task) {
    TaskWatchdog *wd = watchdogList;
    while (wd != NULL) {
        if (wd->task == task) {
            wd->lastKickTime = getSysTime();
            break;
        }
        wd = wd->next;
    }
}


//监控任务的看门狗函数
void monitorTaskWatchdog(void) {
    TaskWatchdog *wd = watchdogList;
    uint32_t currentTime = getSysTime();
    
    while (wd != NULL) {
        if (currentTime - wd->lastKickTime > wd->timeout) {
            printf("Task %s timeout! Taking action...\r\n", wd->task->pcTaskName);
            // 这里可以添加超时处理机制，比如重启任务或系统复位
        }
        wd = wd->next;
    }
}



