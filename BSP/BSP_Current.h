/**
 * @file    BSP_Current.h
 * @brief   A/B 相电流: 片内 OPA1/OPA2 + ADC1
 *          A: OPA1+ PA6, OPA1- PA7, OPA1OUT PB0 (ADC1 CH8)
 *          B: OPA2+ PA4, OPA2- PA5, OPA2OUT PB1 (ADC1 CH9)
 *
 * Doc 原理图: 与 BEMF(PA0/1/2) 独立引脚。
 * 电流: FAST 双通道 CH8/9, 在 PWM 峰 (LS 窗) 采样。
 * BEMF: ReconfigAdc() 切回 FULL 五通道序列。
 */
#ifndef BSP_CURRENT_H
#define BSP_CURRENT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void BSP_Current_Init(void);
/** 配置 ADC1 FULL: CH0/1/2 (BEMF) + CH8/9, 供无驱磁链 */
void BSP_Current_ReconfigAdc(void);
/** PWM 关断时调用, 采零电流偏置 */
void BSP_Current_Calibrate(void);
/** 返回安培 (FAST + LS 窗). 0=超时 */
uint8_t BSP_Current_Read(float *ia, float *ib);

#ifdef __cplusplus
}
#endif

#endif /* BSP_CURRENT_H */
