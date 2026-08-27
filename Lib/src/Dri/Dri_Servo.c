#include "Dri_Servo.h"
#include "Dri_USART1.h"
#include "stm32f1xx_hal_tim.h"
#include "stm32f1xx_hal_uart.h"
#include "tim.h"
#include <stdint.h>
#include <stdlib.h>
uint8_t rxBuf[4] = {0};
uint16_t rxLen=0;
char Err[] = "Error Number";
uint16_t ccr=500;
int Angle = 0;   /* 定义全局角度变量（对应 Dri_Servo.h 中的 extern 声明） */

/**
 *@brief 检查输入是否正确
 */

static uint8_t IsNewDataAvailable(void)
{
   for(int i = 0; i < rxLen; i++)
   {
       if(rxBuf[i]<'0' || rxBuf[i]>'9')  // 检查是否为数字字符
       {
           return 0;  // 如果有非数字字符，返回 0
       }
   }
   return 1;  // 如果所有字符都是数字，返回 1
}

/**
 *@brief 从串口接收角度值并转换为int类型
 *@retval 角度值
 */ 


void Dri_Servo_GetData(void) 
{
    /* 无新数据时直接返回，避免无意义地发送错误提示 */
    if (USART1_GetRxData(rxBuf, &rxLen, sizeof(rxBuf)) == 0)
    {
        return;
    }
    rxBuf[rxLen] = '\0';  // 确保字符串以 null 结尾
    if(rxLen>=1 && rxLen<=3)
    {
        if(IsNewDataAvailable())
        {
            Angle = atoi((char*)rxBuf);  // 将接收到的字符串转换为整数
        }
        else
        {
           HAL_UART_Transmit_IT(&huart1, (const uint8_t *)Err, sizeof(Err) - 1);
        }
    }
    else
    {
        HAL_UART_Transmit_IT(&huart1, (const uint8_t *)Err, sizeof(Err) - 1);
    }
}

int Dri_Servo_ProcessData(void)
{
    ccr=500+Angle*(2000/180);//PSC=71 ARR=19999
    return ccr;
}

void Dri_Servo_TimSet(uint16_t ccrx)
{
    __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1, ccrx);
}