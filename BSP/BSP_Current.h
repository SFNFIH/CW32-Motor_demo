/**
 * @file    BSP_Current.h
 * @brief   A/B 相电流: 片内 OPA1/OPA2 + ADC1
 *          A: OPA1 OUT PB0 (ADC1 CH8)
 *          B: OPA2 OUT PB1 (ADC1 CH9)
 */
#ifndef BSP_CURRENT_H
#define BSP_CURRENT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void BSP_Current_Init(void);
/** PWM 关断或中点时调用, 采零电流偏置 */
void BSP_Current_Calibrate(void);
/** ISR 内采样并缓存原始 ADC */
void BSP_Current_Sample(void);
uint16_t BSP_Current_GetRawA(void);
uint16_t BSP_Current_GetRawB(void);
/** 返回安培. 0=超时 */
uint8_t BSP_Current_Read(float *ia, float *ib);

#ifdef __cplusplus
}
#endif

#endif /* BSP_CURRENT_H */
