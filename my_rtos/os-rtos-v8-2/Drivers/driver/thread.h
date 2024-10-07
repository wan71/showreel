#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>


#define MAX_TASKS 4   			// ����������
#define MAX_PRIORITY 3        // ������ȼ���
#define TASK_STACK_SIZE 128

typedef uint32_t stack_task;
typedef void * TaskHandle_t;
#define portMAX_DELAY ( TickType_t ) 0xffffffffUL



//typedef struct TaskControlBlock {
//    uint32_t *stackPointer;          // ����Ķ�ջָ��
//    uint32_t stack[TASK_STACK_SIZE];             // ÿ������Ķ���ջ�ռ䣬��СΪ128����
//		int priority;       // �������ȼ�
//		int yield;                       // �Ƿ��������ã�1: ����, 0: �����ã�
//    struct TaskControlBlock *next;   // ָ����һ�������ָ��
//} TaskControlBlock;

typedef struct TaskControlBlock {
    uint32_t *stackPointer;          // ����Ķ�ջָ��
    uint32_t *stack;                 // ָ�������ջ�ռ�
    int stackSize;                   // ջ��С
    int priority;                    // �������ȼ�
	  uint32_t delay;
    int yield;                       // �Ƿ��������ã�1: ����, 0: �����ã�
    struct TaskControlBlock *next;   // ָ����һ�������ָ��
} TaskControlBlock;


//extern TaskControlBlock *currentTask;
//extern TaskControlBlock *nextTask;
//extern TaskControlBlock taskTable[2];
//�����ʼ������.
int createTask(void (*taskFunc)(void *), void *param, int priority, int yield, int stackSize,TaskHandle_t * pxCreatedTask);
void schedule(void);
void os_schedule_start(void);
int AddTaskptob(TaskHandle_t p, TaskHandle_t b) ;
int removeTaskpfroma(TaskHandle_t a,TaskHandle_t p);
int set_task_delay(TaskHandle_t p,uint32_t delay);
void Tsakdelay(uint32_t delay);
void every_delay(void);
#endif

