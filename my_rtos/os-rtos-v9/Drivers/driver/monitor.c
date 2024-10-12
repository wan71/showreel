#include "monitor.h"
#include "heap_1.h"

TaskWatchdog *watchdogList = NULL; // �����Ź�����


/*1. ��ȡϵͳ��ǰʱ��*/
uint32_t getSysTime(void) {
    return HAL_GetTick();  // ʹ��ϵͳ�δ��ʱ��������ʱ�䣨��λ��ms��
}


/*2. �����л��ӳ�*/
uint32_t getTaskSwitchDelay(void) {
    static uint32_t lastSwitchTime = 0;
    uint32_t currentTime = getSysTime();
    uint32_t switchDelay = currentTime - lastSwitchTime;
    lastSwitchTime = currentTime;
    return switchDelay;
}

// ��ʼ�������Ź���
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

//��λ�����Ź���Kick the Watchdog��
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


//�������Ŀ��Ź�����
void monitorTaskWatchdog(void) {
    TaskWatchdog *wd = watchdogList;
    uint32_t currentTime = getSysTime();
    
    while (wd != NULL) {
        if (currentTime - wd->lastKickTime > wd->timeout) {
            printf("Task %s timeout! Taking action...\r\n", wd->task->pcTaskName);
            // ���������ӳ�ʱ������ƣ��������������ϵͳ��λ
        }
        wd = wd->next;
    }
}



