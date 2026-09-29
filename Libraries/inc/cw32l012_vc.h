/**
 * @file cw32l012_vc.h
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
/*******************************************************************************
*            VC1:           VC2:          VC3          VC4            
* INP0       PA00           PA01          PA06         PA09
* INP1       PA02           PA03          PA07         PA10      
* INP2       PA04           PA05          PB00         PB10
* INP3       PB00           PB01          PB02         PB01
*
* INN0       PA00           PA01          PA06         PA09
* INN1       PA02           PA03          PA07         PA10
*/

#ifndef __CW32L012_VC_H
#define __CW32L012_VC_H

/******************************************************************************
 * Include files
 ******************************************************************************/
#include "system_cw32l012.h"
/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/**
 ******************************************************************************
 ** \defgroup VcGroup Voltage Comparator (VC)
 **
 ******************************************************************************/
//@{
          

/******************************************************************************
 * Global type definitions
 ******************************************************************************/
// 定义参考电压源
typedef enum {
    VC_REF_EXT = 0,    // 选择外部输入，则关闭内部的分压
    VC_REF_AVCC = 1,
    VC_REF_VCORE = 3,
    
} VC_RefVoltage;

// 定义电阻分压系数
typedef enum {
    VC_DIV_0 = 0,    // 输出电压 = 输入电压 * (1 + 0) / 8
    VC_DIV_1 = 1,    // 输出电压 = 输入电压 * (1 + 1) / 8
    VC_DIV_2 = 2,    // 输出电压 = 输入电压 * (1 + 2) / 8
    VC_DIV_3 = 3,    // 输出电压 = 输入电压 * (1 + 3) / 8
    VC_DIV_4 = 4,    // 输出电压 = 输入电压 * (1 + 4) / 8
    VC_DIV_5 = 5,    // 输出电压 = 输入电压 * (1 + 5) / 8
    VC_DIV_6 = 6,    // 输出电压 = 输入电压 * (1 + 6) / 8
    VC_DIV_7 = 7     // 输出电压 = 输入电压 * (1 + 7) / 8
} VC_Divider;

// 定义通用输入信号选择
typedef enum {
    VC_INPUT_IN0 = 0,
    VC_INPUT_IN1 = 1,
    VC_INPUT_IN2 = 2,
    VC_INPUT_IN3 = 3
} VC_InputSignal;

// 定义正端输入信号选择
typedef enum {
    VC_INP_IN0 = 0,
    VC_INP_IN1 = 1,
    VC_INP_IN2 = 2,
    VC_INP_IN3 = 3
} VC_PosInputSignal;

// 定义负端输入信号选择
typedef enum {
    VC_INN_IN0 = 0,
    VC_INN_IN1 = 1,
    VC_INN_DAC = 2,
    VC_INN_REF = 3
} VC_NegInputSignal;

// 定义中断触发方式
typedef enum {
    VC_INT_DISABLE = 0,
    VC_INT_FALLING = 1,
    VC_INT_RISING = 2,    
    VC_INT_HIGH = 4
} VC_InterruptMode;

// 定义滤波器时钟源
typedef enum {
    VC_FILTER_CLOCK_LSI = 0,
    VC_FILTER_CLOCK_PCLK
}VC_FilterClock;

// 定义滤波器时间
typedef enum {
    VC_FILTER_DISABLE = 0,
    VC_FILTER_1X_2SAMPLES = 1,
    VC_FILTER_1X_4SAMPLES = 2,
    VC_FILTER_1X_8SAMPLES = 3,
    VC_FILTER_2X_6SAMPLES = 4,
    VC_FILTER_2X_8SAMPLES = 5,
    VC_FILTER_4X_6SAMPLES = 6,
    VC_FILTER_4X_8SAMPLES = 7,
    VC_FILTER_8X_6SAMPLES = 8,
    VC_FILTER_8X_8SAMPLES = 9,
    VC_FILTER_16X_5SAMPLES = 10,
    VC_FILTER_16X_6SAMPLES = 11,
    VC_FILTER_16X_8SAMPLES = 12,
    VC_FILTER_32X_5SAMPLES = 13,
    VC_FILTER_32X_6SAMPLES = 14,
    VC_FILTER_32X_8SAMPLES = 15
} VC_FilterTime;

