#include "heap_1.h"
#include <stdlib.h>
/* Index into the ucHeap array. */

/* Max value that fits in a size_t type. */
#define portBYTE_ALIGNMENT      8
#define heapSIZE_MAX                    ( ~( ( size_t ) 0 ) )
#define traceMALLOC( pvAddress, uiSize )
/* Check if adding a and b will result in overflow. */
#define heapADD_WILL_OVERFLOW( a, b )   ( ( a ) > ( heapSIZE_MAX - ( b ) ) )
#define heapBITS_PER_BYTE		( ( size_t ) 8 )
static size_t xNextFreeByte = ( size_t ) 0U;

#define heapBLOCK_ALLOCATED_BITMASK    ( ( ( size_t ) 1 ) << ( ( sizeof( size_t ) * heapBITS_PER_BYTE ) - 1 ) )
	
#define heapBLOCK_SIZE_IS_VALID( xBlockSize )    ( ( ( xBlockSize ) & heapBLOCK_ALLOCATED_BITMASK ) == 0 )


static void prvInsertBlockIntoFreeList(BlockLink_t *pxBlockToInsert);
static void prvHeapInit( void )
{
	BlockLink_t *pxFirstFreeBlock;
	uint8_t *pucAlignedHeap;

	/* Ensure the heap starts on a correctly aligned boundary. */
    //��heap1������ͬ��ȷ��portBYTE_ALIGNMENT�ֽڶ��룬ʵ��ʹ�õ���ַ��pucAlignedHeap
	pucAlignedHeap = ( uint8_t * ) ( ( ( portPOINTER_SIZE_TYPE ) &ucHeap[ portBYTE_ALIGNMENT ] ) & ( ~( ( portPOINTER_SIZE_TYPE ) portBYTE_ALIGNMENT_MASK ) ) );

	/* xStart is used to hold a pointer to the first item in the list of free
	blocks.  The void cast is used to prevent compiler warnings. */
	//��������ṹ��ͷ����ʼ��,pxNextFreeBlockָ��ʵ��ʹ�õ���ַpucAlignedHeap
	xStart.pxNextFreeBlock = ( void * ) pucAlignedHeap;
	//��������ṹ��ͷ��û�п����ڴ棬����xBlockSize��0
	xStart.xBlockSize = ( size_t ) 0;

	/* xEnd is used to mark the end of the list of free blocks. */
	//��������ṹ��β����ʼ��,xBlockSize=configADJUSTED_HEAP_SIZE������Ϊ�˺������������,��������Կ����ֽ���
	xEnd.xBlockSize = configADJUSTED_HEAP_SIZE;
	//��������ṹ��β����ʼ��,pxNextFreeBlockָ��NULL��ʾ��β
	xEnd.pxNextFreeBlock = NULL;

	/* To start with there is a single free block that is sized to take up the
	entire heap space. */
	//��һ�����п�,pxFirstFreeBlock,������xStartָ���pucAlignedHeap
	pxFirstFreeBlock = ( void * ) pucAlignedHeap;
	//���Կ����ڴ�ΪconfigADJUSTED_HEAP_SIZE
	pxFirstFreeBlock->xBlockSize = configADJUSTED_HEAP_SIZE;
	//ָ���������ṹ��β��
	pxFirstFreeBlock->pxNextFreeBlock = &xEnd;
}

