#include <thread.h>
#include "stm32f1xx_hal.h"

//TaskControlBlock *currentTask;
//TaskControlBlock *nextTask;
TaskControlBlock *currentTask = NULL;  // 当前运行任务
TaskControlBlock *nextTask = NULL;     // 下一个任务

TaskControlBlock *taskLists[MAX_PRIORITY];  // 任务链表的头指针
TaskControlBlock *delayList = NULL;         // 延时任务链表
int taskCount = 0;  // 当前任务数量


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
/*
该函数作用为将任务p从链表a中删除
*/

int removeTaskpfroma(TaskHandle_t a, TaskHandle_t p)
{
	 enterCritical();  // 进入临界区
    if (p == NULL) {
        p = (void *)currentTask;
    }

    TaskControlBlock *task = (TaskControlBlock *)p;
    TaskControlBlock **listHead = (TaskControlBlock **)a;
    TaskControlBlock *current = *listHead;
    TaskControlBlock *prev = NULL;

   

    // 如果链表为空，直接返回
    if (current == NULL) {
        exitCritical();  // 退出临界区
        return 0;
    }

    // 遍历链表寻找目标任务
    while (current != NULL) {
        if (current == task) {
            // 找到任务，移除它
            if (prev == NULL) {
                // 如果任务在链表头部，更新链表头指针
                *listHead = current->next;
            } else {
                // 如果任务在链表中间或末尾，调整前一个任务的 next 指针
                prev->next = current->next;
            }

            task->next = NULL;  // 清空任务的 next 指针
            exitCritical();  // 退出临界区
            return 1;
        }

        prev = current;
        current = current->next;
    }

    exitCritical();  // 退出临界区
    return 0;  // 如果没有找到任务，返回 0
}


/*
该任务用于将tcb加入到TaskHandle_t b链表中，如果a不等于0，就会使用triggerPendSV();
*/

int AddTaskptob(TaskHandle_t p, TaskHandle_t b, int a) 
{
	enterCritical();  // 进入临界区
    if (p == NULL) {
        p = (void *)currentTask;
    }
    
    TaskControlBlock *task = (TaskControlBlock *)p;
    TaskControlBlock **listHead = (TaskControlBlock **)b;
    
    
    
    // 如果目标链表为空，直接将任务放入链表头
    if (*listHead == NULL) {
        *listHead = task;
    } else {
        // 遍历链表并将任务添加到末尾
        TaskControlBlock *current = *listHead;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = task;
    }

    task->next = NULL;  // 确保新任务是链表的最后一个节点

    exitCritical();  // 退出临界区
    
    // 如果 `a` 不等于 0，则触发 PendSV 并重置 SysTick
    if (a != 0) {
        enterCritical();  // 再次进入临界区
        HAL_SYSTICK_Config(SystemCoreClock / (1000U / uwTickFreq));  // 重置 SystemClock
        exitCritical();  // 退出临界区
//			
        triggerPendSV();  // 触发 PendSV 中断
    }

    return 1;
}

int set_task_delay(TaskHandle_t p,uint32_t delay)
{
	if(p==NULL)
		{
			p=(void *)currentTask;
		}
		TaskControlBlock *task=(TaskControlBlock *) p;
		if(task!=NULL)
		{
			task->delay=delay;
			return 1;
		}
		return 0;
}

/*
该任务用于从运行态进入delay态，默认是当前任务
*/
void Tsakdelay(uint32_t delay)
{
//	TaskControlBlock *current = currentTask;
		set_task_delay(NULL,delay);
		removeTaskpfroma(&taskLists[currentTask->priority],NULL);
		AddTaskptob(NULL,&delayList,1);
	
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
					removeTaskpfroma(&delayList,(void *)current);
					AddTaskptob((void *)current,&taskLists[current->priority],0);
			 }
//			 
			 // 继续遍历
        prev = current;
        current = current->next;
			 
	 }
	
}

int createTask(void (*taskFunc)(void *), void *param, int priority, int yield, int stackSize,TaskHandle_t * pxCreatedTask ) {
    if (taskCount < MAX_TASKS) {
        // 动态分配 TaskControlBlock
        TaskControlBlock *tcb = (TaskControlBlock *)malloc(sizeof(TaskControlBlock));
        if (tcb == NULL) {
            printf("Memory allocation for TaskControlBlock failed\r\n");
            return 0;  // 内存分配失败
        }

        // 动态分配栈空间
        tcb->stack = (uint32_t *)malloc(stackSize * sizeof(uint32_t));
        if (tcb->stack == NULL) {
            printf("Memory allocation for stack failed\r\n");
            free(tcb);  // 释放已分配的 TaskControlBlock
            return 0;  // 内存分配失败
        }
        // 指向任务栈的顶端
        uint32_t *stack = &(tcb->stack[stackSize]);

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

        // 更新TCB的堆栈指针和其他参数
        tcb->stackPointer = stack;
        tcb->stackSize = stackSize;  // 设置栈大小
        tcb->priority = priority;
        tcb->yield = yield;
				tcb->delay=0;
        tcb->next = NULL;
				taskCount++;

        // 将任务添加到任务链表
        addTaskToList(tcb);
				
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
//      // 遍历到队列末尾

	 if (firstTask->next != NULL) {
      while (lastTask->next != NULL) {
      lastTask = lastTask->next;
      }
      // 将当前任务移到末尾，并处理链表指针
      lastTask->next = firstTask;
      taskLists[i] = firstTask->next;  // 队列头移到下一个任务
      firstTask->next = NULL;          // 当前任务的 next 置空	
			
    }
	return taskLists[i]->yield;
}


void schedule(void) {
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
    }

    // 如果没有其他任务可运行，切换到空闲任务
//    nextTask = idleTask;
}



void os_schedule_start(void)
{
//	schedule(1);
	PendSV_init();
}



