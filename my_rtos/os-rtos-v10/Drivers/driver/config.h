#ifndef __CONFIG_H
#define __CONFIG_H
#include "stm32f1xx_hal.h"
#include "cpu.h"

#define TASK_CPU 0  // �������ã��Ƿ�鿴CPU��ռ�þ���
#define Critical_MODE 1 //1�ǹ��жϣ�2��ԭ�Ӳ���
#define STACK_Control 1	//��ʾջ��ʵ��ʹ�ô�С��С
#define MONITOR_logRealtime 0 //���ʵʱ��
#define MonitorTask 0    //�����Ź�
#endif