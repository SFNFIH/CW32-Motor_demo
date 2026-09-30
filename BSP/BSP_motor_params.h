/**
 * @file    BSP_motor_params.h
 * @brief   DengFOC 2208 (Doc/产品参数)
 */
#ifndef BSP_MOTOR_PARAMS_H
#define BSP_MOTOR_PARAMS_H

#ifdef __cplusplus
extern "C" {
#endif

#define MOTOR_MODEL_NAME          "DengFOC 2208"
#define MOTOR_POLE_PAIRS          7
#define MOTOR_RS_OHM              8.0f
#define MOTOR_LS_H                0.00425f
#define MOTOR_FLUX_WB             0.0f   /* 自整定写入运行时; 手册未给时默认 0 */
#define MOTOR_VBUS_V              12.0f
#define MOTOR_V_MAX               12.0f

/* 低侧 10mΩ, 差分运放 10k/1k=10, 同相: Vout = 2.5 + 10*I*R; ADC Vref=AVDD=5V */
#define CUR_SHUNT_OHM             0.010f
#define CUR_AMP_GAIN              10.0f
#define CUR_VREF_V                5.0f

/* 自整定默认最大功耗 (W), 用于推算注入电流 / i_max */
#define MOTOR_DETECT_MAX_LOSS_W   5.0f

#ifdef __cplusplus
}
#endif

#endif /* BSP_MOTOR_PARAMS_H */
