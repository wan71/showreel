#ifndef __CALLBACK_H
#define __CALLBACK_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// 定义回调函数类型
typedef void (*CallbackFunc)(void *param);

// 带优先级的回调条目结构
typedef struct CallbackEntry {
    CallbackFunc callback;          // 回调函数
    void *param;                    // 回调参数
    int priority;                   // 回调优先级
    struct CallbackEntry *next;     // 链表指针
} CallbackEntry;


void callbackA(void *param);
void callbackB(void *param);
void callbackC(void *param);
void triggerEvent(int eventID);
int registerCallback(int eventID, CallbackFunc cb, void *param, int priority);
void initCallbackSystem(void);
#endif