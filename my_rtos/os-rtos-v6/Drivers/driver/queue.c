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
		// �����ٽ���
    enterCritical();

    // �����ݿ���������
    memcpy(pxQueue->pcWriteTo, pvItemToQueue, pxQueue->uxItemSize);
    
    // ����дָ�룬ѭ������
    pxQueue->pcWriteTo += pxQueue->uxItemSize;
    if (pxQueue->pcWriteTo >= pxQueue->pcTail) {
        pxQueue->pcWriteTo = pxQueue->pcHead;
    }

    pxQueue->uxMessagesWaiting++;  // ��������Ϣ����+1
		exitCritical(); 
		
    return pdPASS;
}


/*���к������ڴӶ����ж�ȡ����*/
BaseType_t xQueueReceive(Queue_t *pxQueue, void *pvBuffer) {
    if (pxQueue->uxMessagesWaiting == 0) {
        // ����Ϊ��
        return pdFAIL;
    }
	// �����ٽ���
    enterCritical();
    // �����ݴӶ����ж�ȡ��������
    memcpy(pvBuffer, pxQueue->pcReadFrom, pxQueue->uxItemSize);

    // ���¶�ָ�룬ѭ������
    pxQueue->pcReadFrom += pxQueue->uxItemSize;
    if (pxQueue->pcReadFrom >= pxQueue->pcTail) {
        pxQueue->pcReadFrom = pxQueue->pcHead;
    }

    pxQueue->uxMessagesWaiting--;  // ��������Ϣ����-1
		exitCritical(); 
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


//BaseType_t xQueueGenericSend(Queue_t *pxQueue, const void * const pvItemToQueue, TickType_t delay)
//{
//		BaseType_t xReturn = pdPASS;
//    BaseType_t xTaskSwitchRequired = pdFALSE;
//		
//		
//	 // �������Ƿ�����
//   xReturn=xQueueSend(pxQueue,pvItemToQueue);
//	if(xReturn == pdFAIL)  //д��ʧ�ܣ��Ѿ�����
//	{
//		if(delay==0)
//		{
//				//����SystemClock
//				HAL_SYSTICK_Config(SystemCoreClock / (1000U / uwTickFreq));
//				  // �˳��ٽ���
//				triggerPendSV();
//		}
//		else{
//			
//			
//			set_task_delay(NULL,delay);
//			removeTaskpfroma(&taskLists[currentTask->priority],NULL);
//			AddTaskptob(NULL,&(pxQueue->xTasksWaitingToSend),0);			
//			AddTaskptob(NULL,&delayList,1); //���뵽�ӳٺ���
//		}
//	
//		return xReturn;
//	}
//		if(xReturn == pdPASS)  //�����ɹ�������д����
//	{
//		// ����Ƿ������������ڵȴ�����
//		if(pxQueue->xTasksWaitingToReceive != NULL)
//		{
//			// ���ȴ����͵�������
//			TaskControlBlock *xTaskToUnblock = pxQueue->xTasksWaitingToReceive;
//			// ������ӷ��͵ȴ��������Ƴ�����������뵽����������
//       removeTaskpfroma(&(pxQueue->xTasksWaitingToReceive), xTaskToUnblock);
//				removeTaskpfroma(&delayList,xTaskToUnblock);
//       AddTaskptob(xTaskToUnblock,&taskLists[xTaskToUnblock->priority], 1); // ���뵽�������в����������л�
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
    
	// �������Ƿ�����
	xReturn = xQueueSend(pxQueue, pvItemToQueue);
	if(xReturn == pdFAIL)  // ������
	{
		if(delay == 0)
		{
			// ��������ʧ��
//			triggerPendSV();
			return xReturn;
		}
		else
		{
			// ������ʱ
			set_task_delay(NULL, delay);
			
			// ����ǰ����Ӿ����������Ƴ�
			removeTaskpfroma(&taskLists[currentTask->priority], NULL);
			
			// ����ǰ������ӵ��ȴ����Ͷ��к��ӳٶ���
			AddTaskptob(NULL, &(pxQueue->xTasksWaitingToSend), 0);	
			AddTaskptob(NULL, &delayList, 1); // �����ӳٶ��У����������л�
		}
		return xReturn;
	}

	// �ɹ����ͺ󣬼���Ƿ��������ڵȴ�������Ϣ
	if(pxQueue->xTasksWaitingToReceive != NULL)
	{
		// ���ѵȴ����յ�����
		TaskControlBlock *xTaskToUnblock = pxQueue->xTasksWaitingToReceive;

		// �ӵȴ����ն������Ƴ����������������
		removeTaskpfroma(&(pxQueue->xTasksWaitingToReceive), xTaskToUnblock);
		removeTaskpfroma(&delayList, xTaskToUnblock);  // ȷ�����ӳٶ������Ƴ�
		AddTaskptob(xTaskToUnblock, &taskLists[xTaskToUnblock->priority], 1);  // ���뵽�������в����������л�
	}

	return xReturn;
}



