#ifndef __DRI_SERVO_H__
#define __DRI_SERVO_H__

#include "stdint.h"

/** 
 *@brief 角度值 
 */

extern int Angle;

/**
 *@brief 计算角度值对应的ccr
 *@retval ccr值
 */

int Dri_Servo_ProcessData(void);

/**
 *@brief 设置脉冲时间
 *@param ccrx：ccr的值
 */

void Dri_Servo_TimSet(uint16_t ccrx);

/**
 *@brief 从串口接收角度值判断合法性，并存为int类型变量
 */

void Dri_Servo_GetData(void);

#endif /* __DRI_SERVO_H__ */