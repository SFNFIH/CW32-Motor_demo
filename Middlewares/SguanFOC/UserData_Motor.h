#ifndef __USERDATA_MOTOR_H
#define __USERDATA_MOTOR_H
#include "SguanFOC.h"
#include "BSP_MOTOR.h"
#include "BSP_motor_params.h"

/**
 * DengFOC 2208 + CW32L012 驱动板参数
 * 电流: A/B 下桥臂分流 → OPA → ADC1 CH8/CH9
 */
static inline void User_Motor_Init(SguanFOC_System_STRUCT *user){
    user->foc.Target_Speed = 0.0f;
    user->foc.Target_Pos = 0.0f;
    user->foc.Target_Id = 0.0f;
    user->foc.Target_Iq = 0.0f;

    user->foc.Target_VF_Uq = 0.0f;
    user->foc.Target_IF_Iq = 0.45f;   /* IF 强拖电流 (A)，按空载再调 */

    user->foc.Ud_in = 0.0f;
    user->foc.Uq_in = 0.0f;

    /* 粗略初值，可后续用辨识覆盖 */
    user->motor.identify.Rs = MOTOR_RS_OHM;
    user->motor.identify.Ld = MOTOR_LS_H;
    user->motor.identify.Lq = MOTOR_LS_H;
    user->motor.identify.Flux = 0.0045f;
    user->motor.identify.B = 0.0001f;
    user->motor.identify.J = 0.00002f;

    user->motor.Poles = (uint8_t)MOTOR_POLE_PAIRS;
    user->motor.VBUS = MOTOR_VBUS_V;

    user->motor.Motor_Dir = 1;
    user->motor.Encoder_Dir = 1;
    user->motor.PWM_Dir = 1;          /* 高电平有效占空比 */
    user->motor.Duty = BSP_MOTOR_PWM_ARR;

    /* 外接反相运放: 正电流 → ADC 下降 → Current_Dir = -1 */
    user->motor.Current_Dir0 = -1;
    user->motor.Current_Dir1 = -1;
    user->motor.Current_Num = 0;      /* AB 相采样 */
    user->motor.ADC_Precision = 4096U;
    user->motor.Amplifier = CUR_AMP_GAIN;
    user->motor.MCU_Voltage = CUR_VREF_V;
    user->motor.Sampling_Rs = CUR_SHUNT_OHM;

    user->safe.VBUS_MAX = 16.0f;
    user->safe.VBUS_MIM = 8.0f;
    user->safe.VBUS_watchdog_limit = 1000U;

    user->safe.Temp_MAX = 80.0f;
    user->safe.Temp_MIN = -20.0f;
    user->safe.Temp_watchdog_limit = 1000U;

    user->safe.Dcur_MAX = 2.5f;
    user->safe.Qcur_MAX = 2.5f;
    user->safe.DQcur_watchdog_limit = 1000U;

    user->safe.Current_limit = 0.15f;
    user->safe.Speed_limit = 8.0f;
    user->safe.Position_limit = 1.0f;

    user->safe.DISABLED_watchdog_limit = 1000U;

    user->flag.PWM_watchdog_limit = 20;
}

#endif /* __USERDATA_MOTOR_H */
