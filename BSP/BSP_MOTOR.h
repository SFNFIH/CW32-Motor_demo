/**
 * @file    BSP_MOTOR.h
 * @brief   三相互补 PWM (SVPWM/SPWM)
 */
#ifndef BSP_MOTOR_H
#define BSP_MOTOR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BSP_MOTOR_PWM_ARR       3199U
#define BSP_MOTOR_PWM_HZ        15000U
#define BSP_MOTOR_PWM_DEADTIME  64U

void BSP_MOTOR_Init(void);
void BSP_MOTOR_Start(void);
void BSP_MOTOR_Stop(void);
void BSP_MOTOR_SetPhaseDuty(uint16_t da, uint16_t db, uint16_t dc);
/** 等待 n 次 ATIM 更新事件 (OC 预装载生效 + 完整 PWM 周期) */
void BSP_MOTOR_WaitUpdate(uint32_t n);
void BSP_MOTOR_EnablePwmIrq(void);
void BSP_MOTOR_DisablePwmIrq(void);

#ifdef __cplusplus
}
#endif

#endif
