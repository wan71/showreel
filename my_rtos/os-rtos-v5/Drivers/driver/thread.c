#include <thread.h>
#include "stm32f1xx_hal.h"

//TaskControlBlock *currentTask;
//TaskControlBlock *nextTask;
TaskControlBlock *currentTask = NULL;  // ��ǰ��������
TaskControlBlock *nextTask = NULL;     // ��һ������

TaskControlBlock *taskLists[MAX_PRIORITY];  // �������ȼ�����
TaskControlBlock *delayList = NULL;         // ��ʱ��������
int taskCount = 0;  // ��ǰ��������

//stack_task taskStacks[MAX_TASKS][TASK_STACK_SIZE];  // ÿ������Ķ�ջ

void addTask(TaskControlBlock *newTask) {
    if (currentTask == NULL) {
        // �����ǰû������ֱ�ӽ���������Ϊ����ĵ�һ������
        currentTask = newTask;
        currentTask->next = currentTask;  // ��ѭ������������ʱָ���Լ�
    } else {
        // ���������ҵ�ĩβ
        TaskControlBlock *temp = currentTask;
        while (temp->next != currentTask) {
            temp = temp->next;
        }
        // ��������ӵ�ĩβ���γ�ѭ������
        temp->next = newTask;
        newTask->next = currentTask;  // �������nextָ������ĵ�һ������
    }
}

void addTaskToList(TaskControlBlock *task) {
    int priority = task->priority;
    if (priority >= MAX_PRIORITY || priority < 0) {
        printf("Invalid priority!\n");
        return;
    }

    task->next = NULL;

    if (taskLists[priority] == NULL) {
        taskLists[priority] = task;  // �������Ϊ�գ�ֱ�Ӽ���
    } else {
        TaskControlBlock *current = taskLists[priority];
        while (current->next != NULL) {
            current = current->next;  // ����ĩβ�������
        }
        current->next = task;
    }
}




int createTask(void (*taskFunc)(void *), void *param,int priority,int yield ,TaskHandle_t * pxCreatedTask ) {
    if (taskCount < MAX_TASKS ) {
//        TaskControlBlock *tcb = &tasks[taskCount++];  // ��ȡһ���յ�������ƿ�
			TaskControlBlock *tcb = (TaskControlBlock *)malloc(sizeof(TaskControlBlock));
			if (tcb == NULL) {
					printf("Memory allocation failed\r\n");
					return 0;  // �ڴ����ʧ��
			}
        uint32_t *stack = &(tcb->stack[TASK_STACK_SIZE]);  // ָ������ջ�Ķ���

        // ��ʼ������Ķ�ջ (xPSR, PC, LR, R12, R3-R0, R11-R4)
        *(--stack) = (uint32_t)0x01000000;  // xPSR: Thumb״̬
        *(--stack) = (uint32_t)taskFunc;    // PC: ������ں�����ַ
        *(--stack) = (uint32_t)0xFFFFFFFD;  // LR: ���ص��߳�ģʽ
        *(--stack) = (uint32_t)0x12121212;  // R12: ����ֵ
        *(--stack) = (uint32_t)0x03030303;  // R3
        *(--stack) = (uint32_t)0x02020202;  // R2
        *(--stack) = (uint32_t)0x01010101;  // R1
        *(--stack) = (uint32_t)param;       // R0: �������

        // ��ʼ��R11-R4��ֵ
        *(--stack) = (uint32_t)0x11111111;  // R11
        *(--stack) = (uint32_t)0x10101010;  // R10
        *(--stack) = (uint32_t)0x09090909;  // R9
        *(--stack) = (uint32_t)0x08080808;  // R8
        *(--stack) = (uint32_t)0x07070707;  // R7
        *(--stack) = (uint32_t)0x06060606;  // R6
        *(--stack) = (uint32_t)0x05050505;  // R5
        *(--stack) = (uint32_t)0x04040404;  // R4

        tcb->stackPointer = stack;  // ����TCB�Ķ�ջָ��
				tcb->priority = priority;
				tcb->yield=yield;
				tcb->next = NULL;
        addTaskToList(tcb);  // ��������ӵ���������
				taskCount++;	
				if(pxCreatedTask != NULL)
				{	
				*pxCreatedTask=(TaskHandle_t)tcb;
				}
				return 1;
			}
		return 0;  // ������������������� NULL
    
}



