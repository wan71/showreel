#include "heap_2.h"

static void prvInsertBlockIntoFreeList(BlockLink_t *pxBlockToInsert);
void traceMALLOC(void *pvReturn, size_t xWantedSize);
static void prvHeapInit( void )
{
BlockLink_t *pxFirstFreeBlock;
uint8_t *pucAlignedHeap;

	/* Ensure the heap starts on a correctly aligned boundary. */
    //与heap1操作相同，确保portBYTE_ALIGNMENT字节对齐，实际使用的首址是pucAlignedHeap
	pucAlignedHeap = ( uint8_t * ) ( ( ( portPOINTER_SIZE_TYPE ) &ucHeap[ portBYTE_ALIGNMENT ] ) & ( ~( ( portPOINTER_SIZE_TYPE ) portBYTE_ALIGNMENT_MASK ) ) );

	/* xStart is used to hold a pointer to the first item in the list of free
	blocks.  The void cast is used to prevent compiler warnings. */
	//空闲链表结构体头部初始化,pxNextFreeBlock指向实际使用的首址pucAlignedHeap
	xStart.pxNextFreeBlock = ( void * ) pucAlignedHeap;
	//空闲链表结构体头部没有可用内存，所以xBlockSize是0
	xStart.xBlockSize = ( size_t ) 0;

	/* xEnd is used to mark the end of the list of free blocks. */
	//空闲链表结构体尾部初始化,xBlockSize=configADJUSTED_HEAP_SIZE仅仅是为了后面的升序排列,不代表可以空闲字节数
	xEnd.xBlockSize = configADJUSTED_HEAP_SIZE;
	//空闲链表结构体尾部初始化,pxNextFreeBlock指向NULL表示结尾
	xEnd.pxNextFreeBlock = NULL;

	/* To start with there is a single free block that is sized to take up the
	entire heap space. */
	//第一个空闲块,pxFirstFreeBlock,即上面xStart指向的pucAlignedHeap
	pxFirstFreeBlock = ( void * ) pucAlignedHeap;
	//可以空闲内存为configADJUSTED_HEAP_SIZE
	pxFirstFreeBlock->xBlockSize = configADJUSTED_HEAP_SIZE;
	//指向空闲链表结构体尾部
	pxFirstFreeBlock->pxNextFreeBlock = &xEnd;
}

/* STATIC FUNCTIONS ARE DEFINED AS MACROS TO MINIMIZE THE FUNCTION CALL DEPTH. */