/*
�ڽ�������ʱ����Ҫ�������Ƿ�Ϊ�գ����Ϊ�����������ֱ��������Ϣ������ͬʱ������ʱ���ơ������л������������ķ�������ȹ��ܡ�
*/
BaseType_t xQueueGenericReceive(Queue_t *pxQueue, void * const pvItemToQueue, TickType_t delay)
{
	BaseType_t xReturn = pdPASS;
  
	// �������Ƿ�Ϊ��
	xReturn = xQueueReceive(pxQueue, pvItemToQueue);
	if(xReturn == pdFAIL)  // ���п�
	{
		if(delay == 0)
		{
			// ��������ʧ��
			triggerPendSV();
			return xReturn;
		}
		else
		{
			// ������ʱ
			set_task_delay(NULL, delay);
			
			// ����ǰ����Ӿ����������Ƴ�
			removeTaskpfroma(&taskLists[currentTask->priority], NULL);

			// ����ǰ������ӵ��ȴ����ն��к��ӳٶ���
			AddTaskptob(NULL, &(pxQueue->xTasksWaitingToReceive), 0);	
			AddTaskptob(NULL, &delayList, 1);  // �����ӳٶ��У����������л�
		}
		
		return xReturn;
	}

	// �ɹ����պ󣬼���Ƿ��������ڵȴ�������Ϣ
	if(pxQueue->xTasksWaitingToSend != NULL)
	{
		// ���ѵȴ����͵�����
		TaskControlBlock *xTaskToUnblock = pxQueue->xTasksWaitingToSend;

		// �ӵȴ����Ͷ������Ƴ����������������
		removeTaskpfroma(&(pxQueue->xTasksWaitingToSend), xTaskToUnblock);
		removeTaskpfroma(&delayList, xTaskToUnblock);  // ȷ�����ӳٶ������Ƴ�
		AddTaskptob(xTaskToUnblock, &taskLists[xTaskToUnblock->priority], 1);  // ���뵽�������в����������л�
	}

	return xReturn;
}



//BaseType_t xQueueGenericReceive(Queue_t *pxQueue, void * const pvItemToQueue, TickType_t delay)
//{
//	BaseType_t xReturn = pdPASS;
//  BaseType_t xTaskSwitchRequired = pdFALSE;

//    
//	// �������Ƿ���ֵ�ɶ�
//   xReturn=xQueueReceive(pxQueue,pvItemToQueue);
//	if(xReturn == pdFAIL)  //����ʧ�ܣ��Ѿ�����
//	{
//		if(delay==0)
//		{
//				//����SystemClock
//				HAL_SYSTICK_Config(SystemCoreClock / (1000U / uwTickFreq));
//				triggerPendSV();  //Pensv�쳣
//		}
//		else
//		{
//			set_task_delay(NULL,delay);
//				removeTaskpfroma(&taskLists[currentTask->priority],NULL);
//				AddTaskptob(NULL,&(pxQueue->xTasksWaitingToReceive),0);	
//				AddTaskptob(NULL,&delayList,1); //���뵽�ӳٺ���
//		}
//	
//	}
//	if(xReturn == pdPASS)  //�����ɹ�������д����
//	{
//		// ����Ƿ������������ڵȴ�����
//		if(pxQueue->xTasksWaitingToSend != NULL)
//		{
//			// ���ȴ����͵�������
//			TaskControlBlock *xTaskToUnblock = pxQueue->xTasksWaitingToSend;
//			// ������ӷ��͵ȴ��������Ƴ�����������뵽����������
//       removeTaskpfroma(&(pxQueue->xTasksWaitingToSend), xTaskToUnblock);
//				removeTaskpfroma(&delayList,xTaskToUnblock);
////				AddTaskptob(task_2,&taskLists,1);
//       AddTaskptob(xTaskToUnblock,&taskLists[xTaskToUnblock->priority], 1); // ���뵽�������в����������л�
//		}
//		
//		
//		
//	}
//	
//	return xReturn;
//}
