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
		// 进入临界区
    enterCritical();

    // 将数据拷贝到队列
    memcpy(pxQueue->pcWriteTo, pvItemToQueue, pxQueue->uxItemSize);
    
    // 更新写指针，循环队列
    pxQueue->pcWriteTo += pxQueue->uxItemSize;
    if (pxQueue->pcWriteTo >= pxQueue->pcTail) {
        pxQueue->pcWriteTo = pxQueue->pcHead;
    }

    pxQueue->uxMessagesWaiting++;  // 队列中消息数量+1
		exitCritical(); 
		
    return pdPASS;
}


/*出列函数用于从队列中读取数据*/
BaseType_t xQueueReceive(Queue_t *pxQueue, void *pvBuffer) {
    if (pxQueue->uxMessagesWaiting == 0) {
        // 队列为空
        return pdFAIL;
    }
	// 进入临界区
    enterCritical();
    // 将数据从队列中读取到缓冲区
    memcpy(pvBuffer, pxQueue->pcReadFrom, pxQueue->uxItemSize);

    // 更新读指针，循环队列
    pxQueue->pcReadFrom += pxQueue->uxItemSize;
    if (pxQueue->pcReadFrom >= pxQueue->pcTail) {
        pxQueue->pcReadFrom = pxQueue->pcHead;
    }

    pxQueue->uxMessagesWaiting--;  // 队列中消息数量-1
		exitCritical(); 
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


//BaseType_t xQueueGenericSend(Queue_t *pxQueue, const void * const pvItemToQueue, TickType_t delay)
//{
//		BaseType_t xReturn = pdPASS;
//    BaseType_t xTaskSwitchRequired = pdFALSE;
//		
//		
//	 // 检查队列是否已满
//   xReturn=xQueueSend(pxQueue,pvItemToQueue);
//	if(xReturn == pdFAIL)  //写入失败，已经满了
//	{
//		if(delay==0)
//		{
//				//重置SystemClock
//				HAL_SYSTICK_Config(SystemCoreClock / (1000U / uwTickFreq));
//				  // 退出临界区
//				triggerPendSV();
//		}
//		else{
//			
//			
//			set_task_delay(NULL,delay);
//			removeTaskpfroma(&taskLists[currentTask->priority],NULL);
//			AddTaskptob(NULL,&(pxQueue->xTasksWaitingToSend),0);			
//			AddTaskptob(NULL,&delayList,1); //加入到延迟函数
//		}
//	
//		return xReturn;
//	}
//		if(xReturn == pdPASS)  //读出成功，唤醒写程序
//	{
//		// 检查是否有其他任务在等待发送
//		if(pxQueue->xTasksWaitingToReceive != NULL)
//		{
//			// 将等待发送的任务唤醒
//			TaskControlBlock *xTaskToUnblock = pxQueue->xTasksWaitingToReceive;
//			// 将任务从发送等待队列中移除，并将其加入到就绪队列中
//       removeTaskpfroma(&(pxQueue->xTasksWaitingToReceive), xTaskToUnblock);
//				removeTaskpfroma(&delayList,xTaskToUnblock);
//       AddTaskptob(xTaskToUnblock,&taskLists[xTaskToUnblock->priority], 1); // 加入到就绪队列并触发任务切换
//		}
//		
//		
//	}
//	

//	return xReturn;
//}
BaseType_t xQueueGenericSend(Queue_t *pxQueue, const void * const pvItemToQueue, TickType_t delay)
{
	BaseType_t xReturn = pdPASS;
    
	// 检查队列是否已满
	xReturn = xQueueSend(pxQueue, pvItemToQueue);
	if(xReturn == pdFAIL)  // 队列满
	{
		if(delay == 0)
		{
			// 立即返回失败
//			triggerPendSV();
			return xReturn;
		}
		else
		{
			// 设置延时
			set_task_delay(NULL, delay);
			
			// 将当前任务从就绪队列中移除
			removeTaskpfroma(&taskLists[currentTask->priority], NULL);
			
			// 将当前任务添加到等待发送队列和延迟队列
			AddTaskptob(NULL, &(pxQueue->xTasksWaitingToSend), 0);	
			AddTaskptob(NULL, &delayList, 1); // 加入延迟队列，触发任务切换
		}
		return xReturn;
	}

	// 成功发送后，检查是否有任务在等待接收消息
	if(pxQueue->xTasksWaitingToReceive != NULL)
	{
		// 唤醒等待接收的任务
		TaskControlBlock *xTaskToUnblock = pxQueue->xTasksWaitingToReceive;

		// 从等待接收队列中移除，并加入就绪队列
		removeTaskpfroma(&(pxQueue->xTasksWaitingToReceive), xTaskToUnblock);
		removeTaskpfroma(&delayList, xTaskToUnblock);  // 确保从延迟队列中移除
		AddTaskptob(xTaskToUnblock, &taskLists[xTaskToUnblock->priority], 1);  // 加入到就绪队列并触发任务切换
	}

	return xReturn;
}



/*
在接收任务时，需要检查队列是否为空，如果为空则将任务挂起，直到有新消息到来。同时，处理超时机制、任务切换、唤醒阻塞的发送任务等功能。
*/
BaseType_t xQueueGenericReceive(Queue_t *pxQueue, void * const pvItemToQueue, TickType_t delay)
{
	BaseType_t xReturn = pdPASS;
  
	// 检查队列是否为空
	xReturn = xQueueReceive(pxQueue, pvItemToQueue);
	if(xReturn == pdFAIL)  // 队列空
	{
		if(delay == 0)
		{
			// 立即返回失败
			triggerPendSV();
			return xReturn;
		}
		else
		{
			// 设置延时
			set_task_delay(NULL, delay);
			
			// 将当前任务从就绪队列中移除
			removeTaskpfroma(&taskLists[currentTask->priority], NULL);

			// 将当前任务添加到等待接收队列和延迟队列
			AddTaskptob(NULL, &(pxQueue->xTasksWaitingToReceive), 0);	
			AddTaskptob(NULL, &delayList, 1);  // 加入延迟队列，触发任务切换
		}
		
		return xReturn;
	}

	// 成功接收后，检查是否有任务在等待发送消息
	if(pxQueue->xTasksWaitingToSend != NULL)
	{
		// 唤醒等待发送的任务
		TaskControlBlock *xTaskToUnblock = pxQueue->xTasksWaitingToSend;

		// 从等待发送队列中移除，并加入就绪队列
		removeTaskpfroma(&(pxQueue->xTasksWaitingToSend), xTaskToUnblock);
		removeTaskpfroma(&delayList, xTaskToUnblock);  // 确保从延迟队列中移除
		AddTaskptob(xTaskToUnblock, &taskLists[xTaskToUnblock->priority], 1);  // 加入到就绪队列并触发任务切换
	}

	return xReturn;
}



//BaseType_t xQueueGenericReceive(Queue_t *pxQueue, void * const pvItemToQueue, TickType_t delay)
//{
//	BaseType_t xReturn = pdPASS;
//  BaseType_t xTaskSwitchRequired = pdFALSE;

//    
//	// 检查队列是否有值可读
//   xReturn=xQueueReceive(pxQueue,pvItemToQueue);
//	if(xReturn == pdFAIL)  //读出失败，已经空了
//	{
//		if(delay==0)
//		{
//				//重置SystemClock
//				HAL_SYSTICK_Config(SystemCoreClock / (1000U / uwTickFreq));
//				triggerPendSV();  //Pensv异常
//		}
//		else
//		{
//			set_task_delay(NULL,delay);
//				removeTaskpfroma(&taskLists[currentTask->priority],NULL);
//				AddTaskptob(NULL,&(pxQueue->xTasksWaitingToReceive),0);	
//				AddTaskptob(NULL,&delayList,1); //加入到延迟函数
//		}
//	
//	}
//	if(xReturn == pdPASS)  //读出成功，唤醒写程序
//	{
//		// 检查是否有其他任务在等待发送
//		if(pxQueue->xTasksWaitingToSend != NULL)
//		{
//			// 将等待发送的任务唤醒
//			TaskControlBlock *xTaskToUnblock = pxQueue->xTasksWaitingToSend;
//			// 将任务从发送等待队列中移除，并将其加入到就绪队列中
//       removeTaskpfroma(&(pxQueue->xTasksWaitingToSend), xTaskToUnblock);
//				removeTaskpfroma(&delayList,xTaskToUnblock);
////				AddTaskptob(task_2,&taskLists,1);
//       AddTaskptob(xTaskToUnblock,&taskLists[xTaskToUnblock->priority], 1); // 加入到就绪队列并触发任务切换
//		}
//		
//		
//		
//	}
//	
//	return xReturn;
//}
