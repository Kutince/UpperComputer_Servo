/**
 * @file oled_int.c
 * @brief OLED 驱动层 —— 硬件抽象与显存管理
 * @details 负责 SSD1306 的 I2C 通信、初始化、显存读写及基础像素操作。
 *          移植到新平台时，通常只需修改 OLED_Send 函数。
 */

#include "Dri_OLED.h"
#include "i2c.h"

/* 显存定义：8 页 × 128 列 */
uint8_t OLED_GRAM[OLED_PAGE][OLED_COLUMN];

/**
 * @brief 向 OLED 发送原始数据
 * @param data 数据缓冲区指针
 * @param len  数据长度（字节）
 */
void OLED_Send(uint8_t *data, uint8_t len)
{
    HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDRESS, data, len, HAL_MAX_DELAY);
}

/**
 * @brief 向 OLED 发送单条控制指令
 * @param cmd SSD1306 指令码
 */
void OLED_SendCmd(uint8_t cmd)
{
    static uint8_t sendBuffer[2] = {0};
    sendBuffer[1] = cmd;
    OLED_Send(sendBuffer, 2);
}

/**
 * @brief 初始化 SSD1306
 * @note  上电后建议延时 20ms 再调用。
 */
void OLED_Init(void)
{
    OLED_SendCmd(0xAE); /* 关闭显示 Display OFF */

    OLED_SendCmd(0x20); OLED_SendCmd(0x10); /* 水平寻址模式 */
    OLED_SendCmd(0xB0); /* 设置页起始地址 */
    OLED_SendCmd(0xC8); /* COM 扫描方向：从下到上 */
    OLED_SendCmd(0x00); OLED_SendCmd(0x10); /* 设置列地址低/高四位 */
    OLED_SendCmd(0x40); /* 设置显示起始行 */
    OLED_SendCmd(0x81); OLED_SendCmd(0xDF); /* 对比度设置 */
    OLED_SendCmd(0xA1); /* 段重映射 */
    OLED_SendCmd(0xA6); /* 正常显示（非反色） */
    OLED_SendCmd(0xA8); OLED_SendCmd(0x3F); /* 多路复用率 1/64 Duty */
    OLED_SendCmd(0xA4); /* 全局显示开启：使用 GDDRAM 内容 */
    OLED_SendCmd(0xD3); OLED_SendCmd(0x00); /* 显示偏移 0 */
    OLED_SendCmd(0xD5); OLED_SendCmd(0xF0); /* 时钟分频与振荡频率 */
    OLED_SendCmd(0xD9); OLED_SendCmd(0x22); /* 预充电周期 */
    OLED_SendCmd(0xDA); OLED_SendCmd(0x12); /* COM 引脚硬件配置 */
    OLED_SendCmd(0xDB); OLED_SendCmd(0x20); /* VCOMH 取消选择电平 */
    OLED_SendCmd(0x8D); OLED_SendCmd(0x14); /* 电荷泵使能 */

    OLED_NewFrame();  /* 清显存 */
    OLED_ShowFrame(); /* 刷新到屏幕 */

    OLED_SendCmd(0xAF); /* 开启显示 Display ON */
}

/**
 * @brief 开启 OLED 显示
 */
void OLED_DisPlay_On(void)
{
    OLED_SendCmd(0x8D); /* 电荷泵使能 */
    OLED_SendCmd(0x14); /* 开启电荷泵 */
    OLED_SendCmd(0xAF); /* 点亮屏幕 */
}

/**
 * @brief 关闭 OLED 显示
 */
void OLED_DisPlay_Off(void)
{
    OLED_SendCmd(0x8D); /* 电荷泵使能 */
    OLED_SendCmd(0x10); /* 关闭电荷泵 */
    OLED_SendCmd(0xAE); /* 关闭屏幕 */
}

/**
 * @brief 设置全局颜色模式
 * @param mode 颜色模式
 */
void OLED_SetColorMode(OLED_ColorMode mode)
{
    if (mode == OLED_COLOR_NORMAL)
    {
        OLED_SendCmd(0xA6); /* 正常显示 */
    }
    if (mode == OLED_COLOR_REVERSED)
    {
        OLED_SendCmd(0xA7); /* 反色显示 */
    }
}

/**
 * @brief 清空显存
 */
