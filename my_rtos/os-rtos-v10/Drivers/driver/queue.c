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
        return NULL;  // �ڴ����ʧ��
    }

    // ��ʼ������ָ��
    pxNewQueue->pcHead = (int8_t *)(pxNewQueue + 1); // ���д洢�ռ�λ�ڽṹ��֮��
    pxNewQueue->pcTail = pxNewQueue->pcHead + xQueueSizeInBytes;
    pxNewQueue->pcWriteTo = pxNewQueue->pcHead;
    pxNewQueue->pcReadFrom = pxNewQueue->pcHead;

    // ��ʼ����������
    pxNewQueue->uxLength = uxQueueLength;
    pxNewQueue->uxItemSize = uxItemSize;
    pxNewQueue->uxMessagesWaiting = 0;
    pxNewQueue->cRxLock = 0;
    pxNewQueue->cTxLock = 0;

    pxNewQueue->xTasksWaitingToSend = NULL;
    pxNewQueue->xTasksWaitingToReceive = NULL;

    return pxNewQueue;
}

/*���к������ڽ�����д����С�*/
BaseType_t xQueueSend(Queue_t *pxQueue, const void *pvItemToQueue) {
    if (pxQueue->uxMessagesWaiting == pxQueue->uxLength) {
        // ��������
        return pdFAIL;
    }
	
//		enterCritical();
    // �����ݿ���������
    memcpy(pxQueue->pcWriteTo, pvItemToQueue, pxQueue->uxItemSize);
    
    // ����дָ�룬ѭ������
    pxQueue->pcWriteTo += pxQueue->uxItemSize;
    if (pxQueue->pcWriteTo >= pxQueue->pcTail) {
        pxQueue->pcWriteTo = pxQueue->pcHead;
    }

    pxQueue->uxMessagesWaiting++;  // ��������Ϣ����+1
	
		// �˳��ٽ���
//    exitCritical();
    return pdPASS;
}


/*���к������ڴӶ����ж�ȡ����*/
BaseType_t xQueueReceive(Queue_t *pxQueue, void *pvBuffer) {
    if (pxQueue->uxMessagesWaiting == 0) {
        // ����Ϊ��
        return pdFAIL;
    }
//	enterCritical();
    // �����ݿ���������
    // �����ݴӶ����ж�ȡ��������
    memcpy(pvBuffer, pxQueue->pcReadFrom, pxQueue->uxItemSize);

    // ���¶�ָ�룬ѭ������
    pxQueue->pcReadFrom += pxQueue->uxItemSize;
    if (pxQueue->pcReadFrom >= pxQueue->pcTail) {
        pxQueue->pcReadFrom = pxQueue->pcHead;
    }

    pxQueue->uxMessagesWaiting--;  // ��������Ϣ����-1
//		 exitCritical();
    return pdPASS;
}

/*�ú������ڴ�ӡ�����е�Ԫ�أ��������*/
void vQueuePrint(Queue_t *pxQueue) {
    printf("Queue Length: %d\r\n", pxQueue->uxLength);
    printf("Messages Waiting: %d\r\n", pxQueue->uxMessagesWaiting);

    int8_t *pTempReadFrom = pxQueue->pcReadFrom;
    for (UBaseType_t i = 0; i < pxQueue->uxMessagesWaiting; i++) {
        printf("Item %d: %d\r\n", i, *((int *)pTempReadFrom));

        // ѭ�����¶�ָ��
        pTempReadFrom += pxQueue->uxItemSize;
        if (pTempReadFrom >= pxQueue->pcTail) {
            pTempReadFrom = pxQueue->pcHead;
        }
    }
}

BaseType_t xQueueGenericSend(Queue_t *pxQueue, const void * const pvItemToQueue, TickType_t delay) {
    BaseType_t xReturn = pdPASS;

    set_task_delay(NULL, delay);  // �����ӳ�ʱ��

    while (1) {
        enterCritical();  // �����ٽ���

        // ��������з�����Ϣ
        xReturn = xQueueSend(pxQueue, pvItemToQueue);

        if (xReturn == pdFAIL) {  // ������
            if (currentTask->delay == 0) {  // û����ʱ�������˳�
                exitCritical();
                return xReturn;
            } else {
                // ������Ӿ��������Ƴ��������ӳٶ���
                removeTaskpfroma(&taskLists[currentTask->priority], NULL);
                AddTaskptob(NULL, &(pxQueue->xTasksWaitingToSend));
                AddTaskptob(NULL, &delayList);  // �����ӳٶ���

                exitCritical();  // �˳��ٽ��������������л�
                restart_pendsv();
            }
        } else if (xReturn == pdPASS) {  // ��Ϣ���ͳɹ�
            // ����Ƿ�������ȴ�����
            if (pxQueue->xTasksWaitingToReceive != NULL) {
                TaskControlBlock *xTaskToUnblock = pxQueue->xTasksWaitingToReceive;

                // �ӵȴ������Ƴ����񣬲������������
                removeTaskpfroma(&(pxQueue->xTasksWaitingToReceive), xTaskToUnblock);
                removeTaskpfroma(&delayList, xTaskToUnblock);
                AddTaskptob(xTaskToUnblock, &taskLists[xTaskToUnblock->priority]);
            }

            exitCritical();  // �˳��ٽ���
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





