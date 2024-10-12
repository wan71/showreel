#include "callback.h"

/*
任务解耦和模块化：回调允许任务或驱动程序注册特定事件的处理函数，使得不同模块可以独立开发并且灵活交互，提升代码的可维护性。

异步处理：通过回调机制，可以在中断或其他触发点快速响应，而复杂逻辑则在任务上下文中异步执行，提高实时性。

事件驱动机制：RTOS通常处理大量的异步事件，回调使得事件可以在特定条件下灵活调度和处理。

内存与资源管理：回调系统可以配合内存管理机制，动态分配与释放资源，提高系统的效率和稳定性。
*/

#define CALLBACK_POOL_SIZE 100        // 定义回调池大小
static CallbackEntry callbackPool[CALLBACK_POOL_SIZE];
static CallbackEntry *freeCallbackList = NULL;

// 初始化内存池
void initCallbackMemoryPool(void) {
    freeCallbackList = &callbackPool[0];
    for(int i = 0; i < CALLBACK_POOL_SIZE - 1; i++) {
        callbackPool[i].next = &callbackPool[i + 1];
    }
    callbackPool[CALLBACK_POOL_SIZE - 1].next = NULL;
}

// 分配一个回调条目
CallbackEntry* allocateCallbackEntry(void) {
    if(freeCallbackList == NULL) {
        return NULL; // 内存池耗尽
    }
    CallbackEntry *entry = freeCallbackList;
    freeCallbackList = freeCallbackList->next;
    entry->next = NULL;
    return entry;
}

// 释放一个回调条目回内存池
void freeCallbackEntry(CallbackEntry *entry) {
    entry->next = freeCallbackList;
    freeCallbackList = entry;
}

#define MAX_EVENTS 10                  // 定义最大事件数量
static CallbackEntry *eventCallbacks[MAX_EVENTS];  // 每个事件的回调链表

// 初始化回调系统
void initCallbackSystem(void) {
    initCallbackMemoryPool();
    for(int i = 0; i < MAX_EVENTS; i++) {
        eventCallbacks[i] = NULL;
    }
}

// 注册回调函数，带优先级
int registerCallback(int eventID, CallbackFunc cb, void *param, int priority) {
    if(eventID < 0 || eventID >= MAX_EVENTS) {
        return -1; // 无效的事件ID
    }
    CallbackEntry *newEntry = allocateCallbackEntry();
    if(newEntry == NULL) {
        return -1; // 内存池耗尽
    }
    newEntry->callback = cb;
    newEntry->param = param;
    newEntry->priority = priority;
    newEntry->next = NULL;

    // 按优先级从高到低插入回调链表
    if(eventCallbacks[eventID] == NULL || eventCallbacks[eventID]->priority < priority) {
        newEntry->next = eventCallbacks[eventID];
        eventCallbacks[eventID] = newEntry;
    } else {
        CallbackEntry *current = eventCallbacks[eventID];
        while(current->next != NULL && current->next->priority >= priority) {
            current = current->next;
        }
        newEntry->next = current->next;
        current->next = newEntry;
    }

    return 0; // 注册成功
}

// 触发事件，按优先级执行回调
void triggerEvent(int eventID) {
    if(eventID < 0 || eventID >= MAX_EVENTS) {
        return; // 无效的事件ID
    }
    CallbackEntry *current = eventCallbacks[eventID];
    while(current != NULL) {
        if(current->callback != NULL) {
            current->callback(current->param);
        }
        current = current->next;
    }
}

void callbackA(void *param) {
    printf("Callback A executed with param: %p\r\n", param);
}

void callbackB(void *param) {
    printf("Callback B executed with param: %p\r\n", param);
}

void callbackC(void *param) {
    printf("Callback C executed with param: %p\r\n", param);
}

