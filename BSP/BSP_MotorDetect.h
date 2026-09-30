/**
 * @file    BSP_MotorDetect.h
 * @brief   电机参数自整定 (移植自 vedderb/bldc FOC detect)
 *
 * VESC 对照:
 *   mcpwm_foc_dc_cal                         → RunAll 开头电流偏置校准
 *   mcpwm_foc_measure_resistance             → MeasureR (含死区电压补偿)
 *   mcpwm_foc_measure_inductance*            → MeasureL → BSP_HFI (六矢量 HFI + FFT)
 *   conf_general_measure_flux_linkage_openloop → MeasureFlux
 *   mcpwm_foc_encoder_detect (简化)          → MeasureEncoder (AS5600 三轴锁相)
 *   measure_r_l_imax / detect_apply_all_foc  → RunAll
 *   conf_general_calc_apply_foc_cc_kp_ki_gain → kp/ki (tc=1000µs, 与 detect_apply 一致)
 *
 * 未移植 (硬件/架构不具备): 无驱 coasting 磁链、Hall 表、
 * CAN 多机、EEPROM 持久化、电机温度补偿、旧版 BLDC detect_motor_param。
 * HFI 电感已按 VESC SIX_VECTOR + FFT bin0/bin2 实现 (见 BSP_HFI.c)。
 */
#ifndef BSP_MOTOR_DETECT_H
#define BSP_MOTOR_DETECT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BSP_DETECT_OK              0
#define BSP_DETECT_ERR_BUSY       -1
#define BSP_DETECT_ERR_CURRENT    -2
#define BSP_DETECT_ERR_VBUS       -3
#define BSP_DETECT_ERR_TIMEOUT    -4
#define BSP_DETECT_ERR_PARAM      -5
#define BSP_DETECT_ERR_FLUX       -6
#define BSP_DETECT_ERR_RUNNING    -7
#define BSP_DETECT_ERR_ENCODER    -8

typedef struct
{
    float rs_ohm;        /* foc_motor_r */
    float ls_h;          /* foc_motor_l */
    float ls_uh;
    float ld_lq_diff_h;  /* foc_motor_ld_lq_diff = Lq-Ld (HFI) */
    float flux_wb;       /* foc_motor_flux_linkage */
    float i_meas_a;
    float i_max_a;       /* 按功耗推算, Apply 时写入 FOC Imax */
    float vbus_v;
    float kp;            /* L * bw, tc=1000µs */
    float ki;            /* R * bw */
    float observer_gain; /* 1e-3/λ² * 1e6 */
    float enc_offset_deg;/* AS5600 机械角 @ 电角度 0° */
    float enc_ratio;     /* ≈ 极对数 (电角度/机械角度) */
    uint8_t enc_inverted;
    uint8_t enc_ok;
    uint8_t valid;
    int8_t  status;
    uint8_t stage;       /* 0 idle 1 R 2 L 3 flux 4 enc 5 done */
} BSP_MotorDetect_Result_t;

void BSP_MotorDetect_Init(void);

/** 完整流程: DC校准 → R → L → Flux → Encoder → kp/ki → 可 Apply */
int BSP_MotorDetect_RunAll(float max_power_loss);

int BSP_MotorDetect_MeasureR(float current_a, int samples, float *r_ohm);
int BSP_MotorDetect_MeasureL(float current_goal_a, int samples,
                             float *l_uh, float *ld_lq_diff_uh);
int BSP_MotorDetect_MeasureFlux(float current_a, float erpm_target,
                                float rs, float ls, float *flux_wb);
/** 三轴 DC 锁相测 AS5600 offset / ratio / inverted */
int BSP_MotorDetect_MeasureEncoder(float current_a);

const BSP_MotorDetect_Result_t *BSP_MotorDetect_GetResult(void);

/** 写回 Rs/Ls/Flux, 并把 i_max 应用到 FOC Imax (对齐 VESC l_current_max) */
void BSP_MotorDetect_Apply(void);

float BSP_MotorDetect_GetRs(void);
float BSP_MotorDetect_GetLs(void);
float BSP_MotorDetect_GetFlux(void);
float BSP_MotorDetect_GetEncOffsetDeg(void);
uint8_t BSP_MotorDetect_GetEncInverted(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_MOTOR_DETECT_H */
