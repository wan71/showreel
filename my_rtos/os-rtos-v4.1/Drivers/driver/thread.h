#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>
#include <string.h>
#define MAX_TASKS 4   			// ����������
#define MAX_PRIORITY 3        // ������ȼ���
#define TASK_STACK_SIZE 128

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
    struct TaskControlBlock *next;   // ָ����һ�������ָ��
} TaskControlBlock;


//extern TaskControlBlock *currentTask;
//extern TaskControlBlock *nextTask;
//extern TaskControlBlock taskTable[2];
//�����ʼ������.
void createTask(void (*taskFunc)(void *), void *param,int priority,int yield );
//void createTask(void (*taskFunc)(void *), int priority, uint32_t *stack);
//void schedule(int a);
void schedule(void);
void os_schedule_start(void);
void deleteTask(TaskControlBlock *tcb);
#endif

