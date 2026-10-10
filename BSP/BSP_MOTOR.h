/**
 * @file    BSP_MOTOR.h
 * @brief   ATIM 三相互补 PWM + 六步方波换相
 */
#ifndef BSP_MOTOR_H
#define BSP_MOTOR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 96 MHz, 边沿对齐: f = 96e6 / (ARR+1) → 20 kHz */
#define BSP_MOTOR_PWM_ARR       4799U
#define BSP_MOTOR_PWM_HZ        20000U
#define BSP_MOTOR_PWM_DEADTIME  64U

void BSP_MOTOR_Init(void);
void BSP_MOTOR_Start(void);
void BSP_MOTOR_Stop(void);

/** 六步换相占空比 0..ARR（主循环可随时改，换相 ISR 读最新值） */
void BSP_MOTOR_SetDuty(uint16_t duty);
uint16_t BSP_MOTOR_GetDuty(void);

/**
 * 六步换相 step=0..5:
 * 0:A+B-  1:A+C-  2:B+C-  3:B+A-  4:C+A-  5:C+B-
 * 浮空相关闭 CH/CHN。
 */
void BSP_MOTOR_CommutateStep(uint8_t step);

#ifdef __cplusplus
}
#endif

#endif
