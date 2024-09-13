#include "cpu.h"
#include "thread.h"
#include "main.h"
#define NVIC_INT_CTRL 			0xE000Ed04	//寄存器地址
#define NVIC_PENDSVSET 		  0x10000000
#define NVIC_SYSPRI2			  0xE000ED22	//寄存器地址
#define NVIC_PENDSV_PRI		  0x000000FF

#define MEM32(addr)				*(volatile unsigned long *)(addr)
#define MEM8(addr)				 *(volatile unsigned char *)(addr)
	
#define configPRIO_BITS         4
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 	( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

extern losTask *currentTask;
extern losTask *nextTask;
void schedule(void);
void pp(void);

void los_task_run(void){
	//设置pendsv异常 
    __set_PSP(0);
	MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI;
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;
}

void los_task_switch(){
    MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;  //触发异常
}



//__asm void PendSV_Handler(void) {
//    // 保存寄存器上下文
//    MRS R0, PSP               // 获取当前进程堆栈指针
//    STMDB R0!, {R4-R11}       // 保存寄存器R4-R11到进程堆栈
//    
//    // 测试代码，可以在此处做一些简单的修改
//    ADD R4, R4, #1            // 简单增加R4的值
//    ADD R5, R5, #1            // 简单增加R5的值
//	
//    
//    // 恢复寄存器上下文
//    LDMIA R0!, {R4-R11}       // 从进程堆栈恢复R4-R11的值
//    MSR PSP, R0               // 恢复进程堆栈指针
//    BX LR                     // 退出异常处理程序
//}

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




void schedule(void) {
  if (currentTask == taskTable[0]) {
        nextTask = taskTable[1];
    } else {
        nextTask = taskTable[0];
    }
}

// 强制函数使用8字节对齐的栈空间
void pp(void){
    // 函数体
	char i;
}