// 定义滤波器配置
typedef struct {
    VC_FilterClock FilterClock;
    VC_FilterTime FilterTime;
}VC_FilterConfig;
    

// 定义VC相应速度
typedef enum {
    VC_RESPONSE_SLOW = 0,
    VC_RESPONSE_FAST
}VC_ResponseSpeed;

// 定义VC模块配置结构体
typedef struct {    
    VC_RefVoltage RefVoltage;        // 参考输入电压源
    VC_Divider div;                  // 分压系数
    VC_PosInputSignal inputPositive;    // 正端输入信号
    VC_NegInputSignal inputNegative;    // 负端输入信号
    boolean_t polarityInvert;        // 输出极性
    boolean_t hysteresisEnable;      // 迟滞窗口
    VC_ResponseSpeed responseSpeed;  // 响应速度
    VC_InterruptMode interruptMode;  // 中断触发方式
    VC_FilterConfig filterConfig;    // 滤波器配置
} VC_InitTypeDef;

typedef enum {
    VC_BLANK_OUTPUT_LOW = 0,
    VC_BLANK_OUTPUT_HIGH
}VC_BlankOutput;

// 定义 BLANKTIME 枚举
typedef enum {
    VC_BLANKTIME_6PCLK = 0,  // BLANK窗口持续时间为 2 ^ (0+2) + N PCLK周期，N = 0~2, 最多6个PCLK
    VC_BLANKTIME_10PCLK,     // BLANK窗口持续时间为 2 ^ (1+2) + N PCLK周期，N = 0~2, 最多10个PCLK
    VC_BLANKTIME_18PCLK,     // BLANK窗口持续时间为 2 ^ (2+2) + N PCLK周期，N = 0~2, 最多18个PCLK
    VC_BLANKTIME_34PCLK,     // BLANK窗口持续时间为 2 ^ (3+2) + N PCLK周期，N = 0~2, 最多34个PCLK
    VC_BLANKTIME_66PCLK,     // BLANK窗口持续时间为 2 ^ (4+2) + N PCLK周期，N = 0~2, 最多66个PCLK
    VC_BLANKTIME_130PCLK,    // BLANK窗口持续时间为 2 ^ (5+2) + N PCLK周期，N = 0~2, 最多130个PCLK
    VC_BLANKTIME_258PCLK,    // BLANK窗口持续时间为 2 ^ (6+2) + N PCLK周期，N = 0~2, 最多258个PCLK
    VC_BLANKTIME_514PCLK,    // BLANK窗口持续时间为 2 ^ (7+2) + N PCLK周期，N = 0~2, 最多514个PCLK
} VC_BlankTime;

