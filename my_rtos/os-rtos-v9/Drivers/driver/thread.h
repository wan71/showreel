#ifndef __THREAD_H
#define __THREAD_H
#include <stdint.h>


#define MAX_TASKS 32   			// ����������
#define MAX_PRIORITY 3        // ������ȼ���
#define TASK_STACK_SIZE 128
#define configMAX_TASK_NAME_LEN                  ( 16 )
typedef uint32_t stack_task;
typedef void * TaskHandle_t;
#define portMAX_DELAY ( TickType_t ) 0xffffffffUL




typedef struct TaskControlBlock {
    uint32_t *stackPointer;          // ����Ķ�ջָ��
    uint32_t *stack;                 // ָ�������ջ�ռ�
    int stackSize;                   // ջ��С
    int priority;                    // �������ȼ�
	  uint32_t delay;
    int yield;                       // �Ƿ��������ã�1: ����, 0: �����ã�
		char	pcTaskName[ configMAX_TASK_NAME_LEN ];  //����
		uint32_t totalRunTime;  // �ۼ�����ʱ��
    uint32_t lastStartTime; // �ϴ�����ʱ��
		struct TaskControlBlock *nextInMonitorList;   // ָ����һ������ļ��ָ��
    struct TaskControlBlock *next;   // ָ����һ�������ָ��
} TaskControlBlock;





//�����ʼ������.
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

