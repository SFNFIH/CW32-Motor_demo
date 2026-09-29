/**
 * @file    BSP_FOC.h
 * @brief   SPWM: 速度 / 角度外环 + 电流上限内环 (电位器设 Imax)
 */
#ifndef BSP_FOC_H
#define BSP_FOC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BSP_FOC_CTRL_SPEED    0U
#define BSP_FOC_CTRL_ANGLE    1U
#define BSP_FOC_CTRL_CURRENT  2U  /* 电位器设 Imax; 内环限流叠加在速度/角度上 */

typedef struct
{
    uint8_t  mode;      /* 0停 1开环爬升 2速度闭环 3角度闭环 4失步恢复 */
    float    id_a;      /* 速度: fe_x10; 角度: amp; 电流设定: 实测电流 A */
    float    iq_a;      /* 速度: rpm_ref; 角度: 目标角(度); 电流设定: Imax A */
    float    theta;
    float    omega_e;   /* 速度: speed_pm; 角度: 实测角(度) */
    float    rpm;       /* 实测 rpm */
    float    i_meas_a;  /* 相电流峰值近似 (A) */
    float    i_lim_a;   /* 电流上限 (A), 叠加到速度/角度 */
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

/** 电位器 0..1000 → 电流上限 (电流设定模式; 对速度/角度环持续生效) */
void BSP_FOC_SetImaxPm(uint16_t permille);
uint16_t BSP_FOC_GetImaxPm(void);

void BSP_FOC_SetCtrlMode(uint8_t mode);
uint8_t BSP_FOC_GetCtrlMode(void);
/** 双击轮换: 速度 → 角度 → 电流上限 → 速度 */
void BSP_FOC_ToggleCtrlMode(void);
/** 电流设定模式下仍在跑的外环: SPEED 或 ANGLE */
uint8_t BSP_FOC_GetMotionMode(void);

void BSP_FOC_OnEncoder(uint16_t raw, uint8_t ok);
void BSP_FOC_OnEncoderRpm(int32_t rpm_x10);
void BSP_FOC_OnEncoderCum(int32_t cum_raw);

/** 刷新电流采样 (停机时也要给 VOFA) */
void BSP_FOC_PollCurrent(void);

/** 主循环调用: 速度或角度外环, 内环始终限流 */
void BSP_FOC_SpeedLoop(void);

void BSP_FOC_SetDirection(int8_t dir);
void BSP_FOC_ToggleDirection(void);
void BSP_FOC_PwmIrq(void);
const BSP_FOC_State_t *BSP_FOC_GetState(void);

#ifdef __cplusplus
}
#endif

#endif
