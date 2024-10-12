#ifndef __THREAD_H
#define __THREAD_H
#include <stdint.h>


#define MAX_TASKS 32   			// 最大任务个数
#define MAX_PRIORITY 3        // 最大优先级数
#define TASK_STACK_SIZE 128
#define configMAX_TASK_NAME_LEN                  ( 16 )
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
		char	pcTaskName[ configMAX_TASK_NAME_LEN ];  //名字
		uint32_t totalRunTime;  // 累计运行时间
    uint32_t lastStartTime; // 上次启动时间
		struct TaskControlBlock *nextInMonitorList;   // 指向下一个任务的监控指针
    struct TaskControlBlock *next;   // 指向下一个任务的指针
} TaskControlBlock;





//任务初始化函数.
int createTask(void (*taskFunc)(void *),const char * const pcName, void *param, int priority, int yield, int stackSize, TaskHandle_t *pxCreatedTask);
//int createTask(void (*taskFunc)(void *), void *param, int priority, int yield, int stackSize,TaskHandle_t * pxCreatedTask);
void schedule(void);
void os_schedule_start(void);
int AddTaskptob(TaskHandle_t p, TaskHandle_t b) ;
int removeTaskpfroma(TaskHandle_t a,TaskHandle_t p);
int set_task_delay(TaskHandle_t p,uint32_t delay);
void Tsakdelay(uint32_t delay);
void every_delay(void);
#endif

