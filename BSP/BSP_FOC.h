/**
 * @file    BSP_FOC.h
 * @brief   SPWM 速度闭环 V/f + AS5600 测速 (低速下限已放宽)
 */
#ifndef BSP_FOC_H
#define BSP_FOC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint8_t  mode;      /* 0停 1开环启动 2速度闭环 */
    float    id_a;      /* amp */
    float    iq_a;      /* rpm_ref/1000 */
    float    theta;
    float    omega_e;
    float    rpm;       /* 实测 rpm */
    uint8_t  running;
    uint32_t isr_cnt;
} BSP_FOC_State_t;

void BSP_FOC_Init(void);
void BSP_FOC_Start(void);
void BSP_FOC_Stop(void);

/** 电位器归一化 0..1000 → 内部转速目标 */
void BSP_FOC_SetSpeed(uint16_t speed_permille);

void BSP_FOC_OnEncoder(uint16_t raw, uint8_t ok);
void BSP_FOC_OnEncoderRpm(int32_t rpm_x10);
void BSP_FOC_OnEncoderCum(int32_t cum_raw);

/** 主循环在 AS5600_Update 之后调用, 跑速度环 (~5ms) */
void BSP_FOC_SpeedLoop(void);

void BSP_FOC_SetDirection(int8_t dir);
void BSP_FOC_ToggleDirection(void);
void BSP_FOC_PwmIrq(void);
const BSP_FOC_State_t *BSP_FOC_GetState(void);

#ifdef __cplusplus
}
#endif

#endif
