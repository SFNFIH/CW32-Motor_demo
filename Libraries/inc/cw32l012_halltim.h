/**
 * @file cw32l012_halltim.h
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
#ifndef __CW32L012_HALLTIM_H__
#define __CW32L012_HALLTIM_H__

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/
#include "cw32l012.h"

/*****************************************************************************/
/* Global pre-processor symbols/macros ('#define')                           */
/*****************************************************************************/
// HALLTIM_CR 控制寄存器位域定义
typedef enum {
    HALLTIM_CR_SOFTTRIG = 0x00800000, // 软件触发信号生成控制
    HALLTIM_CR_FLT2LEN = 0x00FF0000,   // 滤波宽度配置掩码
    HALLTIM_CR_MMS = 0x00000700,      // 触发输出选择掩码
    HALLTIM_CR_DIV = 0x000000C0,      // HALL模块计数时钟来源分频配置掩码
    HALLTIM_CR_FLT1EN = 0x00000004,    // 7/5滤波器使能控制
    HALLTIM_CR_EN = 0x00000001        // HALL模块使能控制
} HALLTIM_CR_BitField;

// HALLTIM_DIER DMA、中断使能控制寄存器位域定义
typedef enum {
    HALLTIM_DIER_SOFTDMA = 0x00000080, // 软件触发DMA使能控制
    HALLTIM_DIER_MATCHDMA = 0x00000040, // 计数器匹配DMA使能控制
    HALLTIM_DIER_OVDMA = 0x00000020,   // 计数器溢出DMA使能控制
    HALLTIM_DIER_CHGDMA = 0x00000010,  // HALL信号DMA中断使能控制
    HALLTIM_DIER_MATCH = 0x00000004,   // 计数器匹配中断使能控制
    HALLTIM_DIER_OV = 0x00000002,      // 计数器溢出中断使能控制
    HALLTIM_DIER_CHG = 0x00000001      // HALL信号变化中断使能控制
} HALLTIM_DIER_BitField;

// HALLTIM_ISR 中断标志寄存器位域定义
typedef enum {
    HALLTIM_ISR_MATCH = 0x00000004, // 匹配标志
    HALLTIM_ISR_OV = 0x00000002,   // 溢出标志
    HALLTIM_ISR_CHG = 0x00000001   // HALL信号变化标志
} HALLTIM_ISR_BitField;

// HALLTIM_ICR 中断标志清除寄存器位域定义
typedef enum {
    HALLTIM_ICR_MATCH = 0x00000004, // 匹配标志清除
    HALLTIM_ICR_OV = 0x00000002,   // 溢出标志清除
    HALLTIM_ICR_CHG = 0x00000001   // HALL信号变化标志清除
} HALLTIM_ICR_BitField;

// HALLTIM_MMS 触发输出选择枚举
typedef enum {
    HALLTIM_MMS_OV = 0x00000000,    // OV，溢出脉冲
    HALLTIM_MMS_PWM = 0x0000010,   // PWM，PWM信号
    HALLTIM_MMS_MATCH = 0x0000020, // MATCH，CCR匹配脉冲
    HALLTIM_MMS_CHANGE = 0x0000030, // CHANGE，HALL信号变化脉冲
    HALLTIM_MMS_CH3FLT = 0x0000040, // CH3FLT，滤波后的CH3波形
    HALLTIM_MMS_CH2FLT = 0x0000050, // CH2FLT，滤波后的CH2波形
    HALLTIM_MMS_CH1FLT = 0x0000060, // CH1FLT，滤波后的CH1波形
    HALLTIM_MMS_CHXOR = 0x0000070   // CHXOR，3通道滤波后的信号异或
} HALLTIM_MMS;

// HALLTIM_DIV HALL模块计数时钟来源分频配置枚举
typedef enum {
    HALLTIM_DIV_PCLK = 0x00000000,  // PCLK
    HALLTIM_DIV_PCLK_DIV2 = 0x0000004, // PCLK/2
    HALLTIM_DIV_PCLK_DIV4 = 0x0000008, // PCLK/4
    HALLTIM_DIV_PCLK_DIV8 = 0x000000C  // PCLK/8
} HALLTIM_DIV;

// 初始化配置结构体
typedef struct {
    boolean_t filter_enable;        // 是否启用7/5滤波器
    uint8_t filter_length;        // 滤波器长度（0-0x7FFF）
    HALLTIM_DIV clock_division;       // 时钟分频（0: PCLK, 0x4: PCLK/2, 0x8: PCLK/4, 0xc: PCLK/8）
    HALLTIM_MMS trigger_output;       // 触发输出模式（0: OV, 0x10: PWM, 0x20: MATCH, 0x30: CHANGE,
                                      //  0x40: CH3FLT, 0x50: CH2FLT, 0x60: CH1FLT, 0x70: CHXOR）
    uint32_t auto_reload_value;   // 自动重装载值
} HALLTIM_InitTypeDef;

// 驱动库函数声明
void HALLTIM_Init(HALLTIM_InitTypeDef *HALLTIM_InitStruct);
void HALLTIM_Cmd(FunctionalState state);
void HALLTIM_SetFilterLength(uint32_t length);
void HALLTIM_SetMode(HALLTIM_MMS mode);
void HALLTIM_SetClockDivider(HALLTIM_DIV div);
void HALLTIM_GenerateSoftTrigger(void);
void HALLTIM_SetAutoReloadValue(uint32_t value);
uint32_t HALLTIM_GetCounterValue(void);
uint32_t HALLTIM_GetPulseWidth(void);
void HALLTIM_SetCompareValue(uint32_t value);
uint32_t HALLTIM_GetStatus(void);
void HALLTIM_ClearStatus(uint32_t flags);
uint32_t HALLTIM_GetRawStateCH1(void);
uint32_t HALLTIM_GetRawStateCH2(void);
uint32_t HALLTIM_GetRawStateCH3(void);
uint32_t HALLTIM_GetFilteredStateCH1(void);
uint32_t HALLTIM_GetFilteredStateCH2(void);
uint32_t HALLTIM_GetFilteredStateCH3(void);

#ifdef __cplusplus
}
#endif

#endif
