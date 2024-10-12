#include "callback.h"

/*
��������ģ�黯���ص������������������ע���ض��¼��Ĵ�������ʹ�ò�ͬģ����Զ�������������������������Ŀ�ά���ԡ�

�첽����ͨ���ص����ƣ��������жϻ����������������Ӧ���������߼������������������첽ִ�У����ʵʱ�ԡ�

�¼��������ƣ�RTOSͨ������������첽�¼����ص�ʹ���¼��������ض������������Ⱥʹ���

�ڴ�����Դ�����ص�ϵͳ��������ڴ������ƣ���̬�������ͷ���Դ�����ϵͳ��Ч�ʺ��ȶ��ԡ�
*/

#define CALLBACK_POOL_SIZE 100        // ����ص��ش�С
static CallbackEntry callbackPool[CALLBACK_POOL_SIZE];
static CallbackEntry *freeCallbackList = NULL;

// ��ʼ���ڴ��
void initCallbackMemoryPool(void) {
    freeCallbackList = &callbackPool[0];
    for(int i = 0; i < CALLBACK_POOL_SIZE - 1; i++) {
        callbackPool[i].next = &callbackPool[i + 1];
    }
    callbackPool[CALLBACK_POOL_SIZE - 1].next = NULL;
}

// ����һ���ص���Ŀ
CallbackEntry* allocateCallbackEntry(void) {
    if(freeCallbackList == NULL) {
        return NULL; // �ڴ�غľ�
    }
    CallbackEntry *entry = freeCallbackList;
    freeCallbackList = freeCallbackList->next;
    entry->next = NULL;
    return entry;
}

// �ͷ�һ���ص���Ŀ���ڴ��
void freeCallbackEntry(CallbackEntry *entry) {
    entry->next = freeCallbackList;
    freeCallbackList = entry;
}

#define MAX_EVENTS 10                  // ��������¼�����
static CallbackEntry *eventCallbacks[MAX_EVENTS];  // ÿ���¼��Ļص�����

// ��ʼ���ص�ϵͳ
void initCallbackSystem(void) {
    initCallbackMemoryPool();
    for(int i = 0; i < MAX_EVENTS; i++) {
        eventCallbacks[i] = NULL;
    }
}

// ע��ص������������ȼ�
int registerCallback(int eventID, CallbackFunc cb, void *param, int priority) {
    if(eventID < 0 || eventID >= MAX_EVENTS) {
        return -1; // ��Ч���¼�ID
    }
    CallbackEntry *newEntry = allocateCallbackEntry();
    if(newEntry == NULL) {
        return -1; // �ڴ�غľ�
    }
    newEntry->callback = cb;
    newEntry->param = param;
    newEntry->priority = priority;
    newEntry->next = NULL;

    // �����ȼ��Ӹߵ��Ͳ���ص�����
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

    return 0; // ע��ɹ�
}

// �����¼��������ȼ�ִ�лص�
void triggerEvent(int eventID) {
    if(eventID < 0 || eventID >= MAX_EVENTS) {
        return; // ��Ч���¼�ID
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

