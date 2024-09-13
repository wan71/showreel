#include <thread.h>


//TaskControlBlock *currentTask;
//TaskControlBlock *nextTask;
TaskControlBlock *currentTask = NULL;  // 当前运行任务
TaskControlBlock *nextTask = NULL;     // 下一个任务
TaskControlBlock taskTable[MAX_TASKS];  // 存储所有任务的 TCB
stack_task taskStacks[MAX_TASKS][TASK_STACK_SIZE];  // 每个任务的堆栈




void createTask(void (*taskFunc)(void *), int priority, stack_task *stack) {
    static int taskIndex = 0;
	  TaskControlBlock *p_stk;
    if (taskIndex < MAX_TASKS) {
			 //初始化任务栈
    //传入末端,先减后操作.
	   stack=stack + TASK_STACK_SIZE -16;
    *(--stack) = (unsigned long)(1<<24);      //xPSR
    *(--stack) = (unsigned long)taskFunc;    //PC寄存器
    *(--stack) = (unsigned long)0x14;         //R14(LR)寄存器
    *(--stack) = (unsigned long)0x12;         //R12
    *(--stack) = (unsigned long)0x33333;         //R3
    *(--stack) = (unsigned long)0x22222;          //R2
    *(--stack) = (unsigned long)0x11111;          //R1
    *(--stack) = (unsigned long)0x0;        //R0(程序入口参数)

    //其他寄存器.
    *(--stack) = (unsigned long)0x11;       //R11
    *(--stack) = (unsigned long)0x10;       //R10
    *(--stack) = (unsigned long)0x9;        //R9
    *(--stack) = (unsigned long)0x8;        //R8
    *(--stack) = (unsigned long)0x7;
    *(--stack) = (unsigned long)0x6;
    *(--stack) = (unsigned long)0x5;
    *(--stack) = (unsigned long)0x4;
     taskTable[taskIndex].stackPointer = stack;
		 
//       taskTable[taskIndex].stackPointer = stack + TASK_STACK_SIZE - 16;  // 初始化栈指针
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



//int lastTaskIndex = -1;  // 记录上一次调度的任务索引

//void schedule() {
//    int highestPriority = -1;
//    TaskControlBlock *taskToRun = NULL;
//    int selectedTaskIndex = -1;

//    for (int i = 0; i < MAX_TASKS; i++) {
//        // 查找所有满足条件的任务，并比较优先级
//        if (taskTable[i].delay == 0 && taskTable[i].priority >= highestPriority) {
//            // 如果优先级相同，选择下一个任务，避免饥饿
//            if (taskTable[i].priority == highestPriority && i <= lastTaskIndex) {
//                continue;  // 跳过当前任务，直到找到下一个任务
//            }

//            highestPriority = taskTable[i].priority;
//            taskToRun = &taskTable[i];
//            selectedTaskIndex = i;
//        }
//    }

//    // 如果找不到比上次高优先级的任务，回到第一个可执行的任务
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
//        lastTaskIndex = selectedTaskIndex;  // 更新上次执行的任务索引
//        nextTask = taskToRun;
//       los_task_switch();  // 触发PendSV进行任务切换	
//    }
//}



