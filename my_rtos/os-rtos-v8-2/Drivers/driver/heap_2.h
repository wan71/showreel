#ifndef __HEAP_2_H
#define __HEAP_2_H
#include "stm32f1xx_hal.h"
#include "cpu.h"

#define vTaskSuspendAll enterCritical
#define xTaskResumeAll exitCritical


#define HEAP_SIZE 1024 * 10  // 10KB
#define BLOCK_SIZE_MIN 16    // ��С���䵥Ԫ
#define portBYTE_ALIGNMENT ( ( size_t ) 8 )         // �ڴ�����С
#define portBYTE_ALIGNMENT_MASK ( 0x0007 )
#define portPOINTER_SIZE_TYPE uint32_t
#define configTOTAL_HEAP_SIZE                    ((size_t)100)
static uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
/* A few bytes might be lost to byte aligning the heap start address. */
#define configADJUSTED_HEAP_SIZE    ( configTOTAL_HEAP_SIZE - portBYTE_ALIGNMENT )
/* Define the linked list structure.  This is used to link free blocks in order
of their size. */
typedef struct A_BLOCK_LINK
{
   //ָ����һ�������ڴ�����ṹ��
	struct A_BLOCK_LINK *pxNextFreeBlock;	/*<< The next free block in the list. */
	//��¼������ֽ�������������ռ����ռ�ֽ���
	size_t xBlockSize;						/*<< The size of the free block. */
} BlockLink_t;

//����ṹ��������ռ�ֽ���
static const uint16_t heapSTRUCT_SIZE	= ( ( sizeof ( BlockLink_t ) + ( portBYTE_ALIGNMENT - 1 ) ) & ~portBYTE_ALIGNMENT_MASK );
//2������ṹ��������ռ�ֽ���������Ϊһ����ֵ���ڷ���ʱ������
#define heapMINIMUM_BLOCK_SIZE	( ( size_t ) ( heapSTRUCT_SIZE * 2 ) )

/* Create a couple of list links to mark the start and end of the list. */
//����2���ֲ���̬ȫ�ֽṹ��������ڹ���
static BlockLink_t xStart, xEnd;

/* Keeps track of the number of free bytes remaining, but says nothing about
fragmentation. */
static size_t xFreeBytesRemaining = configADJUSTED_HEAP_SIZE;
void* pvPortMalloc( size_t xWantedSize );




#endif