//�ú�����������е�ͷ��ַ�������β��
//������һ��������е�ͷ����yield�����Ϊ1���������á�
int jump_wei(TaskControlBlock *firstTask,int i)
{
	    TaskControlBlock *lastTask = firstTask;
      // ����������ĩβ
      while (lastTask->next != NULL) {
      lastTask = lastTask->next;
      }
      // ����ǰ�����Ƶ�ĩβ������������ָ��
      lastTask->next = firstTask;
      taskLists[i] = firstTask->next;  // ����ͷ�Ƶ���һ������
      firstTask->next = NULL;          // ��ǰ����� next �ÿ�	
			return taskLists[i]->yield;
	
}


void schedule(void) {
		enterCritical();  // �����ٽ���
    // �����������ȼ��������б��Ӹ����ȼ��������ȼ�
    for (int i = MAX_PRIORITY - 1; i >= 0; i--) {
        if (taskLists[i] != NULL) {
           
            if (currentTask != NULL && currentTask->priority <= i) {    // �����ǰ������ͬ���ȼ�����
                TaskControlBlock *firstTask = taskLists[i]; // ȡ������������
							  
                if (firstTask->next != NULL && currentTask == firstTask) {  
									// ������������ֻ��һ������ ���� ����һ���ս������̬������ֱ�ӽ���Ĭ��
									// ��������ǣ�֤����ǰ������Ǹ�������е�ͷ�������Ҹ�������в�Ψһ�����ǽ�ͷ���Ƶ�ĩβ��������о������ͬ��������ĵ���
									while(jump_wei(taskLists[i],i))
									{
										if(currentTask == taskLists[i]) //���currentTask == taskLists[i],˵����������ж����ã���
										{
											jump_wei(taskLists[i],i);
											break;
										}
										
									}
							
                		 // ����ǰ�����Ƶ�ĩβ������������ָ��
            }
					}

            // Ĭ���¸����������ȼ�������е�ͷ��
            nextTask = taskLists[i];	
            return;
        }
				exitCritical();   // �˳��ٽ���
    }

    // ���û��������������У��л�����������
//    nextTask = idleTask;
}



void os_schedule_start(void)
{
//	schedule(1);
	
	PendSV_init();
	
}

void deleteTask(TaskControlBlock *tcb) {
    int priority = tcb->priority;
    
    if (taskLists[priority] == NULL) {
        return;  // û������
    }

    TaskControlBlock *temp = taskLists[priority];
    TaskControlBlock *prev = NULL;

    while (temp != NULL) {
        if (temp == tcb) {
            if (prev == NULL) {
                taskLists[priority] = temp->next;  // �����ͷ��㣬ֱ���Ƴ�
            } else {
                prev->next = temp->next;  // ��������ɾ��
            }
            free(temp);
            return;
        }
        prev = temp;
        temp = temp->next;
    }
}

/*
removeTaskFromTaskList(task) �������ڽ�����Ӷ�Ӧ�� taskLists ���Ƴ���
��������Ĺ���ԭ���Ǳ���ָ�����ȼ������������ҵ�Ŀ�����񲢽�����������Ƴ�
*/
void removeTaskFromTaskList(TaskHandle_t p) {
	if(p==NULL)
		{
			p=(void *)currentTask;
		}
		TaskControlBlock *task=(TaskControlBlock *) p;
    int priority = task->priority;

    if (taskLists[priority] == NULL) {
        // ��������б�Ϊ�գ�ֱ�ӷ���
        return;
    }

    TaskControlBlock *current = taskLists[priority];
    TaskControlBlock *prev = NULL;

    // �������ȼ���������Ѱ��Ŀ������
    while (current != NULL) {
        if (current == task) {
            // �ҵ������Ƴ���
            if (prev == NULL) {
                // �������������ͷ����ֱ�ӽ�ͷָ��ָ����һ������
                taskLists[priority] = current->next;
            } else {
                // ��������������м��ĩβ������ǰһ������� next ָ��
                prev->next = current->next;
            }
            task->next = NULL;  // ��������� next ָ�����
            return;
        }
        prev = current;
        current = current->next;
    }
}

