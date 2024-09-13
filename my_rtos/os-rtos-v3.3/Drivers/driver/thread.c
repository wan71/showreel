#include <thread.h>


//TaskControlBlock *currentTask;
//TaskControlBlock *nextTask;
TaskControlBlock *currentTask = NULL;  // ��ǰ��������
TaskControlBlock *nextTask = NULL;     // ��һ������
TaskControlBlock taskTable[MAX_TASKS];  // �洢��������� TCB
stack_task taskStacks[MAX_TASKS][TASK_STACK_SIZE];  // ÿ������Ķ�ջ




void createTask(void (*taskFunc)(void *), int priority, stack_task *stack) {
    static int taskIndex = 0;
	  TaskControlBlock *p_stk;
    if (taskIndex < MAX_TASKS) {
			 //��ʼ������ջ
    //����ĩ��,�ȼ������.
	   stack=stack + TASK_STACK_SIZE -16;
    *(--stack) = (unsigned long)(1<<24);      //xPSR
    *(--stack) = (unsigned long)taskFunc;    //PC�Ĵ���
    *(--stack) = (unsigned long)0x14;         //R14(LR)�Ĵ���
    *(--stack) = (unsigned long)0x12;         //R12
    *(--stack) = (unsigned long)0x33333;         //R3
    *(--stack) = (unsigned long)0x22222;          //R2
    *(--stack) = (unsigned long)0x11111;          //R1
    *(--stack) = (unsigned long)0x0;        //R0(������ڲ���)

    //�����Ĵ���.
    *(--stack) = (unsigned long)0x11;       //R11
    *(--stack) = (unsigned long)0x10;       //R10
    *(--stack) = (unsigned long)0x9;        //R9
    *(--stack) = (unsigned long)0x8;        //R8
    *(--stack) = (unsigned long)0x7;
    *(--stack) = (unsigned long)0x6;
    *(--stack) = (unsigned long)0x5;
    *(--stack) = (unsigned long)0x4;
     taskTable[taskIndex].stackPointer = stack;
		 
//       taskTable[taskIndex].stackPointer = stack + TASK_STACK_SIZE - 16;  // ��ʼ��ջָ��
      taskTable[taskIndex].priority = priority;
      taskTable[taskIndex].delay = 0;
		
        taskIndex++;
    }
}


static int count=0;
void schedule() {
	 int highestPriority = -1;
	 TaskControlBlock *taskToRun = NULL;
	 count=++count%MAX_TASKS;
	 taskToRun = &taskTable[count];
	 nextTask = taskToRun;
	 los_task_switch();
	
	
}



//int lastTaskIndex = -1;  // ��¼��һ�ε��ȵ���������

//void schedule() {
//    int highestPriority = -1;
//    TaskControlBlock *taskToRun = NULL;
//    int selectedTaskIndex = -1;

//    for (int i = 0; i < MAX_TASKS; i++) {
//        // ���������������������񣬲��Ƚ����ȼ�
//        if (taskTable[i].delay == 0 && taskTable[i].priority >= highestPriority) {
//            // ������ȼ���ͬ��ѡ����һ�����񣬱��⼢��
//            if (taskTable[i].priority == highestPriority && i <= lastTaskIndex) {
//                continue;  // ������ǰ����ֱ���ҵ���һ������
//            }

//            highestPriority = taskTable[i].priority;
//            taskToRun = &taskTable[i];
//            selectedTaskIndex = i;
//        }
//    }

//    // ����Ҳ������ϴθ����ȼ������񣬻ص���һ����ִ�е�����
//    if (selectedTaskIndex == -1 && highestPriority != -1) {
//        for (int i = 0; i < MAX_TASKS; i++) {
//            if (taskTable[i].delay == 0 && taskTable[i].priority == highestPriority) {
//                taskToRun = &taskTable[i];
//                selectedTaskIndex = i;
//                break;
//            }
//        }
//    }

//    if (taskToRun && taskToRun != currentTask) {
//        lastTaskIndex = selectedTaskIndex;  // �����ϴ�ִ�е���������
//        nextTask = taskToRun;
//       los_task_switch();  // ����PendSV���������л�	
//    }
//}



