#ifndef __HEAP_2_H
#define __HEAP_2_H
#include "stm32f1xx_hal.h"
#include "cpu.h"

#define vTaskSuspendAll enterCritical
#define xTaskResumeAll exitCritical


#define HEAP_SIZE 1024 * 10  // 10KB
#define BLOCK_SIZE_MIN 16    // 最小分配单元
#define portBYTE_ALIGNMENT ( ( size_t ) 8 )         // 内存对齐大小
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
   //指向下一个空闲内存块管理结构体
	struct A_BLOCK_LINK *pxNextFreeBlock;	/*<< The next free block in the list. */
	//记录申请的字节数，包括链表占用所占字节数
	size_t xBlockSize;						/*<< The size of the free block. */
} BlockLink_t;

//链表结构体对齐后所占字节数
static const uint16_t heapSTRUCT_SIZE	= ( ( sizeof ( BlockLink_t ) + ( portBYTE_ALIGNMENT - 1 ) ) & ~portBYTE_ALIGNMENT_MASK );
//2倍链表结构体对齐后所占字节数，这作为一个阈值，在分配时起作用
#define heapMINIMUM_BLOCK_SIZE	( ( size_t ) ( heapSTRUCT_SIZE * 2 ) )

/* Create a couple of list links to mark the start and end of the list. */
//定义2个局部静态全局结构体变量用于管理
static BlockLink_t xStart, xEnd;

/* Keeps track of the number of free bytes remaining, but says nothing about
fragmentation. */
static size_t xFreeBytesRemaining = configADJUSTED_HEAP_SIZE;
void* pvPortMalloc( size_t xWantedSize );




#endif