void* pvPortMalloc( size_t xWantedSize )
{
BlockLink_t *pxBlock, *pxPreviousBlock, *pxNewBlockLink;
static BaseType_t xHeapHasBeenInitialised = pdFALSE;
void *pvReturn = NULL;
//挂起调度器，防止函数重入
	vTaskSuspendAll();
	{
		/* If this is the first call to malloc then the heap will require
		initialisation to setup the list of free blocks. */
		//第一次调用会初始化内存堆
		if( xHeapHasBeenInitialised == pdFALSE )
		{
			prvHeapInit();
			xHeapHasBeenInitialised = pdTRUE;
		}

		/* The wanted size is increased so it can contain a BlockLink_t
		structure in addition to the requested amount of bytes. */
		if( xWantedSize > 0 )
		{
		    //用户分配字节数+管理结构体占用字节数
			xWantedSize += heapSTRUCT_SIZE;

			/* Ensure that blocks are always aligned to the required number of bytes. */
			//总的字节数再做此字节对齐
			if( ( xWantedSize & portBYTE_ALIGNMENT_MASK ) != 0 )
			{
				/* Byte alignment required. */
				xWantedSize += ( portBYTE_ALIGNMENT - ( xWantedSize & portBYTE_ALIGNMENT_MASK ) );
			}
		}
		//待分配字节数大于0且小于总共堆字节数
		if( ( xWantedSize > 0 ) && ( xWantedSize < configADJUSTED_HEAP_SIZE ) )
		{
			/* Blocks are stored in byte order - traverse the list from the start
			(smallest) block until one of adequate size is found. */
			//pxPreviousBlock指向头链表
			pxPreviousBlock = &xStart;
			//pxBlock指向第一个开始空闲块
			pxBlock = xStart.pxNextFreeBlock;
			//当pxBlock所管理的空闲块字节数小于待分配的
			//且没有遍历到空闲块管理链表尾部则一直遍历
			while( ( pxBlock->xBlockSize < xWantedSize ) && ( pxBlock->pxNextFreeBlock != NULL ) )
			{
				//pxPreviousBlock这里是保存当前空闲块管理结构体,为了后面找到返回的内存地址
				pxPreviousBlock = pxBlock;
				//指向下一个空闲块管理结构体
				pxBlock = pxBlock->pxNextFreeBlock;
			}

			/* If we found the end marker then a block of adequate size was not found. */
			//pxBlock不等于结尾说明找到符合大小的空闲块
			if( pxBlock != &xEnd )
			{
				/* Return the memory space - jumping over the BlockLink_t structure
				at its start. */
				//pvReturn用作返回给用户，这里要偏移一个空闲块管理结构体占用内存大小
				pvReturn = ( void * ) ( ( ( uint8_t * ) pxPreviousBlock->pxNextFreeBlock ) + heapSTRUCT_SIZE );

				/* This block is being returned for use so must be taken out of the
				list of free blocks. */
				//因为pxPreviousBlock->pxNextFreeBlock指向的空闲块被分配了，
				//所以要把pxPreviousBlock->pxNextFreeBlock指向的空闲块移除出去，
				//也就是pxPreviousBlock->pxNextFreeBlock指向pxBlock->pxNextFreeBlock
				//也就是跳过分配出去的那个块
				pxPreviousBlock->pxNextFreeBlock = pxBlock->pxNextFreeBlock;

				/* If the block is larger than required it can be split into two. */
				//这里判断，
				//如果将要分配出去的内存块大小xBlockSize比分配出去的还要大heapMINIMUM_BLOCK_SIZE(2倍管理结构体)
				//为了节约就把再分成2块，一块返回给用户，
				//一块构造一个新的空闲管理结构体后插入空闲链表
				if( ( pxBlock->xBlockSize - xWantedSize ) > heapMINIMUM_BLOCK_SIZE )
				{
					/* This block is to be split into two.  Create a new block
					following the number of bytes requested. The void cast is
					used to prevent byte alignment warnings from the compiler. */
					//注意这里xWantedSize是管理结构体和和真正需要字节数之和
					//所以是在pxBlock基础上偏移xWantedSize作为新的管理结构体
					pxNewBlockLink = ( void * ) ( ( ( uint8_t * ) pxBlock ) + xWantedSize );

					/* Calculate the sizes of two blocks split from the single
					block. */
					//pxNewBlockLink新的管理结构体大小
					//是待分配pxBlock->xBlockSize-xWantedSize
					pxNewBlockLink->xBlockSize = pxBlock->xBlockSize - xWantedSize;
					//更新pxBlock->xBlockSize大小为xWantedSize
					pxBlock->xBlockSize = xWantedSize;

					/* Insert the new block into the list of free blocks. */
					//把新构造的空闲管理结构体按xBlockSize大小升序插入到空闲链表
					prvInsertBlockIntoFreeList( ( pxNewBlockLink ) );
				}
                //还剩空闲字节数要减去分配出去的
				xFreeBytesRemaining -= pxBlock->xBlockSize;
			}
		}
		traceMALLOC( pvReturn, xWantedSize );
	}
 xTaskResumeAll();

//返回给用户
//	return xWantedSize;
	return pvReturn;
}


static void prvInsertBlockIntoFreeList(BlockLink_t *pxBlockToInsert)
{
    BlockLink_t *pxIterator;
    uint8_t *puc;

    /* Iterate through the list until a block is found that has a higher address
    than the block being inserted. */
    for (pxIterator = &xStart; pxIterator->pxNextFreeBlock < pxBlockToInsert; pxIterator = pxIterator->pxNextFreeBlock)
    {
        // Traverse until the correct position is found
    }

    /* Adjust the links to insert the block into the list. */
    puc = (uint8_t *) pxIterator->pxNextFreeBlock;
    if ((puc + pxIterator->xBlockSize) == (uint8_t *) pxBlockToInsert)
    {
        /* The block being inserted is adjacent to the block it is being inserted
        after, so the two blocks can be merged. */
        pxIterator->xBlockSize += pxBlockToInsert->xBlockSize;
        pxBlockToInsert = pxIterator;
    }

    puc = (uint8_t *) pxBlockToInsert;
    if ((puc + pxBlockToInsert->xBlockSize) == (uint8_t *) pxIterator->pxNextFreeBlock)
    {
        /* The block being inserted is adjacent to the block it is being inserted
        before, so the two blocks can be merged. */
        pxBlockToInsert->xBlockSize += pxIterator->pxNextFreeBlock->xBlockSize;
        pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock->pxNextFreeBlock;
    }
    else
    {
        pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock;
    }

    pxIterator->pxNextFreeBlock = pxBlockToInsert;
}

// 记录内存分配的函数
void traceMALLOC(void *pvReturn, size_t xWantedSize) {
    if (pvReturn == NULL) {
        // 如果分配失败，打印失败信息
        printf("Memory allocation failed for size: %zu bytes\n", xWantedSize);
    } else {
        // 如果分配成功，打印分配的内存地址和大小
        printf("Memory allocated at address: %p, size: %zu bytes\n", pvReturn, xWantedSize);
    }
}


