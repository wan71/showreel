#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>
#include <string.h>
#define MAX_TASKS 5   			// 最大任务个数
#define MAX_PRIORITY 3        // 最大优先级数
#define TASK_STACK_SIZE 64

typedef uint32_t stack_task;

//typedef struct _losTask{
//	stack_task *stackPointer; 	// 任务堆栈指针
//	int priority;       // 任务优先级
//  int delay;          // 延迟时间
//}TaskControlBlock;

typedef struct TaskControlBlock {
    uint32_t *stackPointer;          // 任务的堆栈指针
    uint32_t stack[TASK_STACK_SIZE];             // 每个任务的独立栈空间，大小为128个字
		int priority;       // 任务优先级
		int yield;                       // 是否设置礼让（1: 礼让, 0: 不礼让）
		uint32_t delay;                  // 延时时间
    struct TaskControlBlock *next;   // 指向下一个任务的指针
} TaskControlBlock;
typedef void * TaskHandle_t;

//extern TaskControlBlock *currentTask;
//extern TaskControlBlock *nextTask;
//extern TaskControlBlock taskTable[2];
//任务初始化函数.
int createTask(void (*taskFunc)(void *), void *param,int priority,int yield ,TaskHandle_t * pxCreatedTask );
//TaskControlBlock* createTask(void (*taskFunc)(void *), void *param,int priority,int yield );
//void createTask(void (*taskFunc)(void *), int priority, uint32_t *stack);
//void schedule(int a);
void schedule(void);
void os_schedule_start(void);
void deleteTask(TaskControlBlock *tcb);
void removeTaskFromTaskList(TaskHandle_t p);
void addTaskToTaskList(TaskControlBlock *task);
void addTaskToDelayList(TaskControlBlock *task, uint32_t delayTime);
void Tsakdelay(uint32_t delayTime);
void every_delay(void);
//void removeTaskFromTaskList(TaskControlBlock *task);
#endif

