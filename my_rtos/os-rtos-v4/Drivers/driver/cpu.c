#include "cpu.h"

#include "main.h"
#define NVIC_INT_CTRL 			0xE000Ed04	//寄存器地址
#define NVIC_PENDSVSET 		  0x10000000
#define NVIC_SYSPRI2			  0xE000ED22	//寄存器地址
#define NVIC_PENDSV_PRI		  0x000000FF

#define MEM32(addr)				*(volatile unsigned long *)(addr)
#define MEM8(addr)				 *(volatile unsigned char *)(addr)

//extern TaskControlBlock *currentTask;
//extern TaskControlBlock *nextTask;


void los_task_run(void){
	//设置pendsv异常 
    __set_PSP(0);
	MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI;
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;
}

void los_task_switch(){
    MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;  //触发异常
}

__asm void PendSV_Handler(void) {
    IMPORT currentTask
    IMPORT nextTask

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
    //交换
    LDR R0,=currentTask
    LDR R1,=nextTask
    LDR R2,[R1]
    STR R2,[R0]

    //取出堆栈地址
    LDR R0,[R2]
    LDMIA R0!,{R4-R11}

    //切换
    MSR PSP,R0
    // 为了在回到任务后，确保继续使用PSP堆栈
    ORR LR,LR,#0x04
    BX LR   //退出堆栈
}

//__asm void PendSV_Handler(void) {
//    IMPORT currentTask     // 当前任务指针
//    IMPORT nextTask        // 下一个任务指针
//    IMPORT schedule        // 调度函数，用于选择下一个任务
//    
//    MRS R0, PSP            // 获取当前任务的 PSP
//    CBZ R0, no_save        // 如果 PSP 为 0，不需要保存上下文（即系统第一次进入时）

//    // 保存当前任务的上下文（R4-R11）
//    STMDB R0!, {R4-R11}    // 将 R4-R11 寄存器压入当前任务的堆栈
//    LDR R1, =currentTask   // 获取 currentTask 的地址
//    LDR R1, [R1]           // 获取 currentTask 的指针
//    STR R0, [R1]           // 将 PSP 保存到 currentTask 的堆栈指针中

//no_save
//    BL schedule            // 调用调度函数，选择下一个任务
//    
//    LDR R1, =nextTask      // 获取 nextTask 的地址
//    LDR R1, [R1]           // 获取 nextTask 的指针
//    LDR R0, [R1]           // 获取 nextTask 的堆栈指针
//    
//    // 恢复下一个任务的上下文（R4-R11）
//    LDMIA R0!, {R4-R11}    // 从任务堆栈中恢复 R4-R11 寄存器
//    MSR PSP, R0            // 将 PSP 设置为 nextTask 的堆栈指针
//    
//    ORR LR, LR, #0x04      // 使用 PSP 返回
//    BX LR                  // 返回到下一个任务
//}

