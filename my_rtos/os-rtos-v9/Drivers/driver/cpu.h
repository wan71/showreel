#ifndef __CPU_H
#define __CPU_H
#include "stm32f1xx_hal.h"
#include "queue.h"

void PendSV_init(void);
void triggerPendSV();
void enterCritical(void);
void exitCritical(void);
void restart_pendsv();
uint32_t getSysTime_task(void);
#endif

