#ifndef __Dri_OLED_H__
#define __Dri_OLED_H__

#include "main.h"
#include "stdint.h"
#include "string.h"

/* ========================== 硬件参数 ========================== */

/** @brief OLED I2C 从机地址（写模式） */
#define OLED_ADDRESS 0x78

/** @brief OLED 总页数（64 行 / 8 = 8 页） */
#define OLED_PAGE 8

/** @brief OLED 总行数 */
#define OLED_ROW (8 * OLED_PAGE)

/** @brief OLED 总列数 */
#define OLED_COLUMN 128

/* ========================== 数据类型 ========================== */

/**
 * @brief OLED 颜色模式枚举
 * @note  NORMAL  ：黑底白字（像素点亮，bit = 1）
 * @note  REVERSED：白底黑字（像素点灭，bit = 0）
 */
typedef enum {
    OLED_COLOR_NORMAL = 0,
    OLED_COLOR_REVERSED
} OLED_ColorMode;

/* ========================== 显存声明 ========================== */

/** @brief 显存数组：8 页 × 128 列，对应 SSD1306 的 GDDRAM */
extern uint8_t OLED_GRAM[OLED_PAGE][OLED_COLUMN];

/* ========================== 硬件通信接口 ========================== */

/**
 * @brief 向 OLED 发送原始数据
 * @param data 数据缓冲区首地址
 * @param len  待发送字节数
 * @note  本函数为平台移植关键接口，当前基于 STM32 HAL I2C 实现。
 *        更换平台时只需重写此函数。
 */
void OLED_Send(uint8_t *data, uint8_t len);

/**
 * @brief 向 SSD1306 发送单条控制指令
 * @param cmd 指令码
 * @note  自动附加 0x00 命令模式前缀。
 */
void OLED_SendCmd(uint8_t cmd);

/* ========================== 初始化与电源管理 ========================== */

/**
 * @brief 初始化 SSD1306 OLED 控制器
 * @note  建议 STM32 上电后延时 20ms 再调用，确保 OLED 复位完成。
 *        初始化序列包括：寻址模式、电荷泵、对比度、显示开关等。
 */
void OLED_Init(void);

/** @brief 开启 OLED 显示（使能电荷泵并点亮屏幕） */
void OLED_DisPlay_On(void);

/** @brief 关闭 OLED 显示（关闭电荷泵以降低功耗） */
void OLED_DisPlay_Off(void);

/**
 * @brief 设置全局显示颜色模式
 * @param mode NORMAL 或 REVERSED
 * @note  直接修改 SSD1306 显示模式寄存器，影响整屏。
 */
void OLED_SetColorMode(OLED_ColorMode mode);

/* ========================== 帧缓冲管理 ========================== */

/** @brief 清空显存，准备绘制新一帧 */
void OLED_NewFrame(void);

/**
 * @brief 将显存内容刷新到物理屏幕
 * @note  按页为单位通过 I2C 发送，每页 128 字节像素数据。
 */
void OLED_ShowFrame(void);

/* ========================== 像素与显存操作 ========================== */

/**
 * @brief 设置单个像素点
 * @param x 列坐标（0 ~ 127）
 * @param y 行坐标（0 ~ 63）
 * @param color 颜色模式
 * @note  越界坐标自动丢弃。NORMAL 模式置位，REVERSED 模式清零。
 */
void OLED_SetPixel(uint8_t x, uint8_t y, OLED_ColorMode color);

/**
 * @brief 精细修改显存某字节的指定位段
 * @param page   页地址（0 ~ 7）
 * @param column 列地址（0 ~ 127）
 * @param data   原始数据
 * @param start  起始位（0 ~ 7，LSB 为 bit0）
 * @param end    结束位（0 ~ 7，必须 >= start）
 * @param color  颜色模式
 * @note  仅修改 [start, end] 位，其余位保持不变；REVERSED 时数据自动取反。
 */
void OLED_SetByte_Fine(uint8_t page, uint8_t column, uint8_t data, uint8_t start, uint8_t end, OLED_ColorMode color);

/**
 * @brief 覆盖显存中的整字节
 * @param page   页地址（0 ~ 7）
 * @param column 列地址（0 ~ 127）
 * @param data   8 位数据
 * @param color  颜色模式
 */
void OLED_SetByte(uint8_t page, uint8_t column, uint8_t data, OLED_ColorMode color);

/**
 * @brief 从像素坐标开始向下写入最多 8 位（可跨页）
 * @param x     起始列坐标
 * @param y     起始行坐标
 * @param data  源数据（低位对应上方像素）
 * @param len   有效位数（1 ~ 8）
 * @param color 颜色模式
 * @note  若跨越页边界，自动拆分到相邻两个物理字节。
 */
void OLED_SetBits_Fine(uint8_t x, uint8_t y, uint8_t data, uint8_t len, OLED_ColorMode color);

/**
 * @brief 从像素坐标开始向下写入 8 位（可跨页）
 * @param x     起始列坐标
 * @param y     起始行坐标
 * @param data  8 位源数据
 * @param color 颜色模式
 */
void OLED_SetBits(uint8_t x, uint8_t y, uint8_t data, OLED_ColorMode color);

/**
 * @brief 在指定区域写入一块位图数据
 * @param x     起始列坐标
 * @param y     起始行坐标
 * @param data  位图数据指针（列行式排列）
 * @param w     位图宽度（像素）
 * @param h     位图高度（像素）
 * @param color 颜色模式
 * @note  数据格式：每列自上而下，每 8 行打包为 1 字节。
 *        适用于字体、图片等资源的批量写入。
 */
void OLED_SetBlock(uint8_t x, uint8_t y, const uint8_t *data, uint8_t w, uint8_t h, OLED_ColorMode color);

#endif /* __Dri_OLED_H__ */