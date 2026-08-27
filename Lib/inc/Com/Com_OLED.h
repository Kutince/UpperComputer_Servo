#ifndef __COM_OLED_H__
#define __COM_OLED_H__

#include "Dri_OLED.h"
#include "Com_Font.h"

/* ========================== 图形绘制接口 ========================== */

/**
 * @brief 绘制线段（Bresenham 算法）
 * @param x1,y1 起点坐标
 * @param x2,y2 终点坐标
 * @param color 颜色模式
 * @note  支持水平、垂直及任意斜率直线。
 */
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, OLED_ColorMode color);

/**
 * @brief 绘制空心矩形
 * @param x,y 左上角坐标
 * @param w   宽度（像素）
 * @param h   高度（像素）
 * @param color 颜色模式
 */
void OLED_DrawRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, OLED_ColorMode color);

/**
 * @brief 绘制实心矩形
 * @param x,y 左上角坐标
 * @param w   宽度
 * @param h   高度
 * @param color 颜色模式
 * @note  内部通过水平扫描线填充。
 */
void OLED_DrawFilledRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, OLED_ColorMode color);

/**
 * @brief 绘制空心三角形
 * @param x1,y1 顶点 1
 * @param x2,y2 顶点 2
 * @param x3,y3 顶点 3
 * @param color 颜色模式
 */
void OLED_DrawTriangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3, OLED_ColorMode color);

/**
 * @brief 绘制实心三角形
 * @param x1,y1 顶点 1
 * @param x2,y2 顶点 2
 * @param x3,y3 顶点 3
 * @param color 颜色模式
 * @note  采用扫描线算法，按 y 轴方向分上下两部分填充。
 */
void OLED_DrawFilledTriangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3, OLED_ColorMode color);

/**
 * @brief 绘制空心圆（Bresenham 算法）
 * @param x,y 圆心坐标
 * @param r   半径
 * @param color 颜色模式
 */
void OLED_DrawCircle(uint8_t x, uint8_t y, uint8_t r, OLED_ColorMode color);

/**
 * @brief 绘制实心圆（Bresenham 算法）
 * @param x,y 圆心坐标
 * @param r   半径
 * @param color 颜色模式
 * @note  通过水平扫描线逐行填充。
 */
void OLED_DrawFilledCircle(uint8_t x, uint8_t y, uint8_t r, OLED_ColorMode color);

/**
 * @brief 绘制空心椭圆（中点椭圆算法）
 * @param x,y 中心坐标
 * @param a   水平半轴长度
 * @param b   垂直半轴长度
 * @param color 颜色模式
 */
void OLED_DrawEllipse(uint8_t x, uint8_t y, uint8_t a, uint8_t b, OLED_ColorMode color);

/**
 * @brief 绘制图片
 * @param x,y 左上角坐标
 * @param img 图片结构体指针
 * @param color 颜色模式
 * @note  图片数据需为列行式位图格式，可用波特律动取模工具生成。
 */
void OLED_DrawImage(uint8_t x, uint8_t y, const Image *img, OLED_ColorMode color);

/* ========================== 文字绘制接口 ========================== */

/**
 * @brief 绘制单个 ASCII 字符
 * @param x,y   左上角坐标
 * @param ch    待显示字符（基于 ' ' 的偏移编码）
 * @param font  ASCII 字体结构体
 * @param color 颜色模式
 */
void OLED_PrintASCIIChar(uint8_t x, uint8_t y, char ch, const ASCIIFont *font, OLED_ColorMode color);

/**
 * @brief 绘制 ASCII 字符串
 * @param x,y   起始左上角坐标
 * @param str   以 '\\0' 结尾的字符串
 * @param font  ASCII 字体结构体
 * @param color 颜色模式
 * @note  根据字体宽度自动递增 x 坐标，不支持自动换行。
 */
void OLED_PrintASCIIString(uint8_t x, uint8_t y, char *str, const ASCIIFont *font, OLED_ColorMode color);

/**
 * @brief 绘制混合字符串（支持中文与 ASCII 自动切换）
 * @param x,y   起始左上角坐标
 * @param str   UTF-8 编码字符串
 * @param font  中文字体结构体（内含缺省 ASCII 字体）
 * @param color 颜色模式
 * @note  1. 编译器字符集必须设置为 UTF-8。
 *        2. 中文字库需使用波特律动 LED 取模助手生成。
 *        3. 若字库中未找到对应中文字模，且当前字符为 ASCII，则使用 font->ascii 缺省字体显示；否则显示空格占位。
 */
void OLED_PrintString(uint8_t x, uint8_t y, char *str, const Font *font, OLED_ColorMode color);

#endif /* __COM_OLED_H__ */