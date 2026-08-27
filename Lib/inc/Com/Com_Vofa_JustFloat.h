/**
 * @file    App_Vofa_JustFloat.h
 * @brief   VOFA+ JustFloat 协议驱动 —— App(应用)层
 * @author  Assistant
 * @version 1.0
 * @date    2026-08-23
 */

#ifndef __COM_VOFA_JUSTFLOAT_H
#define __COM_VOFA_JUSTFLOAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Int_Vofa_JustFloat.h"

/* ======================== 接口函数 ======================== */

/**
 * @brief  设置指定通道的数据值（用于先填充再统一发送的场景）
 * @param  channel 通道号（从 0 开始）
 * @param  value   float 值
 */
void VOFA_SetChannel(uint8_t channel, float value);

/**
 * @brief  发送当前已填充的所有通道数据
 * @param  count  要发送的通道数
 */
void VOFA_SendChannels(uint8_t count);

#ifdef __cplusplus
}
#endif

#endif /* __COM_VOFA_JUSTFLOAT_H */