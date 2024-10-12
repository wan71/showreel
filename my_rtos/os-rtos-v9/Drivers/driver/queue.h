#ifndef __QUEUE_H
#define __QUEUE_H
#include "thread.h"

typedef   signed          char int8_t;
typedef unsigned long     UBaseType_t;
typedef long BaseType_t;
typedef void * QueueHandle_t;
#define pdFALSE			( ( BaseType_t ) 0 )
#define pdTRUE			( ( BaseType_t ) 1 )

#define pdPASS			( pdTRUE )
#define pdFAIL			( pdFALSE )

typedef struct QueueDefinition{
	int8_t *pcHead;                  /* ���д洢������ʼ��ַ */
  int8_t *pcTail;                  /* ���д洢���Ľ�����־ */
  int8_t *pcWriteTo;               /* ��һ��д���λ�� */
  int8_t *pcReadFrom;              /* ��ǰ����λ�� */
	
	TaskControlBlock *xTasksWaitingToSend;
	TaskControlBlock *xTasksWaitingToReceive;
	
	volatile UBaseType_t uxMessagesWaiting;/*< The number of items currently in the queue. *//* ��ǰ�����е���Ϣ���� */
	UBaseType_t uxLength;			/*< The length of the queue defined as the number of items it will hold, not the number of bytes. *//* ���еĳ��ȣ������п������ɵ����� */
	UBaseType_t uxItemSize;			/*< The size of each items that the queue will hold. *//* ÿ��������Ĵ�С */

	/* �����������ڶ�������� */
	volatile int8_t cRxLock;		/*< Stores the number of items received from the queue (removed from the queue) while the queue was locked.  Set to queueUNLOCKED when the queue is not locked. */
	volatile int8_t cTxLock;		/*< Stores the number of items transmitted to the queue (added to the queue) while the queue was locked.  Set to queueUNLOCKED when the queue is not locked. */
	/* �����������ڶ�������� */
}xQUEUE;

typedef xQUEUE Queue_t;
typedef uint32_t TickType_t;
/******************************************************************�Լ�����*******************************************************************/



Queue_t *xQueueCreate(UBaseType_t uxQueueLength, UBaseType_t uxItemSize);
BaseType_t xQueueSend(Queue_t *pxQueue, const void *pvItemToQueue);
BaseType_t xQueueReceive(Queue_t *pxQueue, void *pvBuffer);
void vQueuePrint(Queue_t *pxQueue);
BaseType_t xQueueGenericSend(Queue_t *pxQueue, const void * const pvItemToQueue, TickType_t delay);
BaseType_t xQueueGenericReceive(Queue_t *pxQueue, void * const pvItemToQueue, TickType_t delay);
#endif

