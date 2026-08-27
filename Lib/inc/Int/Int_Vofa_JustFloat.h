/**
 * @file    Int_Vofa_JustFloat.h
 * @brief   VOFA+ JustFloat 协议驱动 —— Int(驱动)层（HAL 库）
 * @author  Assistant
 * @version 1.0
 * @date    2026-08-23
 * @note    适用于 STM32F103C8T6 及所有 HAL 库的 STM32
 *          小端模式 MCU 无需字节交换
 */

#ifndef __INT_VOFA_JUSTFLOAT_H
#define __INT_VOFA_JUSTFLOAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>
#include <string.h>
#include "usart.h"
#include "gpio.h"

/* ======================== 用户配置区 ======================== */

/**
 * @brief  指定 VOFA 使用的 UART 句柄
 * @note   在 CubeMX 生成的 main.c 中找到对应的 huartx 并填入
 */
#ifndef VOFA_UART
#define VOFA_UART   huart1
#endif

/**
 * @brief  最大支持的通道数（决定内部缓冲区大小）
 */
#ifndef VOFA_MAX_CHANNEL
#define VOFA_MAX_CHANNEL    8
#endif

/**
 * @brief  是否启用 DMA 发送（0=关闭，1=开启）
 * @note   开启后需要确保 CubeMX 中配置了对应 UART 的 DMA TX
 */
#ifndef VOFA_USE_DMA
#define VOFA_USE_DMA        1
#endif

/**
 * @brief  是否启用双缓冲（仅在 DMA 模式下有效）
 * @note   开启后可在 DMA 传输期间准备下一帧数据，避免等待
 */
#ifndef VOFA_USE_DOUBLE_BUF
#define VOFA_USE_DOUBLE_BUF 1
#endif

/* ======================== 接口函数 ======================== */

/**
 * @brief  初始化 VOFA JustFloat 模块
 * @note   必须在 UART 初始化完成后调用
 */
void VOFA_Init(void);

/**
 * @brief  发送一帧 JustFloat 数据（阻塞模式）
 * @param  data   float 数组指针
 * @param  count  通道数（1 ~ VOFA_MAX_CHANNEL）
 * @retval 0:成功  1:参数错误
 */
uint8_t VOFA_SendBlock(float *data, uint8_t count);

#if (VOFA_USE_DMA == 1)

/**
 * @brief  发送一帧 JustFloat 数据（DMA 非阻塞模式）
 * @param  data   float 数组指针
 * @param  count  通道数（1 ~ VOFA_MAX_CHANNEL）
 * @retval 0:成功启动  1:参数错误  2:DMA 正忙（上一帧未发完）
 * @note   若开启双缓冲，会自动切换缓冲区；否则需等待发完再调用
 */
uint8_t VOFA_SendDMA(float *data, uint8_t count);

/**
 * @brief  查询 DMA 是否空闲
 * @retval 0:忙  1:空闲
 */
uint8_t VOFA_IsDMAIdle(void);

/**
 * @brief  必须在 HAL_UART_TxCpltCallback 中调用
 * @param  huart  UART 句柄
 */
void VOFA_TxCpltCallback(UART_HandleTypeDef *huart);

#endif /* VOFA_USE_DMA */

#ifdef __cplusplus
}
#endif

#endif /* __INT_VOFA_JUSTFLOAT_H */