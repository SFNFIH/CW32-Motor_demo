/**
 * @file    BSP_Current.h
 * @brief   A/B 相电流: 片内 OPA1/OPA2 + ADC1
 *          A: OPA1+ PA6, OPA1- PA7, OPA1OUT PB0 (ADC1 CH8)
 *          B: OPA2+ PA4, OPA2- PA5, OPA2OUT PB1 (ADC1 CH9)
 *
 * Doc 原理图: 与 BEMF(PA0/1/2) 独立引脚。ADC1 统一序列含 CH0..2+CH8/9,
 * 电流读 RESULT_3/4。
 */
#ifndef BSP_CURRENT_H
#define BSP_CURRENT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void BSP_Current_Init(void);
/** 配置 ADC1: CH0/1/2 (BEMF) + CH8/9 (电流), 软件触发 */
void BSP_Current_ReconfigAdc(void);
/** PWM 关断时调用, 采零电流偏置 */
void BSP_Current_Calibrate(void);
/** 返回安培. 0=超时 */
uint8_t BSP_Current_Read(float *ia, float *ib);

#ifdef __cplusplus
}
#endif

#endif /* BSP_CURRENT_H */
