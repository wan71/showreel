#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>
#include <string.h>
#define MAX_TASKS 3
#define TASK_STACK_SIZE 28

typedef uint32_t stack_task;

typedef struct _losTask{
	stack_task *stackPointer; 	// 任务堆栈指针
	int priority;       // 任务优先级
  int delay;          // 延迟时间
}TaskControlBlock;




//extern TaskControlBlock *currentTask;
//extern TaskControlBlock *nextTask;
//extern TaskControlBlock taskTable[2];
//任务初始化函数.


void createTask(void (*taskFunc)(void *), int priority, uint32_t *stack);
void los_task_init(TaskControlBlock * task,void (*taskEntry)(void *),void *param,stack_task *stack);
void los_task_run();
void los_task_switch();
void los_task_sched();
void schedule();
void taskDelayHandler();
void setTaskDelay(uint8_t taskIndex, uint32_t delay);
#endif

