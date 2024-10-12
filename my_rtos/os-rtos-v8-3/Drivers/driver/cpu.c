#include "cpu.h"
//#include "cmsis_gcc.h"

#define NVIC_INT_CTRL 			0xE000Ed04	//寄存器地址
#define NVIC_PENDSVSET 		  0x10000000
#define NVIC_SYSPRI2			  0xE000ED22	//寄存器地址
#define NVIC_PENDSV_PRI		  0x000000FF

#define MEM32(addr)				*(volatile unsigned long *)(addr)
#define MEM8(addr)				 *(volatile unsigned char *)(addr)

#define configPRIO_BITS         4
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 	( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

volatile uint32_t my_basepri;  // 保存 BASEPRI 寄存器的值
volatile uint32_t criticalNesting = 0;  // 递归计数器，支持嵌套

void PendSV_init(void){
	//设置pendsv异常 
    __set_PSP(0);
	MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI;
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;
}


void triggerPendSV()  // 触发PendSV进行任务切换
{
	 MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;  //触发异常
	
}


__asm void PendSV_Handler(void) {
    IMPORT currentTask
    IMPORT nextTask
		extern schedule;
    // 获取 PSP 寄存器值（当前任务堆栈指针）
    MRS R0,PSP
    CBZ R0,PendSVHandler_nosave
	  STMDB R0!,{R4-R11}
   //把currentTask的地址 - > R1
    LDR R1,=currentTask
    //R1地址的内容 ->R1
    LDR R1,[R1]
    //让R1的内容保存栈顶的内容
    //即R1保存的是currentTask栈顶内容(压栈后的地址).
    STR R0,[R1]

PendSVHandler_nosave
		
			// 保存 LR 寄存器
			mov r0, #configMAX_SYSCALL_INTERRUPT_PRIORITY
			msr basepri, r0
			dsb
			isb
			MOV     R6, LR  //寄存器6是随意选的
			BL schedule	
			MOV     LR, R6
			mov r0, #0
			msr basepri, r0
			
		
			//交换
			LDR R0,=currentTask
			LDR R1,=nextTask
			LDR R2,[R1]
			STR R2,[R0]
		
		 //取出堆栈地址
			LDR R0,[R2]
			LDMIA R0!,{R4-R11}
			
			
//			MOV R3,LR
			//切换
			MSR PSP,R0
			
			// 为了在回到任务后，确保继续使用PSP堆栈
			ORR LR,LR,#0x04
			
			BX LR   //退出堆栈

}


//// 进入临界区，屏蔽低优先级中断
//void enterCritical(void) {
//    if (criticalNesting == 0) {
//        // 保存当前的 BASEPRI 值
//        my_basepri = __get_BASEPRI();

//        // 设置 BASEPRI，屏蔽优先级为16及以下的中断
//        __set_BASEPRI(0x10);

//        // 确保所有内存访问在屏蔽中断后完成
//        __asm volatile ("dsb");
//        __asm volatile ("isb");
//    }

//    // 递归计数加一
//    criticalNesting++;
//}

//// 退出临界区并恢复中断优先级
//void exitCritical(void) {
//    if (criticalNesting > 0) {
//        // 递归计数减一
//        criticalNesting--;

//        if (criticalNesting == 0) {
//            // 恢复之前保存的 BASEPRI 值
//            __set_BASEPRI(my_basepri);

//            // 确保所有内存操作在恢复 BASEPRI 后完成
//            __asm volatile ("dsb");
//            __asm volatile ("isb");
//        }
//    }
//}


volatile uint32_t lock = 0;  // 简单互斥锁变量

// 获取锁的函数，确保同步
void acquireLock(void) {
    uint32_t tmp;

    do {
        tmp = __LDREXW(&lock);    // 尝试获取锁
    } while (__STREXW(1, &lock) || tmp != 0);  // 如果锁已经被其他任务持有，则继续重试
}

// 释放锁的函数
void releaseLock(void) {
    __DMB();                       // 确保所有内存操作完成
    lock = 0;                      // 释放锁
}

// 进入临界区
void enterCritical(void) {
    acquireLock();                 // 获取锁，确保同步

    if (criticalNesting == 0) {
        // 第一次进入临界区，可以在这里执行某些操作
    }

    criticalNesting++;             // 递增计数
    releaseLock();                 // 释放锁
}

// 退出临界区
void exitCritical(void) {
    acquireLock();                 // 获取锁，确保同步

    if (criticalNesting > 0) {
        criticalNesting--;         // 递减计数

        if (criticalNesting == 0) {
            // 最后一次退出临界区，可以执行清理操作
        }
    }

    releaseLock();                 // 释放锁
}


void restart_pendsv(void)
{
    // 重新配置 SysTick 计时器，确保定时器在 1ms 的周期下运行
    HAL_SYSTICK_Config(SystemCoreClock / (1000U / uwTickFreq));  
    
    // 确保数据同步，防止流水线中的延迟
    __asm volatile ("dsb");  // 确保所有内存访问已完成
    __asm volatile ("isb");  // 刷新指令流水线，确保 PendSV 被及时触发

    // 触发 PendSV 中断，要求任务切换
    triggerPendSV();  
}


