#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>
#include <string.h>
#define MAX_TASKS 5   			// ����������
#define MAX_PRIORITY 3        // ������ȼ���
#define TASK_STACK_SIZE 64

typedef uint32_t stack_task;

//typedef struct _losTask{
//	stack_task *stackPointer; 	// �����ջָ��
//	int priority;       // �������ȼ�
//  int delay;          // �ӳ�ʱ��
//}TaskControlBlock;

typedef struct TaskControlBlock {
    uint32_t *stackPointer;          // ����Ķ�ջָ��
    uint32_t stack[TASK_STACK_SIZE];             // ÿ������Ķ���ջ�ռ䣬��СΪ128����
		int priority;       // �������ȼ�
		int yield;                       // �Ƿ��������ã�1: ����, 0: �����ã�
		uint32_t delay;                  // ��ʱʱ��
    struct TaskControlBlock *next;   // ָ����һ�������ָ��
} TaskControlBlock;
typedef void * TaskHandle_t;

//extern TaskControlBlock *currentTask;
//extern TaskControlBlock *nextTask;
//extern TaskControlBlock taskTable[2];
//�����ʼ������.
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

