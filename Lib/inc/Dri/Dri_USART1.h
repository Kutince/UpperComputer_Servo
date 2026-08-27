#ifndef __DRI_USART1_H__
#define __DRI_USART1_H__

#include "main.h"
#include "usart.h"

/**
 * @brief UART 接收数据结构体
 */ 

#define RX_BUF_SIZE 64

typedef struct {
    uint16_t RxLen;              // 实际接收到的字节数
    uint8_t  RxData[RX_BUF_SIZE]; // 最终存放接收数据的数组
    uint8_t  RxTemp[RX_BUF_SIZE]; // DMA 接收临时缓存
} UART_RxTypeDef;

extern UART_RxTypeDef myUSART1;

/**
 * @brief  启动DMA接收，并关闭半传输中断
 * @note   该函数应在主程序初始化阶段调用一次
 */

void USART1_DMA_Init(void);

/**
 * @brief  将接收到的数据存入缓存区后清楚接收标志，并重启DMA接收
 */

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size);

/**
 * @brief  获取 USART1 接收数据
 * @param  buf: 指向接收数据的缓冲区
 * @param  len: 指向接收数据长度的指针
 * @param  max_len: 目标缓冲区 buf 的最大容量（防止越界写）
 * @retval 1: 成功获取数据，0: 无新数据
 */

uint8_t USART1_GetRxData(uint8_t *buf, uint16_t *len, uint16_t max_len);

/**
 * @brief  发送完成回调函数
 */

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);

#endif /* __DRI_USART1_H__ */