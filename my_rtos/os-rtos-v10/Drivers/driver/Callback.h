#ifndef __CALLBACK_H
#define __CALLBACK_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// ����ص���������
typedef void (*CallbackFunc)(void *param);

// �����ȼ��Ļص���Ŀ�ṹ
typedef struct CallbackEntry {
    CallbackFunc callback;          // �ص�����
    void *param;                    // �ص�����
    int priority;                   // �ص����ȼ�
    struct CallbackEntry *next;     // ����ָ��
} CallbackEntry;


void callbackA(void *param);
void callbackB(void *param);
void callbackC(void *param);
void triggerEvent(int eventID);
int registerCallback(int eventID, CallbackFunc cb, void *param, int priority);
void initCallbackSystem(void);
#endif