#ifndef __USERDATA_CONFIG_H
#define __USERDATA_CONFIG_H
/* CW32L012 + DengFOC 2208 — SguanFOC v3.1.0 无感配置 */

/**
 * 无感滑模观测 (IF 开环启动 → SMO)
 * 需要电流采样；不依赖运行时编码器
 */
#define Define_Run_Mode 8

/** 速度环用 PID（M0+ 软浮点更轻） */
#define Switch_MOTOR_Control_Vel 0

#define Switch_MOTOR_Control_Pos 0

/** 0 = SVPWM */
#define Switch_MOTOR_PWM 0

#define Switch_MOTOR_Filter 0
#define Switch_MOTOR_Identify 0
#define Switch_MOTOR_Start 0
#define Switch_Printf_Debug 0
#define Switch_Cogging_Calculate 0

#define Open_AngleComp_Calculate 0
#define Open_Current_Feedforward 1
#define Open_Velocity_Feedforward 0
#define Open_DOB_Calculate 0
#define Open_Inhibit_Calculate 0
#define Open_MTPA_Calculate 0
#define Open_FW_Calculate 0
#define Open_DeadZone_Calculate 0

#define BASE_Cogging_Num 8.0f

/* 10 kHz 电流环 (中心对齐 PWM @ 96 MHz, ARR=4799) */
#define TIM_T 1e-4f

#endif /* __USERDATA_CONFIG_H */
