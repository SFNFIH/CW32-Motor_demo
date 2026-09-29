/**
 * @file cw32l012_cordic.h
 * @author WHXY
 * @brief
 * @version 0.1
 * @date 2024-08-07
 *
 * @copyright Copyright (c) 2021
 *
 */
/*******************************************************************************
*
* 代码许可和免责信息
* 武汉芯源半导体有限公司授予您使用所有编程代码示例的非专属的版权许可，您可以由此
* 生成根据您的特定需要而定制的相似功能。根据不能被排除的任何法定保证，武汉芯源半
* 导体有限公司及其程序开发商和供应商对程序或技术支持（如果有）不提供任何明示或暗
* 含的保证或条件，包括但不限于暗含的有关适销性、适用于某种特定用途和非侵权的保证
* 或条件。
* 无论何种情形，武汉芯源半导体有限公司及其程序开发商或供应商均不对下列各项负责，
* 即使被告知其发生的可能性时，也是如此：数据的丢失或损坏；直接的、特别的、附带的
* 或间接的损害，或任何后果性经济损害；或利润、业务、收入、商誉或预期可节省金额的
* 损失。
* 某些司法辖区不允许对直接的、附带的或后果性的损害有任何的排除或限制，因此某些或
* 全部上述排除或限制可能并不适用于您。
*
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CW32L012_CORDIC_H__
#define __CW32L012_CORDIC_H__

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/
#include "cw32l012.h"
#include <math.h>

/*****************************************************************************/
/* Global pre-processor symbols/macros ('#define')                           */
/*****************************************************************************/

// CORDIC函数配置
typedef enum {
    CORDIC_FUNC_COS = 0x0, // 余弦cos
    CORDIC_FUNC_SIN = 0x1, // 正弦sin
    CORDIC_FUNC_ATAN2 = 0x2, // 相位角atan2
    CORDIC_FUNC_HYPOT = 0x3, // 模hypot
    CORDIC_FUNC_ATAN = 0x4, // 反正切atan
    CORDIC_FUNC_COSH = 0x5, // 双曲余弦cosh
    CORDIC_FUNC_SINH = 0x6, // 双曲正弦sinh
    CORDIC_FUNC_ATANH = 0x7, // 双曲反正切atanh
    CORDIC_FUNC_LN = 0x8, // 自然对数ln
    CORDIC_FUNC_SQRT = 0x9 // 平方根sqrt
} cordic_func_t;

// 数据格式配置
typedef enum {
    CORDIC_FORMAT_Q1_15 = 0x0, // q1.15格式
    CORDIC_FORMAT_Q1_31 = 0x1  // q1.31格式
} cordic_format_t;

// 迭代次数配置
typedef enum {
    CORDIC_ITER_2 = 0x0,
    CORDIC_ITER_4,
    CORDIC_ITER_6,
    CORDIC_ITER_8,
    CORDIC_ITER_10,
    CORDIC_ITER_12,
    CORDIC_ITER_14,
    CORDIC_ITER_16,
    CORDIC_ITER_18,
    CORDIC_ITER_20,
    CORDIC_ITER_22,
    CORDIC_ITER_24,
    CORDIC_ITER_26,
    CORDIC_ITER_28,
    CORDIC_ITER_30,
    CORDIC_ITER_32,    
} cordic_iter_t;

// CORDIC初始化结构体
typedef struct {
    cordic_func_t func; // 运算函数
    uint8_t scale;      // 扩展范围
    cordic_format_t format; // 数据格式
    cordic_iter_t iter;       // 迭代次数
    uint8_t comp;       // 伸缩因子补偿
    uint8_t ie;         // 中断使能
    uint8_t dmaeoc;     // DMA使能（运算完成）
    uint8_t dmaidle;    // DMA使能（空闲状态）
} cordic_init_t;

// CORDIC状态结构体
typedef struct {
    uint8_t busy; // 工作状态
    uint8_t eoc;  // 运算完成
} cordic_status_t;

// 函数声明
void CORDIC_Init(cordic_init_t *init);
cordic_status_t CORDIC_GetStatus(void);
void CORDIC_SetFunction(cordic_func_t func);
void CORDIC_SetScale(uint8_t scale);
void CORDIC_SetFormat(cordic_format_t format);
void CORDIC_SetIter(uint8_t iter);
void CORDIC_SetComp(uint8_t comp);
void CORDIC_SetIE(uint8_t ie);
void CORDIC_SetDMAEoc(uint8_t dmaeoc);
void CORDIC_SetDMAIdle(uint8_t dmaidle);
int32_t float_to_q1_31(float value);
float q1_31_to_float(int32_t value);
int32_t float_to_q1_15(float value);
float q1_15_to_float(int32_t value);
    
#ifdef __cplusplus
}
#endif

#endif