/* STATIC FUNCTIONS ARE DEFINED AS MACROS TO MINIMIZE THE FUNCTION CALL DEPTH. */
void* pvPortMalloc( size_t xWantedSize )
{
BlockLink_t *pxBlock, *pxPreviousBlock, *pxNewBlockLink;
static BaseType_t xHeapHasBeenInitialised = pdFALSE;
void *pvReturn = NULL;
//�������������ֹ��������
		enterCritical();
	{
		/* If this is the first call to malloc then the heap will require
		initialisation to setup the list of free blocks. */
		//��һ�ε��û��ʼ���ڴ��
		if( xHeapHasBeenInitialised == pdFALSE )
		{
			prvHeapInit();
			xHeapHasBeenInitialised = pdTRUE;
		}

		/* The wanted size is increased so it can contain a BlockLink_t
		structure in addition to the requested amount of bytes. */
		if( xWantedSize > 0 )
		{
		    //�û������ֽ���+����ṹ��ռ���ֽ���
			xWantedSize += heapSTRUCT_SIZE;

			/* Ensure that blocks are always aligned to the required number of bytes. */
			//�ܵ��ֽ����������ֽڶ���
			if( ( xWantedSize & portBYTE_ALIGNMENT_MASK ) != 0 )
			{
				/* Byte alignment required. */
				xWantedSize += ( portBYTE_ALIGNMENT - ( xWantedSize & portBYTE_ALIGNMENT_MASK ) );
			}
		}
		//�������ֽ�������0��С���ܹ����ֽ���
		if( ( xWantedSize > 0 ) && ( xWantedSize < configADJUSTED_HEAP_SIZE ) )
		{
			/* Blocks are stored in byte order - traverse the list from the start
			(smallest) block until one of adequate size is found. */
			//pxPreviousBlockָ��ͷ����
			pxPreviousBlock = &xStart;
			//pxBlockָ���һ����ʼ���п�
			pxBlock = xStart.pxNextFreeBlock;
			//��pxBlock������Ŀ��п��ֽ���С�ڴ������
			//��û�б��������п��������β����һֱ����
			while( ( pxBlock->xBlockSize < xWantedSize ) && ( pxBlock->pxNextFreeBlock != NULL ) )
			{
				//pxPreviousBlock�����Ǳ��浱ǰ���п����ṹ��,Ϊ�˺����ҵ����ص��ڴ��ַ
				pxPreviousBlock = pxBlock;
				//ָ����һ�����п����ṹ��
				pxBlock = pxBlock->pxNextFreeBlock;
			}

			/* If we found the end marker then a block of adequate size was not found. */
			//pxBlock�����ڽ�β˵���ҵ����ϴ�С�Ŀ��п�
			if( pxBlock != &xEnd )
			{
				/* Return the memory space - jumping over the BlockLink_t structure
				at its start. */
				//pvReturn�������ظ��û�������Ҫƫ��һ�����п����ṹ��ռ���ڴ��С
				pvReturn = ( void * ) ( ( ( uint8_t * ) pxPreviousBlock->pxNextFreeBlock ) + heapSTRUCT_SIZE );

				/* This block is being returned for use so must be taken out of the
				list of free blocks. */
				//��ΪpxPreviousBlock->pxNextFreeBlockָ��Ŀ��п鱻�����ˣ�
				//����Ҫ��pxPreviousBlock->pxNextFreeBlockָ��Ŀ��п��Ƴ���ȥ��
				//Ҳ����pxPreviousBlock->pxNextFreeBlockָ��pxBlock->pxNextFreeBlock
				//Ҳ�������������ȥ���Ǹ���
				pxPreviousBlock->pxNextFreeBlock = pxBlock->pxNextFreeBlock;

				/* If the block is larger than required it can be split into two. */
				//�����жϣ�
				//�����Ҫ�����ȥ���ڴ���СxBlockSize�ȷ����ȥ�Ļ�Ҫ��heapMINIMUM_BLOCK_SIZE(2������ṹ��)
				//Ϊ�˽�Լ�Ͱ��ٷֳ�2�飬һ�鷵�ظ��û���
				//һ�鹹��һ���µĿ��й���ṹ�������������
				if( ( pxBlock->xBlockSize - xWantedSize ) > heapMINIMUM_BLOCK_SIZE )
				{
					/* This block is to be split into two.  Create a new block
					following the number of bytes requested. The void cast is
					used to prevent byte alignment warnings from the compiler. */
					//ע������xWantedSize�ǹ���ṹ��ͺ�������Ҫ�ֽ���֮��
					//��������pxBlock������ƫ��xWantedSize��Ϊ�µĹ���ṹ��
					pxNewBlockLink = ( void * ) ( ( ( uint8_t * ) pxBlock ) + xWantedSize );

					/* Calculate the sizes of two blocks split from the single
					block. */
					//pxNewBlockLink�µĹ���ṹ���С
					//�Ǵ�����pxBlock->xBlockSize-xWantedSize
					pxNewBlockLink->xBlockSize = pxBlock->xBlockSize - xWantedSize;
					//����pxBlock->xBlockSize��СΪxWantedSize
					pxBlock->xBlockSize = xWantedSize;

					/* Insert the new block into the list of free blocks. */
					//���¹���Ŀ��й���ṹ�尴xBlockSize��С������뵽��������
					{
						//���¹���Ŀ��й���ṹ�尴xBlockSize��С������뵽��������
					prvInsertBlockIntoFreeList( ( pxNewBlockLink ) );

					}
				}
                //��ʣ�����ֽ���Ҫ��ȥ�����ȥ��
				xFreeBytesRemaining -= pxBlock->xBlockSize;
			}
		}
		traceMALLOC( pvReturn, xWantedSize );
	}
  exitCritical();

//���ظ��û�
//	return xWantedSize;
	return pvReturn;
}

static void prvInsertBlockIntoFreeList(BlockLink_t *pxBlockToInsert)
{
							BlockLink_t *Iter,*pxIterator;
							pxIterator = &xStart;
							uint8_t *puc;
						//pxBlockָ���һ����ʼ���п�
							Iter = xStart.pxNextFreeBlock;
							//��pxBlock������Ŀ��п��ֽ���С�ڴ������
							while((pxBlockToInsert < pxIterator->pxNextFreeBlock)&&( pxIterator->pxNextFreeBlock != NULL ))
							{
									pxIterator=Iter;
									Iter=Iter->pxNextFreeBlock;
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
						 pxIterator->pxNextFreeBlock=pxBlockToInsert;
	
	
}

size_t xPortGetFreeHeapSize( void )
{
    return xFreeBytesRemaining;
}

void vPortInitialiseBlocks( void )
{
    /* This just exists to keep the linker quiet. */
}

void vPortFree( void * pv )
{
    uint8_t * puc = ( uint8_t * ) pv;
    BlockLink_t * pxLink;

    if( pv != NULL )
    {
        /* The memory being freed will have an BlockLink_t structure immediately
         * before it. */
        puc -= xHeapStructSize;
		
        /* This unexpected casting is to keep some compilers from issuing
         * byte alignment warnings. */
        pxLink = ( void * ) puc;
			enterCritical();
			{
		
  /* Add this block to the list of free blocks. */
				prvInsertBlockIntoFreeList( ( ( BlockLink_t * ) pxLink ) );
				xFreeBytesRemaining += pxLink->xBlockSize;
				
			}
			exitCritical();	
            }
        }
    


