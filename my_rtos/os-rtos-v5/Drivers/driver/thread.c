#include <thread.h>
#include "stm32f1xx_hal.h"

//TaskControlBlock *currentTask;
//TaskControlBlock *nextTask;
TaskControlBlock *currentTask = NULL;  // 当前运行任务
TaskControlBlock *nextTask = NULL;     // 下一个任务

TaskControlBlock *taskLists[MAX_PRIORITY];  // 任务优先级队列
TaskControlBlock *delayList = NULL;         // 延时任务链表
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




int createTask(void (*taskFunc)(void *), void *param,int priority,int yield ,TaskHandle_t * pxCreatedTask ) {
    if (taskCount < MAX_TASKS ) {
//        TaskControlBlock *tcb = &tasks[taskCount++];  // 获取一个空的任务控制块
			TaskControlBlock *tcb = (TaskControlBlock *)malloc(sizeof(TaskControlBlock));
			if (tcb == NULL) {
					printf("Memory allocation failed\r\n");
					return 0;  // 内存分配失败
			}
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
				tcb->yield=yield;
				tcb->next = NULL;
        addTaskToList(tcb);  // 将任务添加到任务链表
				taskCount++;	
				if(pxCreatedTask != NULL)
				{	
				*pxCreatedTask=(TaskHandle_t)tcb;
				}
				return 1;
			}
		return 0;  // 如果任务数已满，返回 NULL
    
}



//该函数将任务队列的头地址放入队列尾部
//返回下一个任务队列的头部的yield，如果为1代表了礼让。
int jump_wei(TaskControlBlock *firstTask,int i)
{
	    TaskControlBlock *lastTask = firstTask;
      // 遍历到队列末尾
      while (lastTask->next != NULL) {
      lastTask = lastTask->next;
      }
      // 将当前任务移到末尾，并处理链表指针
      lastTask->next = firstTask;
      taskLists[i] = firstTask->next;  // 队列头移到下一个任务
      firstTask->next = NULL;          // 当前任务的 next 置空	
			return taskLists[i]->yield;
	
}


void schedule(void) {
		enterCritical();  // 进入临界区
    // 遍历所有优先级的任务列表，从高优先级到低优先级
    for (int i = MAX_PRIORITY - 1; i >= 0; i--) {
        if (taskLists[i] != NULL) {
           
            if (currentTask != NULL && currentTask->priority <= i) {    // 如果当前任务是同优先级任务
                TaskControlBlock *firstTask = taskLists[i]; // 取出队列首任务
							  
                if (firstTask->next != NULL && currentTask == firstTask) {  
									// 如果该任务队列只有一个任务 或者 这是一个刚进入就绪态的任务，直接进入默认
									// 如果都不是，证明当前任务就是该任务队列的头部，并且该任务队列不唯一，我们将头部移到末尾该任务队列就完成了同级别任务的调度
									while(jump_wei(taskLists[i],i))
									{
										if(currentTask == taskLists[i]) //如果currentTask == taskLists[i],说明该任务队列都礼让，就
										{
											jump_wei(taskLists[i],i);
											break;
										}
										
									}
							
                		 // 将当前任务移到末尾，并处理链表指针
            }
					}

            // 默认下个任务，是优先级任务队列的头部
            nextTask = taskLists[i];	
            return;
        }
				exitCritical();   // 退出临界区
    }

    // 如果没有其他任务可运行，切换到空闲任务
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

/*
removeTaskFromTaskList(task) 函数用于将任务从对应的 taskLists 中移除。
这个函数的工作原理是遍历指定优先级的任务链表，找到目标任务并将其从链表中移除
*/
void removeTaskFromTaskList(TaskHandle_t p) {
	if(p==NULL)
		{
			p=(void *)currentTask;
		}
		TaskControlBlock *task=(TaskControlBlock *) p;
    int priority = task->priority;

    if (taskLists[priority] == NULL) {
        // 如果任务列表为空，直接返回
        return;
    }

    TaskControlBlock *current = taskLists[priority];
    TaskControlBlock *prev = NULL;

    // 遍历优先级任务链表，寻找目标任务
    while (current != NULL) {
        if (current == task) {
            // 找到任务，移除它
            if (prev == NULL) {
                // 如果任务在链表头部，直接将头指针指向下一个任务
                taskLists[priority] = current->next;
            } else {
                // 如果任务在链表中间或末尾，调整前一个任务的 next 指针
                prev->next = current->next;
            }
            task->next = NULL;  // 将该任务的 next 指针清空
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
        // 如果任务列表为空，直接返回
        return;
    }
		TaskControlBlock *current = delayList;
    TaskControlBlock *prev = NULL;
		  // 遍历优先级任务链表，寻找目标任务
    while (current != NULL) {
        if (current == task) {
            // 找到任务，移除它
            if (prev == NULL) {
                // 如果任务在链表头部，直接将头指针指向下一个任务					
                delayList = current->next;	
            } else {
                // 如果任务在链表中间或末尾，调整前一个任务的 next 指针
                prev->next = current->next;
            }
            task->next = NULL;  // 将该任务的 next 指针清空
            return;
        }
        prev = current;
        current = current->next;
    }
	
}

void addTaskToTaskList(TaskControlBlock *task) {
    int priority = task->priority;
    TaskControlBlock *current = taskLists[priority];

    // 遍历优先级队列，检查任务是否已经存在
    while (current != NULL) {
        if (current == task) {
            // 任务已存在，直接返回
            return;
        }
        current = current->next;
    }

    // 如果任务不在优先级队列中，插入到队列的头部
    task->next = taskLists[priority];
    taskLists[priority] = task;
}

volatile uint8_t pendSVTriggered = 0;  // 标志位
/*
当某个任务进入延时状态时，将其从 taskLists 移入 delayList：
*/
void addTaskToDelayList(TaskControlBlock *task, uint32_t delayTime) {
		enterCritical();
    // 将任务从 taskLists 中移除
    removeTaskFromTaskList(task);

    // 设置任务的延时
    task->delay = delayTime;

	    if (delayList == NULL) {
        delayList = task;  // 如果链表为空，直接加入
				
    } else {
        TaskControlBlock *current = delayList;
        while (current->next != NULL) {
					
            current = current->next;  // 链表末尾添加任务
        }
        current->next = task;
				
    }
		
		task->next=NULL;
		
		//重置SystemClock
		HAL_SYSTICK_Config(SystemCoreClock / (1000U / uwTickFreq));
		// 添加微小的延迟，防止过快切换任务
		exitCritical();
//		triggerPendSV();
		HAL_Delay(1);
    // 调度其他任务
//		// 如果没有触发过调度请求，则触发一次
//    if (!pendSVTriggered) {
//        pendSVTriggered = 1;
//        triggerPendSV();  // 触发 PendSV 任务切换
//    }

		
    
}

void Tsakdelay(uint32_t delayTime)
{

	TaskControlBlock *current = currentTask;
	addTaskToDelayList(currentTask,delayTime);
	
	
}

/*每次时钟中断时，遍历 delayList，减少延时时间，如果延时结束，则将任务移回对应优先级的 taskLists*/

void every_delay(void) {
    TaskControlBlock *prev = NULL;
    TaskControlBlock *current = delayList;
	 while (current != NULL) {
		  if (current->delay > 0) {
            current->delay--;  // 减少延时时间
        }
		 if (current->delay == 0) {
				  // 将任务加入到对应优先级的 taskLists
					  removeTaskFromDelayList(current);
            addTaskToTaskList(current);
			 }
//			 
			 // 继续遍历
        prev = current;
        current = current->next;
			 
	 }
	
}

