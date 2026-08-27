#include "Dri_USART1.h"
#include "Int_Vofa_JustFloat.h"
#include <string.h>

/** 
  * @brief  UART 接收数据结构体初始化
*/

UART_RxTypeDef myUSART1 = {0};

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == USART1)
    {
        myUSART1.RxLen = Size;
        memcpy(myUSART1.RxData, myUSART1.RxTemp, Size);
        if (myUSART1.RxLen < RX_BUF_SIZE)
        {
            myUSART1.RxData[myUSART1.RxLen] = '\0';  // 留出末尾空间，防止越界
        }
        // 重新启动 DMA
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, myUSART1.RxTemp, RX_BUF_SIZE);
        __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    VOFA_TxCpltCallback(huart);  /* 通知 VOFA 模块发送完成 */
}

void USART1_DMA_Init(void)
{
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, myUSART1.RxTemp, RX_BUF_SIZE);
    __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);  // 关闭半传输中断
}

uint8_t USART1_GetRxData(uint8_t *buf, uint16_t *len, uint16_t max_len)
{
    uint16_t copy_len;

    if (myUSART1.RxLen == 0)
    {
        return 0;
    }

    /* 限制拷贝长度，防止写入方缓冲区越界 */
    copy_len = myUSART1.RxLen;
    if (copy_len > max_len)
    {
        copy_len = max_len;
    }

    *len = copy_len;
    memcpy(buf, myUSART1.RxData, copy_len);
    myUSART1.RxLen = 0;  // 清零，表示已读取
    
    return 1;
}