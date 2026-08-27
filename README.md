[上位机调试.md](https://github.com/user-attachments/files/31522896/default.md)
# 上位机调试

## VOFA+ JustFloat协议的学习

从[VOFA+官方JustFloat介绍文本](https://www.vofa.plus/plugin_detail/?name=justfloat)中了解到了该协议的基本用法后，我通过**Kimi2.6**生成了该协议的驱动文件并且了解了其**双缓冲DMA非阻塞**串口发送模式的运作方式

```c
/**
 * @file    vofa_justfloat.c
 * @brief   VOFA+ JustFloat 协议驱动实现
 */

#include "vofa_justfloat.h"

/* ======================== 私有宏 ======================== */

/* JustFloat 帧尾：0x00 0x00 0x80 0x7F （对应 float +inf） */
static const uint8_t kJustFloatTail[4] = {0x00, 0x00, 0x80, 0x7F};

/* 一帧最大字节数 = 数据 + 帧尾 */
#define VOFA_MAX_FRAME_SIZE     (VOFA_MAX_CHANNEL * 4 + 4)

/* ======================== 私有变量 ======================== */

extern UART_HandleTypeDef VOFA_UART;

/* 内部通道数据缓存（用于先填充再发送） */
static float s_vofa_channels[VOFA_MAX_CHANNEL];

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
    memset(s_vofa_channels, 0, sizeof(s_vofa_channels));
    
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
```

### static uint16_t VOFA_PackFrame(uint8_t *dst, float *data, uint8_t count)

```c
static uint16_t VOFA_PackFrame(uint8_t *dst, float *data, uint8_t count)
{
    /* STM32F103 是小端模式，float 内存布局直接拷贝即可 */
    memcpy(dst, data, count * 4);
    memcpy(dst + count * 4, kJustFloatTail, 4);
    return (count * 4 + 4);
}

```

在该内部函数中，实现了将目标数据存入缓冲区，并添加JustFloat结尾帧的功能

### void VOFA_Init(void)

```c
void VOFA_Init(void)
{
    memset(s_vofa_channels, 0, sizeof(s_vofa_channels));
    
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
```

此为初始化函数，清零数组中的数据，有效避免了初始上电时生成随机数据，导致VOFA+乱码

### uint8_t VOFA_SendBlock(float *data, uint8_t count)

```c
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
```

此为阻塞式发送方式，个人认为大部分情况下不会用到该方式发送数据，仅用于初步验证

### uint8_t VOFA_SendDMA(float *data, uint8_t count)

```c
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
```

该函数的作用是把一组 float 数据打包成 **JustFloat 协议帧**，然后通过 **DMA 非阻塞方式**发送出去。

首先校验通道参数**count是否设置正确**，第二步检查**DMA是否处于空闲状态**，第三步**选择缓冲区并打包数据**，第四步**标记忙状态并启动 DMA**

```c
#if (VOFA_USE_DOUBLE_BUF == 1)
    uint8_t idx = s_buf_index;
    s_buf_index ^= 1;   /* 切换缓冲区 */
    len = VOFA_PackFrame(s_tx_buf[idx], data, count);
#else
    len = VOFA_PackFrame(s_tx_buf, data, count);
#endif
```

| 操作                  | 含义                                       |
| --------------------- | ------------------------------------------ |
| `s_buf_index ^= 1`    | 异或 `1` 实现 `0↔1` 切换，即双缓冲乒乓操作 |
| `VOFA_PackFrame(...)` | 把 float 数组 + 帧尾打包到目标缓冲区       |
| `len`                 | 打包后的总字节数（`count * 4 + 4`）        |

```c
    s_dma_busy = 1;
    
#if (VOFA_USE_DOUBLE_BUF == 1)
    HAL_UART_Transmit_DMA(&VOFA_UART, s_tx_buf[idx], len);
#else
    HAL_UART_Transmit_DMA(&VOFA_UART, s_tx_buf, len);
#endif

    return 0;
```

| 操作                      | 说明                                               |
| ------------------------- | -------------------------------------------------- |
| `s_dma_busy = 1`          | 标记"正在发送"，防止单缓冲时重复调用               |
| `HAL_UART_Transmit_DMA()` | HAL 库函数，启动 UART DMA 发送，**立即返回**不等待 |
| `return 0`                | 告诉调用方"已成功提交给 DMA"                       |

## JustFloat首次实验现象

![image-20260823211955145](C:\Users\Kutince\AppData\Roaming\Typora\typora-user-images\image-20260823211955145.png)

```c
    VOFA_SetChannel(1, 3.14f);
    VOFA_SetChannel(2, sinf(HAL_GetTick() / 500.0f));
```

成功在VOFA+中观测到正弦波和3.14常数

## 舵机控制

### sg90控制原理

**180°**版本的sg90电机在脉冲时间为**0.5ms~2.5ms**脉冲时间的PWM脉冲下工作

### PWM波的产生



1. 使能TIM1的channel1的PWM生成通道
2. 分别将PSC设置为71，ARR设置为19999

如此一来，每个PWM波的

**f=72,000,000 / {(19999+1) x (71+1)}=50Hz**

**T=1 / 50=0.02s=20ms**

每个PWM波的脉冲时间取决于PWM的占空比

由于

- **duty=ccr/arr**
- **脉冲时间=T x duty**

所以

**ccr=脉冲时间 / T x arr**

由此可知ccr的范围是500~2500，与输入的角度值（0 ~ 180）**成正比例关系**。

### 串口接收程序设计

#### 串口接收

在串口接收中我使用了DMA空闲接收，并关闭了DMA串口接收过半中断以保证接收的完整性

```c
void USART1_DMA_Init(void)
{
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, myUSART1.RxTemp, RX_BUF_SIZE);
    __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);  // 关闭半传输中断
}
```

此外，为了使接收到的数据稳定，我使用了双缓冲设计

```c
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
```

在舵机的驱动函数中，我调用了数据接收函数，通过atio函数来实现了unsigned char类型的角度值转存为int类型，还加入了角度值输入错误时报错的

```c
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
```

角度值与ccr互相转换函数

```c
int Dri_Servo_ProcessData(void)
{
    ccr=500+Angle*(2000/180);//PSC=71 ARR=19999
    return ccr;
}
```

最后在应用层中添加舵机控制逻辑，将算出的ccr值传入，实现输入角度控制舵机转向功能

```c
void App_ServoContorl(void)
{
    Dri_Servo_GetData();
    Dri_Servo_TimSet(Dri_Servo_ProcessData());
}
```

