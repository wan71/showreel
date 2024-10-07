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
	

    // �����ݿ���������
    memcpy(pxQueue->pcWriteTo, pvItemToQueue, pxQueue->uxItemSize);
    
    // ����дָ�룬ѭ������
    pxQueue->pcWriteTo += pxQueue->uxItemSize;
    if (pxQueue->pcWriteTo >= pxQueue->pcTail) {
        pxQueue->pcWriteTo = pxQueue->pcHead;
    }

    pxQueue->uxMessagesWaiting++;  // ��������Ϣ����+1
	
		
    return pdPASS;
}


/*���к������ڴӶ����ж�ȡ����*/
BaseType_t xQueueReceive(Queue_t *pxQueue, void *pvBuffer) {
    if (pxQueue->uxMessagesWaiting == 0) {
        // ����Ϊ��
        return pdFAIL;
    }

    // �����ݴӶ����ж�ȡ��������
    memcpy(pvBuffer, pxQueue->pcReadFrom, pxQueue->uxItemSize);

    // ���¶�ָ�룬ѭ������
    pxQueue->pcReadFrom += pxQueue->uxItemSize;
    if (pxQueue->pcReadFrom >= pxQueue->pcTail) {
        pxQueue->pcReadFrom = pxQueue->pcHead;
    }

    pxQueue->uxMessagesWaiting--;  // ��������Ϣ����-1
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

BaseType_t xQueueGenericSend(Queue_t *pxQueue, const void * const pvItemToQueue, TickType_t delay)
{
	BaseType_t xReturn = pdPASS;
				// ������ʱ�ȴ�
	set_task_delay(NULL, delay);
	while(1)
	{
		// �����ٽ�����ȷ�����в�����ԭ����
		enterCritical();

		// ��������з�����Ϣ
		xReturn = xQueueSend(pxQueue, pvItemToQueue);

		if (xReturn == pdFAIL)  // ��������
		{
			
		 if ((currentTask->delay) == 0)
			{			
				// �ȼ�� xTasksWaitingToSend �Ƿ�Ϊ�գ�����Ǳ�ڴ���
				if (pxQueue->xTasksWaitingToSend != NULL)
				{
				removeTaskpfroma(&(pxQueue->xTasksWaitingToSend), (TaskHandle_t)&currentTask);				
				}
				// �������Ҫ��ʱֱ���˳�
				exitCritical();
				return xReturn;
			}
			else if ((currentTask->delay) > 0)
			{
				
				
				// ����ǰ����Ӿ��������Ƴ��������ӳٶ���
				removeTaskpfroma(&taskLists[currentTask->priority], NULL);
				AddTaskptob(NULL, &(pxQueue->xTasksWaitingToSend));
				AddTaskptob(NULL, &delayList);  // �����ӳٶ���

				// �˳��ٽ��������������л�
				exitCritical();
				restart_pendsv();
			}
	
		}
		else
		{
			set_task_delay(NULL, 0);
			// �ɹ�������Ϣ�󣬼���Ƿ�������ȴ�����
			if (pxQueue->xTasksWaitingToReceive != NULL)
			{
				// ���ѵȴ����յ�����
				TaskControlBlock *xTaskToUnblock = pxQueue->xTasksWaitingToReceive;

				// �ӽ��յȴ������Ƴ����񣬲������������
				removeTaskpfroma(&(pxQueue->xTasksWaitingToReceive), xTaskToUnblock);
				removeTaskpfroma(&delayList, xTaskToUnblock);
				AddTaskptob(xTaskToUnblock, &taskLists[xTaskToUnblock->priority]);

				// �˳��ٽ��������������л�
				exitCritical();
				restart_pendsv();
				return xReturn;
			}

			// �˳��ٽ�������Ϣ�ѷ��ͳɹ����˳�ѭ��
			exitCritical();
			return xReturn;
		}
	}
}


BaseType_t xQueueGenericReceive(Queue_t *pxQueue, void * const pvItemToQueue, TickType_t delay)
{
	BaseType_t xReturn = pdPASS;
	// ������ʱ�ȴ�
	set_task_delay(NULL, delay);
	while(1)
	{
		// �����ٽ�����ȷ�����в�����ԭ����
		enterCritical();

		// ���ԴӶ����н�����Ϣ
		xReturn = xQueueReceive(pxQueue, pvItemToQueue);

		if (xReturn == pdFAIL)  // ����Ϊ��
		{
			if ((currentTask->delay) > 0)
			{


				// ����ǰ����Ӿ��������Ƴ��������ӳٶ���
				removeTaskpfroma(&taskLists[currentTask->priority], NULL);
				AddTaskptob(NULL, &(pxQueue->xTasksWaitingToReceive));
				AddTaskptob(NULL, &delayList);

				// �˳��ٽ��������������л�
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

				// �������Ҫ��ʱֱ���˳�
				exitCritical();
				return xReturn;
			}
		
		}
		else
		{
				set_task_delay(NULL, 0);
			// �ɹ����յ���Ϣ�󣬼���Ƿ�������ȴ�����
			if (pxQueue->xTasksWaitingToSend != NULL)
			{
				// ���ѵȴ����͵�����
				TaskControlBlock *xTaskToUnblock = pxQueue->xTasksWaitingToSend;

				// �ӷ��͵ȴ������Ƴ����񣬲������������
				removeTaskpfroma(&(pxQueue->xTasksWaitingToSend), xTaskToUnblock);
				removeTaskpfroma(&delayList, xTaskToUnblock);
				AddTaskptob(xTaskToUnblock, &taskLists[xTaskToUnblock->priority]);

				// �˳��ٽ��������������л�
				exitCritical();
				restart_pendsv();
				return xReturn;
			}

			// �˳��ٽ�������Ϣ�ѳɹ����գ��˳�ѭ��
			exitCritical();
			return xReturn;
		}
	}
}


