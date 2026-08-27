/**
 * @file oled_app.c
 * @brief OLED 应用层 —— 高级图形与文字绘制
 * @details 基于驱动层提供的显存接口（OLED_SetPixel / OLED_SetBlock），
 *          实现几何图形、图片及文字渲染，不直接操作硬件。
 */

#include "Com_OLED.h"
#include <math.h>
#include <stdlib.h>

/**
 * @brief 绘制线段（Bresenham 算法）
 */
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, OLED_ColorMode color)
{
    static uint8_t temp = 0;
    if (x1 == x2)
    {
        if (y1 > y2) { temp = y1; y1 = y2; y2 = temp; }
        for (uint8_t y = y1; y <= y2; y++)
            OLED_SetPixel(x1, y, color);
    }
    else if (y1 == y2)
    {
        if (x1 > x2) { temp = x1; x1 = x2; x2 = temp; }
        for (uint8_t x = x1; x <= x2; x++)
            OLED_SetPixel(x, y1, color);
    }
    else
    {
        int16_t dx = x2 - x1;
        int16_t dy = y2 - y1;
        int16_t ux = ((dx > 0) << 1) - 1;
        int16_t uy = ((dy > 0) << 1) - 1;
        int16_t x = x1, y = y1, eps = 0;
        dx = abs(dx);
        dy = abs(dy);
        if (dx > dy)
        {
            for (x = x1; x != x2; x += ux)
            {
                OLED_SetPixel(x, y, color);
                eps += dy;
                if ((eps << 1) >= dx) { y += uy; eps -= dx; }
            }
        }
        else
        {
            for (y = y1; y != y2; y += uy)
            {
                OLED_SetPixel(x, y, color);
                eps += dx;
                if ((eps << 1) >= dy) { x += ux; eps -= dy; }
            }
        }
    }
}

/**
 * @brief 绘制空心矩形
 */
void OLED_DrawRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, OLED_ColorMode color)
{
    OLED_DrawLine(x, y, x + w, y, color);
    OLED_DrawLine(x, y + h, x + w, y + h, color);
    OLED_DrawLine(x, y, x, y + h, color);
    OLED_DrawLine(x + w, y, x + w, y + h, color);
}

/**
 * @brief 绘制实心矩形
 */
void OLED_DrawFilledRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, OLED_ColorMode color)
{
    for (uint8_t i = 0; i < h; i++)
    {
        OLED_DrawLine(x, y + i, x + w, y + i, color);
    }
}

/**
 * @brief 绘制空心三角形
 */
void OLED_DrawTriangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3, OLED_ColorMode color)
{
    OLED_DrawLine(x1, y1, x2, y2, color);
    OLED_DrawLine(x2, y2, x3, y3, color);
    OLED_DrawLine(x3, y3, x1, y1, color);
}

/**
 * @brief 绘制实心三角形
 */
void OLED_DrawFilledTriangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3, OLED_ColorMode color)
{
    uint8_t a = 0, b = 0, y = 0, last = 0;
    if (y1 > y2) { a = y2; b = y1; } else { a = y1; b = y2; }
    y = a;
    for (; y <= b; y++)
    {
        if (y <= y3)
        {
            OLED_DrawLine(x1 + (y - y1) * (x2 - x1) / (y2 - y1), y,
                          x1 + (y - y1) * (x3 - x1) / (y3 - y1), y, color);
        }
        else { last = y - 1; break; }
    }
    for (; y <= b; y++)
    {
        OLED_DrawLine(x2 + (y - y2) * (x3 - x2) / (y3 - y2), y,
                      x1 + (y - last) * (x3 - x1) / (y3 - last), y, color);
    }
}

/**
 * @brief 绘制空心圆（Bresenham 算法）
 */
void OLED_DrawCircle(uint8_t x, uint8_t y, uint8_t r, OLED_ColorMode color)
{
    int16_t a = 0, b = r, di = 3 - (r << 1);
    while (a <= b)
    {
        OLED_SetPixel(x - b, y - a, color);
        OLED_SetPixel(x + b, y - a, color);
        OLED_SetPixel(x - a, y + b, color);
        OLED_SetPixel(x - b, y - a, color);
        OLED_SetPixel(x - a, y - b, color);
        OLED_SetPixel(x + b, y + a, color);
        OLED_SetPixel(x + a, y - b, color);
        OLED_SetPixel(x + a, y + b, color);
        OLED_SetPixel(x - b, y + a, color);
        a++;
        if (di < 0) { di += 4 * a + 6; }
        else { di += 10 + 4 * (a - b); b--; }
        OLED_SetPixel(x + a, y + b, color);
    }
}

/**
 * @brief 绘制实心圆（Bresenham 算法）
 */
void OLED_DrawFilledCircle(uint8_t x, uint8_t y, uint8_t r, OLED_ColorMode color)
{
    int16_t a = 0, b = r, di = 3 - (r << 1);
    while (a <= b)
    {
        for (int16_t i = x - b; i <= x + b; i++)
        {
            OLED_SetPixel(i, y + a, color);
            OLED_SetPixel(i, y - a, color);
        }
        for (int16_t i = x - a; i <= x + a; i++)
        {
            OLED_SetPixel(i, y + b, color);
            OLED_SetPixel(i, y - b, color);
        }
        a++;
        if (di < 0) { di += 4 * a + 6; }
        else { di += 10 + 4 * (a - b); b--; }
    }
}

