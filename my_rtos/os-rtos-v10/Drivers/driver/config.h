#ifndef __CONFIG_H
#define __CONFIG_H
#include "stm32f1xx_hal.h"
#include "cpu.h"

#define TASK_CPU 0  // 调试配置，是否查看CPU的占用境况
#define Critical_MODE 1 //1是关中断，2是原子操作
#define STACK_Control 1	//显示栈的实际使用大小大小
#define MONITOR_logRealtime 0 //检测实时性
#define MonitorTask 0    //任务看门狗
#endif