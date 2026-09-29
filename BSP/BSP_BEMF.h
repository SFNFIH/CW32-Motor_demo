/**
 * @file    BSP_BEMF.h
 * @brief   反电势 ADC: PA0/1/2, ATIM CH4 触发, EOS 中断
 */
#ifndef BSP_BEMF_H
#define BSP_BEMF_H

#include <stdint.h>
#include "cw32l012_gpio.h"
#include "cw32l012_sysctrl.h"
#include "cw32l012_adc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BSP_BEMF_ADC     CW_ADC1

/** SampleData[0]=A, [1]=B, [2]=C — 与参考工程索引对齐(减去电流槽) */
extern volatile uint16_t g_bemf_sample[3];

void BSP_BEMF_Init(void);

/** ADC1 EOS: 读结果并回调无感处理 */
void BSP_BEMF_IRQHandler(void);

uint16_t BSP_BEMF_ReadPhase(uint8_t phase);
void BSP_BEMF_Read(uint16_t *a, uint16_t *b, uint16_t *c);
uint16_t BSP_BEMF_ReadA(void);
uint16_t BSP_BEMF_ReadB(void);
uint16_t BSP_BEMF_ReadC(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_BEMF_H */
