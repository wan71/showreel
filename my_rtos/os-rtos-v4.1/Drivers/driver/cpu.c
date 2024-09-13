#include "cpu.h"

#include "main.h"
#define NVIC_INT_CTRL 			0xE000Ed04	//�Ĵ�����ַ
#define NVIC_PENDSVSET 		  0x10000000
#define NVIC_SYSPRI2			  0xE000ED22	//�Ĵ�����ַ
#define NVIC_PENDSV_PRI		  0x000000FF

#define MEM32(addr)				*(volatile unsigned long *)(addr)
#define MEM8(addr)				 *(volatile unsigned char *)(addr)

#define configPRIO_BITS         4
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 	( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
//extern TaskControlBlock *currentTask;
//extern TaskControlBlock *nextTask;


void PendSV_init(void){
	//����pendsv�쳣 
    __set_PSP(0);
	MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI;
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;
}


void triggerPendSV()  // ����PendSV���������л�
{
	 MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;  //�����쳣
	
}


__asm void PendSV_Handler(void) {
    IMPORT currentTask
    IMPORT nextTask
		extern schedule;
    // ��ȡ PSP �Ĵ���ֵ����ǰ�����ջָ�룩
    MRS R0,PSP
    CBZ R0,PendSVHandler_nosave
	  STMDB R0!,{R4-R11}
   //��currentTask�ĵ�ַ - > R1
    LDR R1,=currentTask
    //R1��ַ������ ->R1
    LDR R1,[R1]
    //��R1�����ݱ���ջ��������
    //��R1�������currentTaskջ������(ѹջ��ĵ�ַ).
    STR R0,[R1]

PendSVHandler_nosave
		
			// ���� LR �Ĵ���
			mov r0, #configMAX_SYSCALL_INTERRUPT_PRIORITY
			msr basepri, r0
			dsb
			isb
			MOV     R6, LR  //�Ĵ���6������ѡ��
			BL schedule	
			MOV     LR, R6
			mov r0, #0
			msr basepri, r0
			
		
			//����
			LDR R0,=currentTask
			LDR R1,=nextTask
			LDR R2,[R1]
			STR R2,[R0]
		
		 //ȡ����ջ��ַ
			LDR R0,[R2]
			LDMIA R0!,{R4-R11}
			
			
//			MOV R3,LR
			//�л�
			MSR PSP,R0
			
			// Ϊ���ڻص������ȷ������ʹ��PSP��ջ
			ORR LR,LR,#0x04
			
			BX LR   //�˳���ջ

}

