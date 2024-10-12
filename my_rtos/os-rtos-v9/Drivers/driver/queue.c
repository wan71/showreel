#include "queue.h"
#include "stm32f1xx_hal.h"
#include "cpu.h"
//#include "heap_1.c"
extern TaskControlBlock *currentTask;
extern TaskControlBlock *nextTask;
extern TaskControlBlock *taskLists[MAX_PRIORITY]; 
extern TaskControlBlock *delayList;
extern TaskHandle_t task_1;
extern TaskHandle_t task_2;
extern TaskHandle_t task_3;

Queue_t *xQueueCreate(UBaseType_t uxQueueLength, UBaseType_t uxItemSize) {
    UBaseType_t xQueueSizeInBytes = uxQueueLength * uxItemSize;
    Queue_t *pxNewQueue = (Queue_t *)pvPortMalloc(sizeof(Queue_t) + xQueueSizeInBytes);

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
	
//		enterCritical();
    // 将数据拷贝到队列
    memcpy(pxQueue->pcWriteTo, pvItemToQueue, pxQueue->uxItemSize);
    
    // 更新写指针，循环队列
    pxQueue->pcWriteTo += pxQueue->uxItemSize;
    if (pxQueue->pcWriteTo >= pxQueue->pcTail) {
        pxQueue->pcWriteTo = pxQueue->pcHead;
    }

    pxQueue->uxMessagesWaiting++;  // 队列中消息数量+1
	
		// 退出临界区
//    exitCritical();
    return pdPASS;
}


/*出列函数用于从队列中读取数据*/
BaseType_t xQueueReceive(Queue_t *pxQueue, void *pvBuffer) {
    if (pxQueue->uxMessagesWaiting == 0) {
        // 队列为空
        return pdFAIL;
    }
//	enterCritical();
    // 将数据拷贝到队列
    // 将数据从队列中读取到缓冲区
    memcpy(pvBuffer, pxQueue->pcReadFrom, pxQueue->uxItemSize);

    // 更新读指针，循环队列
    pxQueue->pcReadFrom += pxQueue->uxItemSize;
    if (pxQueue->pcReadFrom >= pxQueue->pcTail) {
        pxQueue->pcReadFrom = pxQueue->pcHead;
    }

    pxQueue->uxMessagesWaiting--;  // 队列中消息数量-1
//		 exitCritical();
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

BaseType_t xQueueGenericSend(Queue_t *pxQueue, const void * const pvItemToQueue, TickType_t delay) {
    BaseType_t xReturn = pdPASS;

    set_task_delay(NULL, delay);  // 设置延迟时间

    while (1) {
        enterCritical();  // 进入临界区

        // 尝试向队列发送消息
        xReturn = xQueueSend(pxQueue, pvItemToQueue);

        if (xReturn == pdFAIL) {  // 队列满
            if (currentTask->delay == 0) {  // 没有延时，立即退出
                exitCritical();
                return xReturn;
            } else {
                // 将任务从就绪队列移除并加入延迟队列
                removeTaskpfroma(&taskLists[currentTask->priority], NULL);
                AddTaskptob(NULL, &(pxQueue->xTasksWaitingToSend));
                AddTaskptob(NULL, &delayList);  // 加入延迟队列

                exitCritical();  // 退出临界区并触发任务切换
                restart_pendsv();
            }
        } else if (xReturn == pdPASS) {  // 消息发送成功
            // 检查是否有任务等待接收
            if (pxQueue->xTasksWaitingToReceive != NULL) {
                TaskControlBlock *xTaskToUnblock = pxQueue->xTasksWaitingToReceive;

                // 从等待队列移除任务，并加入就绪队列
                removeTaskpfroma(&(pxQueue->xTasksWaitingToReceive), xTaskToUnblock);
                removeTaskpfroma(&delayList, xTaskToUnblock);
                AddTaskptob(xTaskToUnblock, &taskLists[xTaskToUnblock->priority]);
            }

            exitCritical();  // 退出临界区
            return xReturn;
        }
    }
}

BaseType_t xQueueGenericReceive(Queue_t *pxQueue, void * const pvItemToQueue, TickType_t delay) {
    BaseType_t xReturn = pdPASS;

    // Set delay for current task
    set_task_delay(NULL, delay);

    while (1) {
        // Enter critical section for queue operation
        enterCritical();

        // Try to receive message from queue
        xReturn = xQueueReceive(pxQueue, pvItemToQueue);

        if (xReturn == pdFAIL) {  // Queue is empty
            if (currentTask->delay > 0) {
                // Remove current task from ready list and add to delay/waiting queue
                removeTaskpfroma(&taskLists[currentTask->priority], NULL);
                AddTaskptob(NULL, &(pxQueue->xTasksWaitingToReceive));
                AddTaskptob(NULL, &delayList);

                // Exit critical section and trigger task switch
                exitCritical();
                restart_pendsv();
            } else {
                // Exit critical section and return failure
                exitCritical();
                return xReturn;
            }
        }

        if (xReturn == pdPASS) {
            // Message received successfully
            // Check if there are any tasks waiting to send
            if (pxQueue->xTasksWaitingToSend != NULL) {
                TaskControlBlock *xTaskToUnblock = pxQueue->xTasksWaitingToSend;

                // Remove task from send-waiting queue and add to ready list
                removeTaskpfroma(&(pxQueue->xTasksWaitingToSend), xTaskToUnblock);
                removeTaskpfroma(&delayList, xTaskToUnblock);
                AddTaskptob(xTaskToUnblock, &taskLists[xTaskToUnblock->priority]);
            }

            // Exit critical section and return success
            exitCritical();
            return xReturn;
        }
    }
}





