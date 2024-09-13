#include "cpu.h"

#include "main.h"
#define NVIC_INT_CTRL 			0xE000Ed04	//�Ĵ�����ַ
#define NVIC_PENDSVSET 		  0x10000000
#define NVIC_SYSPRI2			  0xE000ED22	//�Ĵ�����ַ
#define NVIC_PENDSV_PRI		  0x000000FF

#define MEM32(addr)				*(volatile unsigned long *)(addr)
#define MEM8(addr)				 *(volatile unsigned char *)(addr)

//extern TaskControlBlock *currentTask;
//extern TaskControlBlock *nextTask;


void los_task_run(void){
	//����pendsv�쳣 
    __set_PSP(0);
	MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI;
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;
}

void los_task_switch(){
    MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;  //�����쳣
}

__asm void PendSV_Handler(void) {
    IMPORT currentTask
    IMPORT nextTask

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
    //����
    LDR R0,=currentTask
    LDR R1,=nextTask
    LDR R2,[R1]
    STR R2,[R0]

    //ȡ����ջ��ַ
    LDR R0,[R2]
    LDMIA R0!,{R4-R11}

    //�л�
    MSR PSP,R0
    // Ϊ���ڻص������ȷ������ʹ��PSP��ջ
    ORR LR,LR,#0x04
    BX LR   //�˳���ջ
}

//__asm void PendSV_Handler(void) {
//    IMPORT currentTask     // ��ǰ����ָ��
//    IMPORT nextTask        // ��һ������ָ��
//    IMPORT schedule        // ���Ⱥ���������ѡ����һ������
//    
//    MRS R0, PSP            // ��ȡ��ǰ����� PSP
//    CBZ R0, no_save        // ��� PSP Ϊ 0������Ҫ���������ģ���ϵͳ��һ�ν���ʱ��

//    // ���浱ǰ����������ģ�R4-R11��
//    STMDB R0!, {R4-R11}    // �� R4-R11 �Ĵ���ѹ�뵱ǰ����Ķ�ջ
//    LDR R1, =currentTask   // ��ȡ currentTask �ĵ�ַ
//    LDR R1, [R1]           // ��ȡ currentTask ��ָ��
//    STR R0, [R1]           // �� PSP ���浽 currentTask �Ķ�ջָ����

//no_save
//    BL schedule            // ���õ��Ⱥ�����ѡ����һ������
//    
//    LDR R1, =nextTask      // ��ȡ nextTask �ĵ�ַ
//    LDR R1, [R1]           // ��ȡ nextTask ��ָ��
//    LDR R0, [R1]           // ��ȡ nextTask �Ķ�ջָ��
//    
//    // �ָ���һ������������ģ�R4-R11��
//    LDMIA R0!, {R4-R11}    // �������ջ�лָ� R4-R11 �Ĵ���
//    MSR PSP, R0            // �� PSP ����Ϊ nextTask �Ķ�ջָ��
//    
//    ORR LR, LR, #0x04      // ʹ�� PSP ����
//    BX LR                  // ���ص���һ������
//}

