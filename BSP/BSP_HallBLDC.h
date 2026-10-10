/**
 * @file    BSP_HallBLDC.h
 * @brief   HALLTIM 霍尔变化中断换相 + ATIM 六步驱动
 *
 * 霍尔引脚 (原理图): HA=PB13, HB=PB14, HC=PB15 → HALLTIM CH1/2/3
 * 换相仅在 BTIM3_HALLTIM_IRQHandler 中完成；主循环只调 SetDuty。
 */
#ifndef BSP_HALL_BLDC_H
#define BSP_HALL_BLDC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void BSP_HallBLDC_Init(void);
void BSP_HallBLDC_Start(void);
void BSP_HallBLDC_Stop(void);

/** 主循环：只改占空比 0..1000（千分比） */
void BSP_HallBLDC_SetDutyPermille(uint16_t permille);

void BSP_HallBLDC_SetDirection(int8_t dir);
void BSP_HallBLDC_ToggleDirection(void);
int8_t BSP_HallBLDC_GetDirection(void);

/** HALLTIM CAP 中断入口（在 BTIM3_HALLTIM_IRQHandler 调用） */
void BSP_HallBLDC_IRQHandler(void);

uint8_t  BSP_HallBLDC_GetHallState(void);
uint8_t  BSP_HallBLDC_GetStep(void);
uint8_t  BSP_HallBLDC_IsRunning(void);
uint32_t BSP_HallBLDC_GetHallPeriod(void);
uint16_t BSP_HallBLDC_GetFault(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_HALL_BLDC_H */
