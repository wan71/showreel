#include <thread.h>
#include <string.h>
void los_task_init(losTask * task,void (*taskEntry)(void *),void *param,stack_task *stack){
    //��ʼ������ջ
    //����ĩ��,�ȼ������.
		stack_task test1 = 0;
		stack_task *test2 = 0;
    *(--stack) = (unsigned long)(1<<24);      //xPSR
    *(--stack) = (unsigned long)taskEntry;    //PC�Ĵ���
		test2 = stack;
    *(--stack) = (unsigned long)0x14;         //R14(LR)�Ĵ���
		test2 = stack;
    *(--stack) = (unsigned long)0x12;         //R12
	test2 = stack;
    *(--stack) = (unsigned long)0x33333;         //R3
	test2 = stack;
    *(--stack) = (unsigned long)0x22222;          //R2
    *(--stack) = (unsigned long)0x11111;          //R1
    *(--stack) = (unsigned long)param;        //R0(������ڲ���)

    //�����Ĵ���.
    *(--stack) = (unsigned long)0x11;       //R11
    *(--stack) = (unsigned long)0x10;       //R10
    *(--stack) = (unsigned long)0x9;        //R9
    *(--stack) = (unsigned long)0x8;        //R8
    *(--stack) = (unsigned long)0x7;
    *(--stack) = (unsigned long)0x6;
    *(--stack) = (unsigned long)0x5;
    *(--stack) = (unsigned long)0x4;
	
    task->stack = stack;
}





void los_task_sched(){
//	if (currentTask == taskTable[0]){
//		nextTask = taskTable[1];
//	}else{
//		nextTask = taskTable[0];
//	}
	los_task_switch();
}