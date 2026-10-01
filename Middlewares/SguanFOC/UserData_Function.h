#ifndef __USERDATA_FUNCTION_H
#define __USERDATA_FUNCTION_H
#include <stdint.h>
#include "main.h"
#include "BSP_MOTOR.h"
#include "BSP_Current.h"
#include "BSP_UART.h"
#include "BSP_led.h"
#include "SguanFOC.h"

extern volatile uint32_t g_millis;

static inline void User_Delay(uint32_t ms);

static inline void User_Initial_Init(void)
{
    /* PWM / 1ms 中断在 main 里已打开；此处可补二次使能 */
    BSP_MOTOR_EnablePwmIrq();
}

static inline void User_StartMotor_Init(void)
{
    BSP_Current_Calibrate();
    BSP_MOTOR_Start();
    BSP_LED_On();
}

static inline void User_Delay(uint32_t ms)
{
    uint32_t start = g_millis;
    while ((g_millis - start) < ms)
    {
    }
}

static inline int32_t User_ReadADC_Raw(int32_t Current_CH)
{
    switch (Current_CH)
    {
    case 0:
        return (int32_t)BSP_Current_GetRawA();
    case 1:
        return (int32_t)BSP_Current_GetRawB();
    default:
        return 0;
    }
}

static inline float User_Encoder_ReadRad(void)
{
    /* 无感运行不依赖编码器；启动对齐若读到 0 则保持 Motor_Dir */
    return 0.0f;
}

static inline uint8_t User_Encoder_ReadHall(uint8_t CH)
{
    (void)CH;
    return 0U;
}

static inline void User_PwmDuty_Set(uint32_t Duty_u,
                                    uint32_t Duty_v,
                                    uint32_t Duty_w)
{
    BSP_MOTOR_SetPhaseDuty((uint16_t)Duty_u,
                           (uint16_t)Duty_v,
                           (uint16_t)Duty_w);
}

static inline float User_VBUS_DataGet(void)
{
    /* 使用标定母线电压；避免与电位器抢 ADC2 */
    return (float)0xFF800000;
}

static inline float User_Temperature_DataGet(void)
{
    return (float)0xFF800000;
}

static inline void User_CorrespondSet(uint8_t *ch, uint16_t size)
{
    BSP_UART_Write(ch, (uint32_t)size);
}

#endif /* __USERDATA_FUNCTION_H */