void OLED_NewFrame(void)
{
    memset(OLED_GRAM, 0, sizeof(OLED_GRAM));
}

/**
 * @brief 将显存刷新到屏幕
 */
void OLED_ShowFrame(void)
{
    static uint8_t sendBuffer[OLED_COLUMN + 1];
    sendBuffer[0] = 0x40; /* 数据模式控制字节 */
    for (uint8_t i = 0; i < OLED_PAGE; i++)
    {
        OLED_SendCmd(0xB0 + i); /* 设置页地址 */
        OLED_SendCmd(0x00);     /* 设置列低地址 */
        OLED_SendCmd(0x10);     /* 设置列高地址 */
        memcpy(sendBuffer + 1, OLED_GRAM[i], OLED_COLUMN);
        OLED_Send(sendBuffer, OLED_COLUMN + 1);
    }
}

/**
 * @brief 设置单个像素点
 */
void OLED_SetPixel(uint8_t x, uint8_t y, OLED_ColorMode color)
{
    if (x >= OLED_COLUMN || y >= OLED_ROW)
        return;
    if (!color)
    {
        OLED_GRAM[y / 8][x] |= 1 << (y % 8);
    }
    else
    {
        OLED_GRAM[y / 8][x] &= ~(1 << (y % 8));
    }
}

/**
 * @brief 精细设置显存中某一字节的位段
 */
void OLED_SetByte_Fine(uint8_t page, uint8_t column, uint8_t data, uint8_t start, uint8_t end, OLED_ColorMode color)
{
    static uint8_t temp;
    if (page >= OLED_PAGE || column >= OLED_COLUMN)
        return;
    if (color)
        data = ~data;

    temp = data | (0xff << (end + 1)) | (0xff >> (8 - start));
    OLED_GRAM[page][column] &= temp;
    temp = data & ~(0xff << (end + 1)) & ~(0xff >> (8 - start));
    OLED_GRAM[page][column] |= temp;
}

/**
 * @brief 设置显存中的整字节
 */
void OLED_SetByte(uint8_t page, uint8_t column, uint8_t data, OLED_ColorMode color)
{
    if (page >= OLED_PAGE || column >= OLED_COLUMN)
        return;
    if (color)
        data = ~data;
    OLED_GRAM[page][column] = data;
}

/**
 * @brief 从像素坐标开始向下写入最多 8 位（可跨页）
 */
void OLED_SetBits_Fine(uint8_t x, uint8_t y, uint8_t data, uint8_t len, OLED_ColorMode color)
{
    uint8_t page = y / 8;
    uint8_t bit = y % 8;
    if (bit + len > 8)
    {
        OLED_SetByte_Fine(page, x, data << bit, bit, 7, color);
        OLED_SetByte_Fine(page + 1, x, data >> (8 - bit), 0, len + bit - 1 - 8, color);
    }
    else
    {
        OLED_SetByte_Fine(page, x, data << bit, bit, bit + len - 1, color);
    }
}

/**
 * @brief 从像素坐标开始向下写入 8 位（可跨页）
 */
void OLED_SetBits(uint8_t x, uint8_t y, uint8_t data, OLED_ColorMode color)
{
    uint8_t page = y / 8;
    uint8_t bit = y % 8;
    OLED_SetByte_Fine(page, x, data << bit, bit, 7, color);
    if (bit)
    {
        OLED_SetByte_Fine(page + 1, x, data >> (8 - bit), 0, bit - 1, color);
    }
}

/**
 * @brief 在指定区域写入一块位图数据
 */
void OLED_SetBlock(uint8_t x, uint8_t y, const uint8_t *data, uint8_t w, uint8_t h, OLED_ColorMode color)
{
    uint8_t fullRow = h / 8; /* 完整字节行数 */
    uint8_t partBit = h % 8; /* 尾部有效位数 */
    for (uint8_t i = 0; i < w; i++)
    {
        for (uint8_t j = 0; j < fullRow; j++)
        {
            OLED_SetBits(x + i, y + j * 8, data[i + j * w], color);
        }
    }
    if (partBit)
    {
        uint16_t fullNum = w * fullRow;
        for (uint8_t i = 0; i < w; i++)
        {
            OLED_SetBits_Fine(x + i, y + (fullRow * 8), data[fullNum + i], partBit, color);
        }
    }
}