/**
 * @brief 绘制空心椭圆（中点椭圆算法）
 */
void OLED_DrawEllipse(uint8_t x, uint8_t y, uint8_t a, uint8_t b, OLED_ColorMode color)
{
    int xpos = 0, ypos = b;
    int a2 = a * a, b2 = b * b;
    int d = b2 + a2 * (0.25 - b);
    while (a2 * ypos > b2 * xpos)
    {
        OLED_SetPixel(x + xpos, y + ypos, color);
        OLED_SetPixel(x - xpos, y + ypos, color);
        OLED_SetPixel(x + xpos, y - ypos, color);
        OLED_SetPixel(x - xpos, y - ypos, color);
        if (d < 0)
            d = d + b2 * ((xpos << 1) + 3), xpos += 1;
        else
            d = d + b2 * ((xpos << 1) + 3) + a2 * (-(ypos << 1) + 2), xpos += 1, ypos -= 1;
    }
    d = b2 * (xpos + 0.5) * (xpos + 0.5) + a2 * (ypos - 1) * (ypos - 1) - a2 * b2;
    while (ypos > 0)
    {
        OLED_SetPixel(x + xpos, y + ypos, color);
        OLED_SetPixel(x - xpos, y + ypos, color);
        OLED_SetPixel(x + xpos, y - ypos, color);
        OLED_SetPixel(x - xpos, y - ypos, color);
        if (d < 0)
            d = d + b2 * ((xpos << 1) + 2) + a2 * (-(ypos << 1) + 3), xpos += 1, ypos -= 1;
        else
            d = d + a2 * (-(ypos << 1) + 3), ypos -= 1;
    }
}

/**
 * @brief 绘制图片
 */
void OLED_DrawImage(uint8_t x, uint8_t y, const Image *img, OLED_ColorMode color)
{
    OLED_SetBlock(x, y, img->data, img->w, img->h, color);
}

/**
 * @brief 绘制单个 ASCII 字符
 */
void OLED_PrintASCIIChar(uint8_t x, uint8_t y, char ch, const ASCIIFont *font, OLED_ColorMode color)
{
    OLED_SetBlock(x, y, font->chars + (ch - ' ') * (((font->h + 7) / 8) * font->w), font->w, font->h, color);
}

/**
 * @brief 绘制 ASCII 字符串
 */
void OLED_PrintASCIIString(uint8_t x, uint8_t y, char *str, const ASCIIFont *font, OLED_ColorMode color)
{
    uint8_t x0 = x;
    while (*str)
    {
        OLED_PrintASCIIChar(x0, y, *str, font, color);
        x0 += font->w;
        str++;
    }
}

/**
 * @brief 获取 UTF-8 编码字符的字节长度
 * @param string 字符首字节指针
 * @return 1~4 表示 UTF-8 字节数；0 表示非法编码
 * @note  内部静态辅助函数，仅用于本文件的字符串解析。
 */
static uint8_t _OLED_GetUTF8Len(char *string)
{
    if ((string[0] & 0x80) == 0x00) return 1;
    else if ((string[0] & 0xE0) == 0xC0) return 2;
    else if ((string[0] & 0xF0) == 0xE0) return 3;
    else if ((string[0] & 0xF8) == 0xF0) return 4;
    return 0;
}

/**
 * @brief 绘制混合字符串（支持中文与 ASCII 自动切换）
 */
void OLED_PrintString(uint8_t x, uint8_t y, char *str, const Font *font, OLED_ColorMode color)
{
    uint16_t i = 0;                                       /* 字符串索引 */
    uint8_t oneLen = (((font->h + 7) / 8) * font->w) + 4; /* 单字模占用字节数（含 4 字节 UTF-8 头） */
    uint8_t found;                                        /* 字模查找成功标志 */
    uint8_t utf8Len;                                      /* 当前字符 UTF-8 长度 */
    uint8_t *head;                                        /* 字库表项头指针 */

    while (str[i])
    {
        found = 0;
        utf8Len = _OLED_GetUTF8Len(str + i);
        if (utf8Len == 0) break; /* 非法 UTF-8 序列，终止绘制 */

        /* 在字库中顺序查找匹配字符（TODO：可优化为二分查找或哈希） */
        for (uint8_t j = 0; j < font->len; j++)
        {
            head = (uint8_t *)(font->chars) + (j * oneLen);
            if (memcmp(str + i, head, utf8Len) == 0)
            {
                OLED_SetBlock(x, y, head + 4, font->w, font->h, color);
                x += font->w;
                i += utf8Len;
                found = 1;
                break;
            }
        }

        /* 缺省处理：ASCII 使用备用字体，非 ASCII 显示空格占位 */
        if (found == 0)
        {
            if (utf8Len == 1)
            {
                OLED_PrintASCIIChar(x, y, str[i], font->ascii, color);
                x += font->ascii->w;
                i += utf8Len;
            }
            else
            {
                OLED_PrintASCIIChar(x, y, ' ', font->ascii, color);
                x += font->ascii->w;
                i += utf8Len;
            }
        }
    }
}