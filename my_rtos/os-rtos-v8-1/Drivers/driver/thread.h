#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>


#define MAX_TASKS 4   			// 最大任务个数
#define MAX_PRIORITY 3        // 最大优先级数
#define TASK_STACK_SIZE 128

typedef uint32_t stack_task;
typedef void * TaskHandle_t;
#define portMAX_DELAY ( TickType_t ) 0xffffffffUL




typedef struct TaskControlBlock {
    uint32_t *stackPointer;          // 任务的堆栈指针
    uint32_t *stack;                 // 指向任务的栈空间
    int stackSize;                   // 栈大小
    int priority;                    // 任务优先级
	  uint32_t delay;
    int yield;                       // 是否设置礼让（1: 礼让, 0: 不礼让）
    struct TaskControlBlock *next;   // 指向下一个任务的指针
} TaskControlBlock;


//extern TaskControlBlock *currentTask;
//extern TaskControlBlock *nextTask;
//extern TaskControlBlock taskTable[2];
//任务初始化函数.
int createTask(void (*taskFunc)(void *), void *param, int priority, int yield, int stackSize,TaskHandle_t * pxCreatedTask);
void schedule(void);
void os_schedule_start(void);
int AddTaskptob(TaskHandle_t p, TaskHandle_t b) ;
int removeTaskpfroma(TaskHandle_t a,TaskHandle_t p);
int set_task_delay(TaskHandle_t p,uint32_t delay);
void Tsakdelay(uint32_t delay);
void every_delay(void);
#endif

