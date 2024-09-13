#include <thread.h>


//TaskControlBlock *currentTask;
//TaskControlBlock *nextTask;
TaskControlBlock *currentTask = NULL;  // 当前运行任务
TaskControlBlock *nextTask = NULL;     // 下一个任务

TaskControlBlock *taskLists[MAX_PRIORITY];  // 任务链表的头指针
int taskCount = 0;  // 当前任务数量

//stack_task taskStacks[MAX_TASKS][TASK_STACK_SIZE];  // 每个任务的堆栈

void addTask(TaskControlBlock *newTask) {
    if (currentTask == NULL) {
        // 如果当前没有任务，直接将新任务作为链表的第一个任务
        currentTask = newTask;
        currentTask->next = currentTask;  // 自循环，单个任务时指向自己
    } else {
        // 遍历链表，找到末尾
        TaskControlBlock *temp = currentTask;
        while (temp->next != currentTask) {
            temp = temp->next;
        }
        // 将新任务加到末尾，形成循环链表
        temp->next = newTask;
        newTask->next = currentTask;  // 新任务的next指向链表的第一个任务
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
        taskLists[priority] = task;  // 如果链表为空，直接加入
    } else {
        TaskControlBlock *current = taskLists[priority];
        while (current->next != NULL) {
            current = current->next;  // 链表末尾添加任务
        }
        current->next = task;
    }
}




void createTask(void (*taskFunc)(void *), void *param,int priority) {
    if (taskCount < MAX_TASKS ) {
//        TaskControlBlock *tcb = &tasks[taskCount++];  // 获取一个空的任务控制块
			TaskControlBlock *tcb = (TaskControlBlock *)malloc(sizeof(TaskControlBlock));
			if (tcb == NULL) {
					printf("Memory allocation failed\r\n");
					return;  // 内存分配失败
			}
			taskCount++;
//			
        uint32_t *stack = &(tcb->stack[TASK_STACK_SIZE]);  // 指向任务栈的顶端

        // 初始化任务的堆栈 (xPSR, PC, LR, R12, R3-R0, R11-R4)
        *(--stack) = (uint32_t)0x01000000;  // xPSR: Thumb状态
        *(--stack) = (uint32_t)taskFunc;    // PC: 任务入口函数地址
        *(--stack) = (uint32_t)0xFFFFFFFD;  // LR: 返回到线程模式
        *(--stack) = (uint32_t)0x12121212;  // R12: 测试值
        *(--stack) = (uint32_t)0x03030303;  // R3
        *(--stack) = (uint32_t)0x02020202;  // R2
        *(--stack) = (uint32_t)0x01010101;  // R1
        *(--stack) = (uint32_t)param;       // R0: 任务参数

        // 初始化R11-R4的值
        *(--stack) = (uint32_t)0x11111111;  // R11
        *(--stack) = (uint32_t)0x10101010;  // R10
        *(--stack) = (uint32_t)0x09090909;  // R9
        *(--stack) = (uint32_t)0x08080808;  // R8
        *(--stack) = (uint32_t)0x07070707;  // R7
        *(--stack) = (uint32_t)0x06060606;  // R6
        *(--stack) = (uint32_t)0x05050505;  // R5
        *(--stack) = (uint32_t)0x04040404;  // R4

        tcb->stackPointer = stack;  // 更新TCB的堆栈指针
				tcb->priority = priority;
				tcb->next = NULL;
        addTaskToList(tcb);  // 将任务添加到任务链表
			}
    
}


//void schedule() {
//    if (currentTask != NULL) {
////        nextTask = currentTask->next;  // 切换到下一个任务
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
        return;  // 没有任务
    }

    TaskControlBlock *temp = taskLists[priority];
    TaskControlBlock *prev = NULL;

    while (temp != NULL) {
        if (temp == tcb) {
            if (prev == NULL) {
                taskLists[priority] = temp->next;  // 如果是头结点，直接移除
            } else {
                prev->next = temp->next;  // 从链表中删除
            }
            free(temp);
            return;
        }
        prev = temp;
        temp = temp->next;
    }
}



