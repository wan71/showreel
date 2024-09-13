#include "sys_time.h"
#include <string.h>
int sys_tick = 0;



/**
 * @brief ϵͳʱ��
 * 
 */
void Sys_Tick_Count(void)
{
    sys_tick += 1;
	  
}



/**
 * @description: ��ȡϵͳ�δ��ʱ
 * @param {*}
 * @return {*}
 */
unsigned short int Get_Sys_Tick()
{
    return sys_tick;
}

/**
 * @description: �ж��Ƿ�ʱ
 * @param {unsigned long int} start ���㿪ʼ��ʱ��
 * @param {unsigned long int} timeout   ��ʱʱ��
 * @return {*}
 */
unsigned short int Is_Timeout(unsigned short int start, unsigned short int timeout)
{
    return ((unsigned short int)(Get_Sys_Tick() - start)) > timeout ?  1: 0;
}


