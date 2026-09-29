/**
 * @file    BSP_FOC.h
 * @brief   SPWM: 速度闭环 V/f 或 角度闭环 (电位器→机械角)
 */
#ifndef BSP_FOC_H
#define BSP_FOC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BSP_FOC_CTRL_SPEED  0U
#define BSP_FOC_CTRL_ANGLE  1U

typedef struct
{
    uint8_t  mode;      /* 0停 1开环爬升 2速度闭环 3角度闭环 4失步恢复 */
    float    id_a;      /* 速度: fe_x10; 角度: amp */
    float    iq_a;      /* 速度: rpm_ref; 角度: 目标角(度) */
    float    theta;
    float    omega_e;   /* 速度: speed_pm; 角度: 实测角(度) */
    float    rpm;       /* 实测 rpm */
    uint8_t  running;
    uint32_t isr_cnt;
} BSP_FOC_State_t;

void BSP_FOC_Init(void);
void BSP_FOC_Start(void);
void BSP_FOC_Stop(void);

/** 电位器归一化 0..1000 → 转速目标 (速度模式) */
void BSP_FOC_SetSpeed(uint16_t speed_permille);

/** 电位器/目标角 raw 0..4095 → 0..360°; 反馈用 cum 绝对多圈, 超圈可回位 */
void BSP_FOC_SetAngleRaw(uint16_t raw_0_4095);

void BSP_FOC_SetCtrlMode(uint8_t mode);
uint8_t BSP_FOC_GetCtrlMode(void);
void BSP_FOC_ToggleCtrlMode(void);

void BSP_FOC_OnEncoder(uint16_t raw, uint8_t ok);
void BSP_FOC_OnEncoderRpm(int32_t rpm_x10);
void BSP_FOC_OnEncoderCum(int32_t cum_raw);

/** 主循环调用: 按当前模式跑速度环或角度环 */
void BSP_FOC_SpeedLoop(void);

void BSP_FOC_SetDirection(int8_t dir);
void BSP_FOC_ToggleDirection(void);
void BSP_FOC_PwmIrq(void);
const BSP_FOC_State_t *BSP_FOC_GetState(void);

#ifdef __cplusplus
}
#endif

#endif