// 定义 BLANK窗口使能控制
typedef enum{
    VC_CR2_HALLTIM1TRGO = (int32_t) (1 << 31), // HALLTIM1TRGO信号触发 BLANK窗口使能控制
    VC_CR2_BTIM3TRGO = (1 << 30),    // BTIM3TRGO信号触发 BLANK窗口使能控制
    VC_CR2_BTIM2TRGO = (1 << 29),    // BTIM2TRGO信号触发 BLANK窗口使能控制
    VC_CR2_BTIM1TRGO = (1 << 28),    // BTIM1TRGO信号触发 BLANK窗口使能控制
    VC_CR2_GTIM4OC4REFC = (1 << 27), // GTIM4OC4REFC上升沿信号触发 BLANK窗口使能控制
    VC_CR2_GTIM4OC3REFC = (1 << 26), // GTIM4OC3REFC上升沿信号触发 BLANK窗口使能控制
    VC_CR2_GTIM4OC2REFC = (1 << 25), // GTIM4OC2REFC上升沿信号触发 BLANK窗口使能控制
    VC_CR2_GTIM4OC1REFC = (1 << 24), // GTIM4OC1REFC上升沿信号触发 BLANK窗口使能控制
    VC_CR2_GTIM4TRGO = (1 << 23),   // GTIM4TRGO信号触发 BLANK窗口使能控制
    VC_CR2_GTIM3OC4REFC = (1 << 22), // GTIM3OC4REFC上升沿信号触发 BLANK窗口使能控制
    VC_CR2_GTIM3OC3REFC = (1 << 21), // GTIM3OC3REFC上升沿信号触发 BLANK窗口使能控制
    VC_CR2_GTIM3OC2REFC = (1 << 20), // GTIM3OC2REFC上升沿信号触发 BLANK窗口使能控制
    VC_CR2_GTIM3OC1REFC = (1 << 19), // GTIM3OC1REFC上升沿信号触发 BLANK窗口使能控制
    VC_CR2_GTIM3TRGO = (1 << 18),   // GTIM3TRGO信号触发 BLANK窗口使能控制
    VC_CR2_GTIM2OC4REFC = (1 << 17), // GTIM2OC4REFC上升沿信号触发 BLANK窗口使能控制
    VC_CR2_GTIM2OC3REFC = (1 << 16), // GTIM2OC3REFC上升沿信号触发 BLANK窗口使能控制
    VC_CR2_GTIM2OC2REFC = (1 << 15), // GTIM2OC2REFC上升沿信号触发 BLANK窗口使能控制
    VC_CR2_GTIM2OC1REFC = (1 << 14), // GTIM2OC1REFC上升沿信号触发 BLANK窗口使能控制
    VC_CR2_GTIM2TRGO = (1 << 13),   // GTIM2TRGO信号触发 BLANK窗口使能控制
    VC_CR2_GTIM1OC4REFC = (1 << 12), // GTIM1OC4REFC上升沿信号触发 BLANK窗口使能控制
    VC_CR2_GTIM1OC3REFC = (1 << 11), // GTIM1OC3REFC上升沿信号触发 BLANK窗口使能控制
    VC_CR2_GTIM1OC2REFC = (1 << 10), // GTIM1OC2REFC上升沿信号触发 BLANK窗口使能控制
    VC_CR2_GTIM1OC1REFC = (1 << 9),  // GTIM1OC1REFC上升沿信号触发 BLANK窗口使能控制
    VC_CR2_GTIM1TRGO = (1 << 8),    // GTIM1TRGO信号触发 BLANK窗口使能控制
    VC_CR2_ATIM1OC6REFC = (1 << 7), // ATIM1 OC6REFC上升沿信号触发 BLANK窗口使能控制
    VC_CR2_ATIM1OC5REFC = (1 << 6), // ATIM1 OC5REFC上升沿信号触发 BLANK窗口使能控制
    VC_CR2_ATIM1OC4REFC = (1 << 5), // ATIM1 OC4REFC上升沿信号触发 BLANK窗口使能控制
    VC_CR2_ATIM1OC3REFC = (1 << 4), // ATIM1 OC3REFC上升沿信号触发 BLANK窗口使能控制
    VC_CR2_ATIM1OC2REFC = (1 << 3), // ATIM1 OC2REFC上升沿信号触发 BLANK窗口使能控制
    VC_CR2_ATIM1OC1REFC = (1 << 2), // ATIM1 OC1REFC上升沿信号触发 BLANK窗口使能控制
    VC_CR2_ATIM1TRGO2 = (1 << 1),  // ATIM1 TRGO2信号触发 BLANK窗口使能控制
    VC_CR2_ATIM1TRGO = (1 << 0)    // ATIM1 TRGO信号触发 BLANK窗口使能控制
} VC_BlankTrigSource;

typedef struct {
    VC_BlankOutput     BlankOutput;
    VC_BlankTime       BlankTime;
    VC_BlankTrigSource BlankTrigSource;
}VC_BlankWindowCfg;
    

// 函数声明
void VC_Init(VC_TypeDef *VCx, VC_InitTypeDef* VC_InitStruct);
void VC_Cmd(VC_TypeDef *VCx, FunctionalState state);
void VC_SetRefVoltage(VC_TypeDef *VCx, VC_RefVoltage RefVoltage, VC_Divider div);
void VC_SetInputSignals(VC_TypeDef *VCx, VC_PosInputSignal inputPositive, VC_NegInputSignal inputNegative);
void VC_SetInterruptMode(VC_TypeDef *VCx, VC_InterruptMode mode);
void VC_SetFilterConfig(VC_TypeDef *VCx, VC_FilterConfig *config);
uint8_t VC_GetInterruptFlag(VC_TypeDef *VCx);
void VC_ClearInterruptFlag(VC_TypeDef *VCx);
uint8_t VC_GetFilterOutput(VC_TypeDef *VCx);
void VC_SetBlankWindow(VC_TypeDef *VCx, VC_BlankWindowCfg *config);


#ifdef __cplusplus
}
#endif

#endif /* __CW32L012_VC_H */
/******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/

