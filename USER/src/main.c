/**
 * @file    main.c
 * @brief   SguanFOC v3.1.0 无感 SMO: 电位器给转速, 按键启停
 */
#include "main.h"
#include "BSP_UART.h"
#include "BSP_Button.h"
#include "BSP_Potentiometer.h"
#include "BSP_MOTOR.h"
#include "BSP_Current.h"
#include "BSP_led.h"
#include "cw32l012_btim.h"
#include "cw32l012_atim.h"
#include "SguanFOC.h"
#include "Sguan_MotorStatus.h"

#define APP_HCLK_HZ   96000000U
#define POT_LO        100U
#define POT_HI        4000U
/* 电位器 → 机械角速度 rad/s (约 0~955 rpm) */
#define SPEED_MAX_RAD 100.0f

volatile uint32_t g_millis;

static void SYSCTRL_Configuration(void);
static void BTIM1_1ms_Init(void);
static void OnClick(void);
static void OnDoubleClick(void);
static void Uart_PollRx(void);
static float PotToSpeedRad(uint16_t adc);

static volatile uint8_t s_req_run;
static volatile uint8_t s_req_dir;
static int8_t s_speed_sign = 1;

void InitTick(uint32_t HclkFreq)
{
    SysTick->LOAD = (HclkFreq / 1000U) - 1U;
    SysTick->VAL = 0U;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
}

static float PotToSpeedRad(uint16_t adc)
{
    float x;

    if (adc <= POT_LO)
    {
        return 0.0f;
    }
    if (adc >= POT_HI)
    {
        return SPEED_MAX_RAD;
    }
    x = ((float)adc - (float)POT_LO) * SPEED_MAX_RAD /
        ((float)POT_HI - (float)POT_LO);
    return x;
}

static void OnClick(void)
{
    s_req_run = 1U;
}

static void OnDoubleClick(void)
{
    s_req_dir = 1U;
}

static void Uart_PollRx(void)
{
    static uint8_t buf[64];
    static uint16_t len;

    while ((CW_UART1->ISR & UARTx_ISR_RC_Msk) != 0U)
    {
        uint8_t ch = (uint8_t)CW_UART1->RDR;
        CW_UART1->ICR = UARTx_ICR_RC_Msk;

        if (len < (uint16_t)(sizeof(buf) - 1U))
        {
            buf[len++] = ch;
        }
        if (ch == (uint8_t)'?')
        {
            SguanFOC_Printf_Loop(buf, len);
            len = 0U;
        }
        else if ((ch == (uint8_t)'\n') || (ch == (uint8_t)'\r'))
        {
            len = 0U;
        }
    }
}

int main(void)
{
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
    BSP_MOTOR_EnablePwmIrq();

    while (1)
    {
        float speed_rad;

        SguanFOC_main_Loop();
        Uart_PollRx();

        if (s_req_run != 0U)
        {
            s_req_run = 0U;
            if ((Sguan.status == MOTOR_STATUS_STANDBY) ||
                (Sguan.status == MOTOR_STATUS_DISABLED))
            {
                Sguan.Func_Start();
            }
            else if (Sguan.status >= MOTOR_STATUS_IDLE)
            {
                Sguan.Func_Stop();
                BSP_MOTOR_Stop();
                BSP_LED_Off();
            }
        }

        if (s_req_dir != 0U)
        {
            s_req_dir = 0U;
            s_speed_sign = (int8_t)(-s_speed_sign);
        }

        speed_rad = PotToSpeedRad(BSP_Potentiometer_Read());
        if (Sguan.status >= MOTOR_STATUS_IDLE)
        {
            Sguan.Func_Set_Velocity((float)s_speed_sign * speed_rad);
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
    NVIC_SetPriority(BTIM1_IRQn, 2U);
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
