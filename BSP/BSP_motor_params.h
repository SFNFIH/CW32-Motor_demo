/**
 * @file    BSP_motor_params.h
 * @brief   4006 KV380 (产品表: 极数14 / 电阻170mΩ / KV360 / 4-6S)
 */
#ifndef BSP_MOTOR_PARAMS_H
#define BSP_MOTOR_PARAMS_H

#ifdef __cplusplus
extern "C" {
#endif

#define MOTOR_MODEL_NAME          "4006 KV380"
#define MOTOR_POLE_PAIRS          7          /* 转子极数 14 */
#define MOTOR_RS_OHM              0.17f      /* 170 mΩ (产品表) */
/* 产品表未给电感，按同级 4006 粗估，建议辨识后覆盖 */
#define MOTOR_LS_H                55e-6f
#define MOTOR_KV                  360.0f     /* 产品表 KV；型号名 KV380 */
/* ψ ≈ 60 / (√3 · π · KV · Pp) */
#define MOTOR_FLUX_WB             0.00437f
#define MOTOR_I_NOLOAD_A          0.3f
#define MOTOR_I_CONT_A            18.0f      /* 最大连续 60s */
#define MOTOR_VBUS_V              16.0f      /* 默认按 4S；支持 4-6S */
#define MOTOR_V_MAX               22.2f      /* 6S 上限参考 */

/* 低侧 10mΩ, 外接反相增益约 10; ADC Vref=AVDD=5V */
#define CUR_SHUNT_OHM             0.010f
#define CUR_AMP_GAIN              10.0f
#define CUR_VREF_V                5.0f

#ifdef __cplusplus
}
#endif

#endif /* BSP_MOTOR_PARAMS_H */
