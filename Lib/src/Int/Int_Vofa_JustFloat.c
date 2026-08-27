/**
 * @file    Int_Vofa_JustFloat.c
 * @brief   VOFA+ JustFloat 协议驱动实现 —— Int(驱动)层
 */

#include "Int_Vofa_JustFloat.h"

/* ======================== 私有宏 ======================== */

/* JustFloat 帧尾：0x00 0x00 0x80 0x7F （对应 float +inf） */
static const uint8_t kJustFloatTail[4] = {0x00, 0x00, 0x80, 0x7F};

/* 一帧最大字节数 = 数据 + 帧尾 */
#define VOFA_MAX_FRAME_SIZE     (VOFA_MAX_CHANNEL * 4 + 4)

/* ======================== 私有变量 ======================== */

extern UART_HandleTypeDef VOFA_UART;

/* App 层通道数据缓存（在 App 层定义） */
extern float s_vofa_channels[VOFA_MAX_CHANNEL];

#if (VOFA_USE_DMA == 1)

/* 发送缓冲区 */
#if (VOFA_USE_DOUBLE_BUF == 1)
static uint8_t s_tx_buf[2][VOFA_MAX_FRAME_SIZE];
static volatile uint8_t s_buf_index = 0;    /* 当前写入的缓冲区索引 */
#else
static uint8_t s_tx_buf[VOFA_MAX_FRAME_SIZE];
#endif

static volatile uint8_t s_dma_busy = 0;     /* DMA 发送状态标志 */

#endif /* VOFA_USE_DMA */

/* ======================== 私有函数 ======================== */

/**
 * @brief  将 float 数据打包到字节缓冲区
 * @param  dst    目标缓冲区
 * @param  data   float 数组
 * @param  count  通道数
 * @return 打包后的总字节数
 */
static uint16_t VOFA_PackFrame(uint8_t *dst, float *data, uint8_t count)
{
    /* STM32F103 是小端模式，float 内存布局直接拷贝即可 */
    memcpy(dst, data, count * 4);
    memcpy(dst + count * 4, kJustFloatTail, 4);
    return (count * 4 + 4);
}

/* ======================== 公有函数 ======================== */

/**
 * @brief  初始化
 */
void VOFA_Init(void)
{
    memset(s_vofa_channels, 0, sizeof(float) * VOFA_MAX_CHANNEL);
    
#if (VOFA_USE_DMA == 1)
#if (VOFA_USE_DOUBLE_BUF == 1)
    memset(s_tx_buf[0], 0, sizeof(s_tx_buf[0]));
    memset(s_tx_buf[1], 0, sizeof(s_tx_buf[1]));
    s_buf_index = 0;
#else
    memset(s_tx_buf, 0, sizeof(s_tx_buf));
#endif
    s_dma_busy = 0;
#endif
}

/**
 * @brief  阻塞发送
 */
uint8_t VOFA_SendBlock(float *data, uint8_t count)
{
    uint8_t temp_buf[VOFA_MAX_FRAME_SIZE];
    uint16_t len;

    if (count == 0 || count > VOFA_MAX_CHANNEL)
    {
        return 1;
    }

    len = VOFA_PackFrame(temp_buf, data, count);
    HAL_UART_Transmit(&VOFA_UART, temp_buf, len, HAL_MAX_DELAY);
    return 0;
}

#if (VOFA_USE_DMA == 1)

/**
 * @brief  DMA 非阻塞发送
 */
uint8_t VOFA_SendDMA(float *data, uint8_t count)
{
    uint16_t len;

    if (count == 0 || count > VOFA_MAX_CHANNEL)
    {
        return 1;
    }

    /* 若未开启双缓冲，检查 DMA 是否空闲 */
#if (VOFA_USE_DOUBLE_BUF == 0)
    if (s_dma_busy)
    {
        return 2;
    }
#endif

    /* 选择缓冲区并打包 */
#if (VOFA_USE_DOUBLE_BUF == 1)
    uint8_t idx = s_buf_index;
    s_buf_index ^= 1;   /* 切换缓冲区 */
    len = VOFA_PackFrame(s_tx_buf[idx], data, count);
#else
    len = VOFA_PackFrame(s_tx_buf, data, count);
#endif

    s_dma_busy = 1;
    
#if (VOFA_USE_DOUBLE_BUF == 1)
    HAL_UART_Transmit_DMA(&VOFA_UART, s_tx_buf[idx], len);
#else
    HAL_UART_Transmit_DMA(&VOFA_UART, s_tx_buf, len);
#endif

    return 0;
}

/**
 * @brief  查询 DMA 状态
 */
uint8_t VOFA_IsDMAIdle(void)
{
    return (s_dma_busy == 0) ? 1 : 0;
}

/**
 * @brief  发送完成回调（必须放到 HAL_UART_TxCpltCallback 中）
 */
void VOFA_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == VOFA_UART.Instance)
    {
        s_dma_busy = 0;
    }
}

#endif /* VOFA_USE_DMA */