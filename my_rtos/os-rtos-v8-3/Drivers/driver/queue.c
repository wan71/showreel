#include "queue.h"
#include "stm32f1xx_hal.h"
#include "cpu.h"
extern TaskControlBlock *currentTask;
extern TaskControlBlock *nextTask;
extern TaskControlBlock *taskLists[MAX_PRIORITY]; 
extern TaskControlBlock *delayList;
extern TaskHandle_t task_1;
extern TaskHandle_t task_2;
extern TaskHandle_t task_3;

Queue_t *xQueueCreate(UBaseType_t uxQueueLength, UBaseType_t uxItemSize) {
    UBaseType_t xQueueSizeInBytes = uxQueueLength * uxItemSize;
    Queue_t *pxNewQueue = (Queue_t *)malloc(sizeof(Queue_t) + xQueueSizeInBytes);

    if (pxNewQueue == NULL) {
        return NULL;  // 内存分配失败
    }

    // 初始化队列指针
    pxNewQueue->pcHead = (int8_t *)(pxNewQueue + 1); // 队列存储空间位于结构体之后
    pxNewQueue->pcTail = pxNewQueue->pcHead + xQueueSizeInBytes;
    pxNewQueue->pcWriteTo = pxNewQueue->pcHead;
    pxNewQueue->pcReadFrom = pxNewQueue->pcHead;

    // 初始化其他参数
    pxNewQueue->uxLength = uxQueueLength;
    pxNewQueue->uxItemSize = uxItemSize;
    pxNewQueue->uxMessagesWaiting = 0;
    pxNewQueue->cRxLock = 0;
    pxNewQueue->cTxLock = 0;

    pxNewQueue->xTasksWaitingToSend = NULL;
    pxNewQueue->xTasksWaitingToReceive = NULL;

    return pxNewQueue;
}

/*入列函数用于将数据写入队列。*/
BaseType_t xQueueSend(Queue_t *pxQueue, const void *pvItemToQueue) {
    if (pxQueue->uxMessagesWaiting == pxQueue->uxLength) {
        // 队列已满
        return pdFAIL;
    }
	

    // 将数据拷贝到队列
    memcpy(pxQueue->pcWriteTo, pvItemToQueue, pxQueue->uxItemSize);
    
    // 更新写指针，循环队列
    pxQueue->pcWriteTo += pxQueue->uxItemSize;
    if (pxQueue->pcWriteTo >= pxQueue->pcTail) {
        pxQueue->pcWriteTo = pxQueue->pcHead;
    }

    pxQueue->uxMessagesWaiting++;  // 队列中消息数量+1
	
		
    return pdPASS;
}


/*出列函数用于从队列中读取数据*/
BaseType_t xQueueReceive(Queue_t *pxQueue, void *pvBuffer) {
    if (pxQueue->uxMessagesWaiting == 0) {
        // 队列为空
        return pdFAIL;
    }

    // 将数据从队列中读取到缓冲区
    memcpy(pvBuffer, pxQueue->pcReadFrom, pxQueue->uxItemSize);

    // 更新读指针，循环队列
    pxQueue->pcReadFrom += pxQueue->uxItemSize;
    if (pxQueue->pcReadFrom >= pxQueue->pcTail) {
        pxQueue->pcReadFrom = pxQueue->pcHead;
    }

    pxQueue->uxMessagesWaiting--;  // 队列中消息数量-1
    return pdPASS;
}

/*该函数用于打印队列中的元素，方便调试*/
void vQueuePrint(Queue_t *pxQueue) {
    printf("Queue Length: %d\r\n", pxQueue->uxLength);
    printf("Messages Waiting: %d\r\n", pxQueue->uxMessagesWaiting);

    int8_t *pTempReadFrom = pxQueue->pcReadFrom;
    for (UBaseType_t i = 0; i < pxQueue->uxMessagesWaiting; i++) {
        printf("Item %d: %d\r\n", i, *((int *)pTempReadFrom));

        // 循环更新读指针
        pTempReadFrom += pxQueue->uxItemSize;
        if (pTempReadFrom >= pxQueue->pcTail) {
            pTempReadFrom = pxQueue->pcHead;
        }
    }
}

