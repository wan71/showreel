#include <thread.h>
#include <string.h>
#include "config.h"
#include "stm32f1xx_hal.h"
#include "cpu.h"
#include "heap_1.h"
TaskControlBlock *currentTask = NULL;  // 当前运行任务
TaskControlBlock *nextTask = NULL;     // 下一个任务
TaskControlBlock *monitorList= NULL;  // 监控链表
	



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

int removeTaskpfroma(TaskHandle_t a, TaskHandle_t p) {
    if (p == NULL) {
			 // 检查 currentTask 是否有效
        if (currentTask == NULL) {
            printf("Error: currentTask is NULL!\r\n");
            return -1;  // 提前返回以避免死机
				}
        p = (void *)currentTask;
    }

    TaskControlBlock *task = (TaskControlBlock *)p;
    TaskControlBlock **listHead = (TaskControlBlock **)a;
    TaskControlBlock *current = *listHead;
    TaskControlBlock *prev = NULL;
    int removeCount = 0;  // 记录删除的任务数量

    // 如果链表为空，直接返回
    if (current == NULL) {
        return 0;
    }

    // 遍历链表，移除所有与目标任务相同的节点
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

            // 移除任务，清空 next 指针
            TaskControlBlock *taskToRemove = current;
            current = current->next;
            taskToRemove->next = NULL;

            removeCount++;  // 增加移除计数
        } else {
            // 当前任务与目标任务不匹配，继续遍历
            prev = current;
            current = current->next;
        }
    }

    return removeCount;  // 返回删除的任务数量
}

/*
该任务用于将tcb加入到TaskHandle_t b链表中，如果a不等于0，就会使用triggerPendSV();
*/
int AddTaskptob(TaskHandle_t p, TaskHandle_t b) 
{
    if (p == NULL) {
        p = (void *)currentTask;
    }

    TaskControlBlock *task = (TaskControlBlock *)p;
    TaskControlBlock **listHead = (TaskControlBlock **)b;

    // 检查链表中是否已经存在该任务
    TaskControlBlock *current = *listHead;
    while (current != NULL) {
        if (current == task) {
            // 如果任务已经在链表中，直接返回，不再添加
            return 0;
        }
        current = current->next;
    }

    // 如果目标链表为空，直接将任务放入链表头
    if (*listHead == NULL) {
        *listHead = task;
    } else {
        // 遍历链表并将任务添加到末尾
        current = *listHead;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = task;
    }

    task->next = NULL;  // 确保新任务是链表的最后一个节点

    return 1;  // 返回 1 表示成功添加任务
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
		enterCritical();
		set_task_delay(NULL,delay);
		removeTaskpfroma(&taskLists[currentTask->priority],NULL);
		AddTaskptob(NULL,&delayList);
		exitCritical();
		restart_pendsv();
}

/*每次时钟中断时，遍历 delayList，减少延时时间，如果延时结束，则将任务移回对应优先级的 taskLists*/

void every_delay(void) {
	
		enterCritical();
    TaskControlBlock *prev = NULL;
    TaskControlBlock *current = delayList;
	 while (current != NULL) {
		  if (current->delay > 0) {
            current->delay--;  // 减少延时时间
        }
		 if (current->delay == 0) {
				  // 将任务加入到对应优先级的 taskLists
					
					removeTaskpfroma(&delayList,(void *)current);
					AddTaskptob((void *)current,&taskLists[current->priority]);
			 }
//			 
			 // 继续遍历
        prev = current;
        current = current->next;
			 
	 }
	exitCritical();
}

int createTask(void (*taskFunc)(void *),const char * const pcName, void *param, int priority, int yield, int stackSize, TaskHandle_t *pxCreatedTask) {
    if (taskCount < MAX_TASKS) {
        // 动态分配 TaskControlBlock
		
//        TaskControlBlock *tcb = (TaskControlBlock *)pvPortMalloc(sizeof(TaskControlBlock));
				TaskControlBlock *tcb = (TaskControlBlock *)malloc(sizeof(TaskControlBlock));
        if (tcb == NULL) {
            printf("Memory allocation for TaskControlBlock failed\r\n");
            return 0;  // 内存分配失败
        }

        // 动态分配栈空间
//				tcb->stack = (uint32_t *)malloc(stackSize * sizeof(uint32_t));
        tcb->stack = (uint32_t *)pvPortMalloc(stackSize * sizeof(uint32_t));
        if (tcb->stack == NULL) {
            printf("Memory allocation for stack failed\r\n");
            vPortFree(tcb->stack);  // 释放已分配的 TaskControlBlock
            return 0;  // 内存分配失败
        }

#if(STACK_Control)
			{
				initializeTaskStack(tcb);
			}
#endif

        // 指向任务栈的顶端（注意：数组索引从0开始，因此应减1）
        uint32_t *stack = tcb->stack + stackSize;

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
        tcb->delay = 0;
        tcb->next = NULL;
				/*
				添加函数名字
				*/
				for( int x = ( UBaseType_t ) 0; x < ( UBaseType_t ) configMAX_TASK_NAME_LEN; x++ )
				{
					tcb->pcTaskName[ x ] = pcName[ x ];

					/* Don't copy all configMAX_TASK_NAME_LEN if the string is shorter than
					configMAX_TASK_NAME_LEN characters just in case the memory after the
					string is not accessible (extremely unlikely). */
					if( pcName[ x ] == 0x00 )
					{
						break;
					}
				}
				tcb->pcTaskName[ configMAX_TASK_NAME_LEN - 1 ] = '\0';
				
				
			
			// 将任务加入监控链表
					tcb->nextInMonitorList = monitorList;
					monitorList = tcb;
	#if( TASK_CPU )
					// 初始化任务的运行时间和其他参数
					tcb->totalRunTime = 0;
					tcb->lastStartTime = 0;
			#endif 
				
        taskCount++;

        // 将任务添加到任务链表
        addTaskToList(tcb);

        if (pxCreatedTask != NULL) {
            *pxCreatedTask = (TaskHandle_t)tcb;
        }

        return 1;
    }
    return 0;  // 如果任务数已满，返回 0
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

void taskSwitch(TaskControlBlock *oldTask, TaskControlBlock *newTask) {
    uint32_t currentTime = getSysTime_task();  // 获取系统当前时间
	
    // 更新旧任务的运行时间
	  if (oldTask != NULL) {
    oldTask->totalRunTime += (currentTime - oldTask->lastStartTime);// 记录新任务的启动时间
    newTask->lastStartTime = currentTime;
		}
		else
		{
			 newTask->lastStartTime = currentTime;
		}
   
	
	return;
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
            }
					}

            // 默认下个任务，是优先级任务队列的头部
            nextTask = taskLists[i];
							#if( TASK_CPU)
									taskSwitch(currentTask,nextTask);
							#endif

						
            return;
        }
    }
	
    // 如果没有其他任务可运行，切换到空闲任务
//    nextTask = idleTask;
}



static void idle(void *param)
{
	/*自定义*/
	for(;;){
//		HAL_SuspendTick();	//停止系统滴答计时器
	//	HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON,PWR_SLEEPENTRY_WFI);  //进入睡眠模式
		HAL_Delay(1);
//		HAL_ResumeTick();        //释放滴答定时器
//		printf("this is idle\r\n");
	
	}
	
};



void os_schedule_start(void)
{
	createTask(idle,"idle",NULL,0,1,32,NULL);//创建空闲函数
	PendSV_init();
}



