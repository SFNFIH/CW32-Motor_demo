/**
 * @file    BSP_MotorDetect.h
 * @brief   电机参数自整定 (移植自 vedderb/bldc FOC detect 思路)
 *
 * VESC 对应关系:
 *   mcpwm_foc_measure_resistance      → BSP_MotorDetect_MeasureR
 *   mcpwm_foc_measure_inductance_*    → BSP_MotorDetect_MeasureL  (脉冲 di/dt, 非 HFI)
 *   conf_general_measure_flux_linkage → BSP_MotorDetect_MeasureFlux (开环 + AS5600)
 *   measure_r_l_imax / detect_apply   → BSP_MotorDetect_RunAll
 *   conf_general_calc_apply_foc_cc_*  → 结果中的 kp/ki (L*bw, R*bw)
 *
 * 本工程无 Id/Iq 电流环与 HFI, 用三相桥 DC 注入 + 相电流采样实现等价物理量。
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

typedef struct
{
    float rs_ohm;       /* 相电阻 (Ω), 对应 foc_motor_r */
    float ls_h;         /* 平均电感 (H), 对应 foc_motor_l */
    float ls_uh;        /* 同上, µH, 便于 VOFA */
    float flux_wb;      /* 磁链 (Wb), 对应 foc_motor_flux_linkage */
    float i_meas_a;     /* 测量时使用的电流 (A) */
    float i_max_a;      /* 按功耗推算的电流上限 (A) */
    float vbus_v;       /* 测量时母线电压 */
    float kp;           /* 电流环 kp = L * bw */
    float ki;           /* 电流环 ki = R * bw */
    float observer_gain;/* VESC: 1e-3/λ² * 1e6 */
    uint8_t valid;      /* 1=整定成功 */
    int8_t  status;     /* 0 或负错误码 */
    uint8_t stage;      /* 0 idle 1 R 2 L 3 flux 4 done */
} BSP_MotorDetect_Result_t;

void BSP_MotorDetect_Init(void);

/** 运行完整 R→L→Flux→增益 (电机必须停机). max_power_loss 建议 3~8 W */
int BSP_MotorDetect_RunAll(float max_power_loss);

/** 单独测相电阻; current_a 为注入目标电流; samples 为平均次数 */
int BSP_MotorDetect_MeasureR(float current_a, int samples, float *r_ohm);

/**
 * 电压脉冲测电感 (经典 di/dt; VESC 现用 HFI, 本板用脉冲等效).
 * 结果单位 µH; ld_lq_diff 本实现填 0 (无 HFI 分轴).
 */
int BSP_MotorDetect_MeasureL(float current_goal_a, int samples,
                             float *l_uh, float *ld_lq_diff_uh);

/** 开环转动估计磁链; 需已有 rs/ls; erpm_target 建议 2000~6000 */
int BSP_MotorDetect_MeasureFlux(float current_a, float erpm_target,
                                float rs, float ls, float *flux_wb);

const BSP_MotorDetect_Result_t *BSP_MotorDetect_GetResult(void);

/** 将成功结果写回运行时参数 (供后续控制使用) */
void BSP_MotorDetect_Apply(void);

float BSP_MotorDetect_GetRs(void);
float BSP_MotorDetect_GetLs(void);
float BSP_MotorDetect_GetFlux(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_MOTOR_DETECT_H */