void removeTaskFromDelayList(TaskHandle_t p)
{
	if(p==NULL)
		{
			p=(void *)currentTask;
		}
		TaskControlBlock *task=(TaskControlBlock *) p;
		
		int priority = task->priority;

    if (delayList == NULL) {
        // ��������б�Ϊ�գ�ֱ�ӷ���
        return;
    }
		TaskControlBlock *current = delayList;
    TaskControlBlock *prev = NULL;
		  // �������ȼ���������Ѱ��Ŀ������
    while (current != NULL) {
        if (current == task) {
            // �ҵ������Ƴ���
            if (prev == NULL) {
                // �������������ͷ����ֱ�ӽ�ͷָ��ָ����һ������					
                delayList = current->next;	
            } else {
                // ��������������м��ĩβ������ǰһ������� next ָ��
                prev->next = current->next;
            }
            task->next = NULL;  // ��������� next ָ�����
            return;
        }
        prev = current;
        current = current->next;
    }
	
}

void addTaskToTaskList(TaskControlBlock *task) {
    int priority = task->priority;
    TaskControlBlock *current = taskLists[priority];

    // �������ȼ����У���������Ƿ��Ѿ�����
    while (current != NULL) {
        if (current == task) {
            // �����Ѵ��ڣ�ֱ�ӷ���
            return;
        }
        current = current->next;
    }

    // ������������ȼ������У����뵽���е�ͷ��
    task->next = taskLists[priority];
    taskLists[priority] = task;
}

volatile uint8_t pendSVTriggered = 0;  // ��־λ
/*
��ĳ�����������ʱ״̬ʱ������� taskLists ���� delayList��
*/
void addTaskToDelayList(TaskControlBlock *task, uint32_t delayTime) {
		enterCritical();
    // ������� taskLists ���Ƴ�
    removeTaskFromTaskList(task);

    // �����������ʱ
    task->delay = delayTime;

	    if (delayList == NULL) {
        delayList = task;  // �������Ϊ�գ�ֱ�Ӽ���
				
    } else {
        TaskControlBlock *current = delayList;
        while (current->next != NULL) {
					
            current = current->next;  // ����ĩβ�������
        }
        current->next = task;
				
    }
		
		task->next=NULL;
		
		//����SystemClock
		HAL_SYSTICK_Config(SystemCoreClock / (1000U / uwTickFreq));
		// ���΢С���ӳ٣���ֹ�����л�����
		exitCritical();
//		triggerPendSV();
		HAL_Delay(1);
    // ������������
//		// ���û�д��������������򴥷�һ��
//    if (!pendSVTriggered) {
//        pendSVTriggered = 1;
//        triggerPendSV();  // ���� PendSV �����л�
//    }

		
    
}

void Tsakdelay(uint32_t delayTime)
{

	TaskControlBlock *current = currentTask;
	addTaskToDelayList(currentTask,delayTime);
	
	
}

/*ÿ��ʱ���ж�ʱ������ delayList��������ʱʱ�䣬�����ʱ�������������ƻض�Ӧ���ȼ��� taskLists*/

void every_delay(void) {
    TaskControlBlock *prev = NULL;
    TaskControlBlock *current = delayList;
	 while (current != NULL) {
		  if (current->delay > 0) {
            current->delay--;  // ������ʱʱ��
        }
		 if (current->delay == 0) {
				  // ��������뵽��Ӧ���ȼ��� taskLists
					  removeTaskFromDelayList(current);
            addTaskToTaskList(current);
			 }
//			 
			 // ��������
        prev = current;
        current = current->next;
			 
	 }
	
}