BaseType_t xQueueGenericSend(Queue_t *pxQueue, const void * const pvItemToQueue, TickType_t delay)
{
	BaseType_t xReturn = pdPASS;
				// 设置延时等待
	set_task_delay(NULL, delay);
	while(1)
	{
		// 进入临界区，确保队列操作的原子性
		enterCritical();

		// 尝试向队列发送消息
		xReturn = xQueueSend(pxQueue, pvItemToQueue);

		if (xReturn == pdFAIL)  // 队列已满
		{
			
		 if ((currentTask->delay) == 0)
			{			
				// 先检查 xTasksWaitingToSend 是否为空，避免潜在错误
				if (pxQueue->xTasksWaitingToSend != NULL)
				{
				removeTaskpfroma(&(pxQueue->xTasksWaitingToSend), (TaskHandle_t)&currentTask);				
				}
				// 如果不需要延时直接退出
				exitCritical();
				return xReturn;
			}
			else if ((currentTask->delay) > 0)
			{
				
				
				// 将当前任务从就绪队列移除，加入延迟队列
				removeTaskpfroma(&taskLists[currentTask->priority], NULL);
				AddTaskptob(NULL, &(pxQueue->xTasksWaitingToSend));
				AddTaskptob(NULL, &delayList);  // 加入延迟队列

				// 退出临界区并触发任务切换
				exitCritical();
				restart_pendsv();
			}
	
		}
		else
		{
			set_task_delay(NULL, 0);
			// 成功发送消息后，检查是否有任务等待接收
			if (pxQueue->xTasksWaitingToReceive != NULL)
			{
				// 唤醒等待接收的任务
				TaskControlBlock *xTaskToUnblock = pxQueue->xTasksWaitingToReceive;

				// 从接收等待队列移除任务，并加入就绪队列
				removeTaskpfroma(&(pxQueue->xTasksWaitingToReceive), xTaskToUnblock);
				removeTaskpfroma(&delayList, xTaskToUnblock);
				AddTaskptob(xTaskToUnblock, &taskLists[xTaskToUnblock->priority]);

				// 退出临界区并触发任务切换
				exitCritical();
				restart_pendsv();
				return xReturn;
			}

			// 退出临界区，消息已发送成功，退出循环
			exitCritical();
			return xReturn;
		}
	}
}


BaseType_t xQueueGenericReceive(Queue_t *pxQueue, void * const pvItemToQueue, TickType_t delay)
{
	BaseType_t xReturn = pdPASS;
	// 设置延时等待
	set_task_delay(NULL, delay);
	while(1)
	{
		// 进入临界区，确保队列操作的原子性
		enterCritical();

		// 尝试从队列中接收消息
		xReturn = xQueueReceive(pxQueue, pvItemToQueue);

		if (xReturn == pdFAIL)  // 队列为空
		{
			if ((currentTask->delay) > 0)
			{


				// 将当前任务从就绪队列移除，加入延迟队列
				removeTaskpfroma(&taskLists[currentTask->priority], NULL);
				AddTaskptob(NULL, &(pxQueue->xTasksWaitingToReceive));
				AddTaskptob(NULL, &delayList);

				// 退出临界区并触发任务切换
				exitCritical();
				restart_pendsv();
			}
			if ((currentTask->delay) == 0)
			{

					if (pxQueue->xTasksWaitingToReceive != NULL)
				{
//								removeTaskpfroma(&(pxQueue->xTasksWaitingToReceive),&task_3);		
					removeTaskpfroma(&(pxQueue->xTasksWaitingToReceive),(TaskHandle_t)&currentTask);		
				}

				// 如果不需要延时直接退出
				exitCritical();
				return xReturn;
			}
		
		}
		else
		{
				set_task_delay(NULL, 0);
			// 成功接收到消息后，检查是否有任务等待发送
			if (pxQueue->xTasksWaitingToSend != NULL)
			{
				// 唤醒等待发送的任务
				TaskControlBlock *xTaskToUnblock = pxQueue->xTasksWaitingToSend;

				// 从发送等待队列移除任务，并加入就绪队列
				removeTaskpfroma(&(pxQueue->xTasksWaitingToSend), xTaskToUnblock);
				removeTaskpfroma(&delayList, xTaskToUnblock);
				AddTaskptob(xTaskToUnblock, &taskLists[xTaskToUnblock->priority]);

				// 退出临界区并触发任务切换
				exitCritical();
				restart_pendsv();
				return xReturn;
			}

			// 退出临界区，消息已成功接收，退出循环
			exitCritical();
			return xReturn;
		}
	}
}


