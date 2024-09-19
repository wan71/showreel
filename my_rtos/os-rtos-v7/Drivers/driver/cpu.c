#include "cpu.h"

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
volatile uint32_t my_basepri;  // ���� BASEPRI �Ĵ�����ֵ
volatile uint32_t criticalNesting = 0;  // �ݹ��������֧��Ƕ��

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


// �����ٽ��������ε����ȼ��ж�
void enterCritical(void) {
    if (criticalNesting == 0) {
        // ���浱ǰ�� BASEPRI ֵ
        my_basepri = __get_BASEPRI();

        // ���� BASEPRI���������ȼ�Ϊ16�����µ��ж�
        __set_BASEPRI(0x10);

        // ȷ�������ڴ�����������жϺ����
        __asm volatile ("dsb");
        __asm volatile ("isb");
    }

    // �ݹ������һ
    criticalNesting++;
}

// �˳��ٽ������ָ��ж����ȼ�
void exitCritical(void) {
    if (criticalNesting > 0) {
        // �ݹ������һ
        criticalNesting--;

        if (criticalNesting == 0) {
            // �ָ�֮ǰ����� BASEPRI ֵ
            __set_BASEPRI(my_basepri);

            // ȷ�������ڴ�����ڻָ� BASEPRI �����
            __asm volatile ("dsb");
            __asm volatile ("isb");
        }
    }
}

void restart_pendsv(void)
{
    // �������� SysTick ��ʱ����ȷ����ʱ���� 1ms ������������
    HAL_SYSTICK_Config(SystemCoreClock / (1000U / uwTickFreq));  
    
    // ȷ������ͬ������ֹ��ˮ���е��ӳ�
    __asm volatile ("dsb");  // ȷ�������ڴ���������
    __asm volatile ("isb");  // ˢ��ָ����ˮ�ߣ�ȷ�� PendSV ����ʱ����

    // ���� PendSV �жϣ�Ҫ�������л�
    triggerPendSV();  
}


