/**
 * @file    BSP_Current.h
 * @brief   A/B 相电流: 片内 OPA1/OPA2 + ADC1
 *          A: OPA1+ PA6, OPA1- PA7, OPA1OUT PB0 (ADC1 CH8)
 *          B: OPA2+ PA4, OPA2- PA5, OPA2OUT PB1 (ADC1 CH9)
 */
#ifndef BSP_CURRENT_H
#define BSP_CURRENT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void BSP_Current_Init(void);
/** 重新配置 ADC1 为电流双通道 (BEMF Exit 后调用) */
void BSP_Current_ReconfigAdc(void);
/** PWM 关断时调用, 采零电流偏置 */
void BSP_Current_Calibrate(void);
/** 返回安培. 0=超时 */
uint8_t BSP_Current_Read(float *ia, float *ib);

#ifdef __cplusplus
}
#endif

#endif /* BSP_CURRENT_H */
