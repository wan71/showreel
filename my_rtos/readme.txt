嵌入式系统中的实时任务调度与同步
概述
本项目专注于嵌入式系统中的实时任务调度、任务同步和延时管理。我们实现了自定义的任务切换机制，主要使用 PendSV 来完成任务切换，同时还处理了任务队列管理和延时处理。这些机制为严格时间要求的嵌入式系统提供了轻量级、高效的调度器。

关键技术点与挑战
在开发过程中，我们遇到了多个关键技术点与挑战，以下是系统的核心部分概述：

1. PendSV：高效的任务切换
PendSV 是 ARM Cortex-M 系列处理器中用于任务切换的低优先级中断。我们利用 PendSV 来实现任务切换，其关键作用是触发上下文切换，从而在任务之间实现无缝切换。在任务切换过程中，restart_pendsv() 函数用于重置系统时钟并触发 PendSV 中断，以确保任务调度顺利进行。

技术难点：

如何确保 PendSV 仅在需要任务切换时触发。
如何结合 PendSV 与任务队列实现高效的多任务调度。
2. 任务调度：基于优先级的调度机制
我们的任务调度器通过使用任务优先级队列，确保优先级高的任务能够首先执行。同时，在同一优先级的任务之间，通过轮询（round-robin）的方式实现调度。每个优先级有独立的任务链表，调度器根据当前的系统状态来决定从哪个链表中取出任务进行执行。

技术难点：

如何设计任务列表以实现优先级调度。
如何平衡优先级调度与轮询调度，确保系统的公平性与实时性。
3. 任务同步：队列和任务延时管理
任务同步是多任务系统中的核心功能。我们实现了任务队列机制，当任务需要等待队列中的资源时，会被挂起到等待队列中。在发送或接收任务时，如果队列满或空，任务将根据情况被挂起或唤醒。

xQueueGenericSend() 和 xQueueGenericReceive() 函数负责处理消息队列的发送与接收，同时根据队列状态将任务移动到等待队列或延时队列。

技术难点：

在任务发送或接收失败时，如何将任务挂起并管理任务队列。
在任务延时结束或队列可用时，如何高效唤醒任务。
4. TaskDelay：任务延时机制
为了支持任务延时执行，我们实现了 TaskDelay() 函数。每个任务可以指定延时时间，超过指定时间后，任务会从延时队列中恢复，并重新加入就绪队列以便执行。

技术难点：

如何高效地管理延时队列，确保任务按时被唤醒。
如何与任务调度和同步机制结合，保证延时任务与其他任务的无缝切换。
代码中的示例
1. 使用 PendSV 和任务调度
c
复制代码
void restart_pendsv() {
   HAL_SYSTICK_Config(SystemCoreClock / (1000U / uwTickFreq));  // 重置 SystemClock
   triggerPendSV();  // 触发 PendSV 中断
}
2. 任务发送与队列管理
c
复制代码
BaseType_t xQueueGenericSend(Queue_t *pxQueue, const void * const pvItemToQueue, TickType_t delay) {
    BaseType_t xReturn = pdPASS;
    set_task_delay(NULL, delay);
    while(1) {
        enterCritical();
        xReturn = xQueueSend(pxQueue, pvItemToQueue);
        if (xReturn == pdFAIL && currentTask->delay > 0) {
            removeTaskpfroma(&taskLists[currentTask->priority], NULL);
            AddTaskptob(NULL, &(pxQueue->xTasksWaitingToSend));
            AddTaskptob(NULL, &delayList);
            exitCritical();
            restart_pendsv();
        } else if (xReturn == pdFAIL && currentTask->delay == 0) {
            removeTaskpfroma(&(pxQueue->xTasksWaitingToSend), NULL);
            exitCritical();
            return xReturn;
        } else {
            set_task_delay(NULL, 0);
            if (pxQueue->xTasksWaitingToReceive != NULL) {
                TaskControlBlock *xTaskToUnblock = pxQueue->xTasksWaitingToReceive;
                removeTaskpfroma(&(pxQueue->xTasksWaitingToReceive), xTaskToUnblock);
                removeTaskpfroma(&delayList, xTaskToUnblock);
                AddTaskptob(xTaskToUnblock, &taskLists[xTaskToUnblock->priority]);
                exitCritical();
                restart_pendsv();
            }
            exitCritical();
            return xReturn;
        }
    }
}
3. 延时队列管理
c
复制代码
int removeTaskpfroma(TaskHandle_t a, TaskHandle_t p) {
    TaskControlBlock *task = (TaskControlBlock *)p;
    TaskControlBlock **listHead = (TaskControlBlock **)a;
    TaskControlBlock *current = *listHead;
    TaskControlBlock *prev = NULL;
    int removeCount = 0;
    while (current != NULL) {
        if (current == task) {
            if (prev == NULL) {
                *listHead = current->next;
            } else {
                prev->next = current->next;
            }
            TaskControlBlock *taskToRemove = current;
            current = current->next;
            taskToRemove->next = NULL;
            removeCount++;
        } else {
            prev = current;
            current = current->next;
        }
    }
    return removeCount;
}
未来改进
在未来的版本中，我们计划改进以下几方面：

优化任务延时机制：通过使用更高效的数据结构来管理延时任务，使得唤醒操作更加快速。
多核支持：当前的实现基于单核系统，未来我们计划扩展支持多核任务调度，以提高系统的并行处理能力。
任务优先级动态调整：实现基于任务执行情况的优先级动态调整，使系统更加灵活应对任务负载变化。

