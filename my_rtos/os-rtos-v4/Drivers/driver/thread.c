#include <thread.h>


//TaskControlBlock *currentTask;
//TaskControlBlock *nextTask;
TaskControlBlock *currentTask = NULL;  // ��ǰ��������
TaskControlBlock *nextTask = NULL;     // ��һ������

TaskControlBlock *taskLists[MAX_PRIORITY];  // ���������ͷָ��
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




void createTask(void (*taskFunc)(void *), void *param,int priority) {
    if (taskCount < MAX_TASKS ) {
//        TaskControlBlock *tcb = &tasks[taskCount++];  // ��ȡһ���յ�������ƿ�
			TaskControlBlock *tcb = (TaskControlBlock *)malloc(sizeof(TaskControlBlock));
			if (tcb == NULL) {
					printf("Memory allocation failed\r\n");
					return;  // �ڴ����ʧ��
			}
			taskCount++;
//			
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
				tcb->next = NULL;
        addTaskToList(tcb);  // ��������ӵ���������
			}
    
}


//void schedule() {
//    if (currentTask != NULL) {
////        nextTask = currentTask->next;  // �л�����һ������
//			 nextTask = currentTask->next ? currentTask->next : taskLists[1];
//			  los_task_switch();
//    }
//}
void schedule(int a) {
    for (int i = MAX_PRIORITY - 1; i >= 0; i--) {
        if (taskLists[i] != NULL) {
            if (currentTask == NULL || currentTask->priority < i) {
                nextTask = taskLists[i];
							  if(a==0)
								{
								los_task_switch();
								}
            } else {
                nextTask = currentTask->next ? currentTask->next : taskLists[i];
							 if(a==0)
							 {
							los_task_switch();
							 }
            }
					
            return;
        }
    }
}

void os_schedule_start(void)
{
	schedule(1);
	los_task_run();
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



