/**
 * @file    Com_Vofa_JustFloat.c
 * @brief   VOFA+ JustFloat 协议驱动实现 —— Com(通信)层
 */

#include "Com_Vofa_JustFloat.h"

/* ======================== 私有变量 ======================== */

/* 内部通道数据缓存（用于先填充再发送） */
float s_vofa_channels[VOFA_MAX_CHANNEL];

/* ======================== 公有函数 ======================== */

/**
 * @brief  设置单个通道值
 */
void VOFA_SetChannel(uint8_t channel, float value)
{
    if (channel < VOFA_MAX_CHANNEL)
    {
        s_vofa_channels[channel] = value;
    }
}

/**
 * @brief  发送已填充的通道
 */
void VOFA_SendChannels(uint8_t count)
{
    if (count > VOFA_MAX_CHANNEL)
    {
        count = VOFA_MAX_CHANNEL;
    }
    
#if (VOFA_USE_DMA == 1)
    VOFA_SendDMA(s_vofa_channels, count);
#else
    VOFA_SendBlock(s_vofa_channels, count);
#endif
}