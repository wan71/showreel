#ifndef __HEAP_1_H
#define __HEAP_1_H
#include "stm32f1xx_hal.h"
#include "cpu.h"

#define HEAP_SIZE 1024 * 10  // 10KB
#define BLOCK_SIZE_MIN 16    // 最小分配单元
#define portBYTE_ALIGNMENT ( ( size_t ) 8 )         // 内存对齐大小
#define portBYTE_ALIGNMENT_MASK ( 0x0007 )
#define portPOINTER_SIZE_TYPE uint32_t
#define configTOTAL_HEAP_SIZE                    ((size_t)0xC00)
static uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
/* A few bytes might be lost to byte aligning the heap start address. */
#define configADJUSTED_HEAP_SIZE    ( configTOTAL_HEAP_SIZE - portBYTE_ALIGNMENT )
void * pvPortMalloc( size_t xWantedSize );
#endif