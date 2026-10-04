/**
 * @file    main.c
 * @brief   SguanFOC VF 开环：上电启动，AS5600 在 ESP32 侧 ttyACM0 观察
 */
#include "main.h"
#include "BSP_UART.h"
#include "BSP_Button.h"
#include "BSP_Potentiometer.h"
#include "BSP_MOTOR.h"
#include "BSP_Current.h"
#include "BSP_led.h"
#include "BSP_motor_params.h"
#include "cw32l012_btim.h"
#include "SguanFOC.h"
#include "Sguan_MotorStatus.h"
#include <stdio.h>

#define APP_HCLK_HZ 96000000U

volatile uint32_t g_millis;

static void SYSCTRL_Configuration(void);
static void BTIM1_1ms_Init(void);
static void OnClick(void);
static void OnDoubleClick(void);
static int8_t s_speed_sign = 1;
static uint8_t s_started;

void InitTick(uint32_t HclkFreq)
{
    SysTick->LOAD = (HclkFreq / 1000U) - 1U;
    SysTick->VAL = 0U;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
}

static void AppStart(void)
{
    Sguan.motor.PWM_Dir = 1;
    Sguan.motor.Motor_Dir = 1;
    Sguan.motor.Duty = BSP_MOTOR_PWM_ARR;
    Sguan.motor.VBUS = MOTOR_VBUS_V;
    Sguan.foc.Real_VBUS = MOTOR_VBUS_V;
    Sguan.foc.Target_VF_Uq = 2.5f;
    BSP_MOTOR_Start();
    BSP_LED_On();
    Sguan.Func_Start();
    SguanFOC_main_Loop();
    Sguan.foc.Target_VF_Uq = 2.5f;
    Sguan.Func_Set_Velocity(60.0f);
    BSP_MOTOR_EnablePwmIrq();
    s_started = 1U;
}

static void AppStop(void)
{
    Sguan.Func_Stop();
    BSP_MOTOR_DisablePwmIrq();
    BSP_MOTOR_Stop();
    BSP_LED_Off();
    s_started = 0U;
}

void App_Tick1ms(void)
{
    static uint8_t pot_div;
    uint16_t adc;
    float spd;

    BSP_Button_Tick1ms();

    pot_div++;
    if (pot_div < 10U)
    {
        return;
    }
    pot_div = 0U;

    if ((s_started == 0U) ||
        (Sguan.status < MOTOR_STATUS_IDLE) ||
        (Sguan.status >= MOTOR_STATUS_OVERVOLTAGE))
    {
        return;
    }

    adc = BSP_Potentiometer_Read();
    if (adc < 100U)
    {
        spd = 50.0f;
    }
    else if (adc > 4000U)
    {
        spd = 120.0f;
    }
    else
    {
        spd = 50.0f + ((float)(adc - 100U) * 70.0f) / 3900.0f;
    }
    Sguan.foc.Target_VF_Uq = 2.5f;
    Sguan.Func_Set_Velocity((float)s_speed_sign * spd);
}

static void OnClick(void)
{
    if (s_started != 0U)
    {
        AppStop();
    }
    else
    {
        AppStart();
    }
}

static void OnDoubleClick(void)
{
    s_speed_sign = (int8_t)(-s_speed_sign);
}

int main(void)
{
    uint32_t last_tx = 0U;

    SYSCTRL_Configuration();
    InitTick(APP_HCLK_HZ);

    BSP_LED_Init();
    BSP_UART_Init();
    BSP_Potentiometer_Init();
    BSP_Current_Init();
    BSP_MOTOR_Init();
    BSP_Button_Init();
    BSP_Button_SetClickCallback(OnClick);
    BSP_Button_SetDoubleClickCallback(OnDoubleClick);
    BTIM1_1ms_Init();

    AppStart();

    while (1)
    {
        char line[96];
        int n;

        SguanFOC_main_Loop();

        if ((g_millis - last_tx) >= 200U)
        {
            last_tx = g_millis;
            n = snprintf(line, sizeof(line),
                         "st=%u spd=%d uq=%d dir=%d\r\n",
                         (unsigned)Sguan.status,
                         (int)Sguan.encoder.Real_Speed,
                         (int)(Sguan.foc.Uq_in * 100.0f),
                         (int)s_speed_sign);
            if (n > 0)
            {
                BSP_UART_Write(line, (uint32_t)n);
            }
        }
    }
}

static void BTIM1_1ms_Init(void)
{
    BTIM_TimeBaseInitTypeDef tb = {0};

    __SYSCTRL_BTIM123_CLK_ENABLE();
    tb.BTIM_Mode = BTIM_MODE_TIMER;
    tb.BTIM_CountMode = BTIM_COUNT_MODE_REPETITIVE;
    tb.BTIM_Prescaler = 95U;
    tb.BTIM_Period = 999U;
    BTIM_TimeBaseInit(CW_BTIM1, &tb);
    BTIM_ITConfig(CW_BTIM1, BTIM_IT_UPDATE, ENABLE);
    BTIM_Cmd(CW_BTIM1, ENABLE);
    NVIC_SetPriority(BTIM1_IRQn, 0U);
    NVIC_EnableIRQ(BTIM1_IRQn);
}

static void SYSCTRL_Configuration(void)
{
    SYSCTRL_HSI_Enable(SYSCTRL_HSIOSC_DIV1);
    SYSCTRL_SysClk_Switch(SYSCTRL_SYSCLKSRC_HSI);
    SYSCTRL_PCLKPRS_Config(SYSCTRL_PCLK_DIV1);
    SystemCoreClock = APP_HCLK_HZ;
    __SYSCTRL_GPIOA_CLK_ENABLE();
    __SYSCTRL_GPIOB_CLK_ENABLE();
    __SYSCTRL_GPIOC_CLK_ENABLE();
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
}
#endif
