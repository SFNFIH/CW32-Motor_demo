/**
 * @file    main.c
 * @brief   AS5600 速度闭环: 电位器给转速目标, 磁编反馈
 */
#include "main.h"
#include "BSP_UART.h"
#include "BSP_Button.h"
#include "BSP_Potentiometer.h"
#include "BSP_FOC.h"
#include "BSP_AS5600.h"
#include "BSP_DebugSnap.h"

#define APP_HCLK_HZ   96000000U
#define POT_LO        100U
#define POT_HI        4000U

static void SYSCTRL_Configuration(void);
static void DelayMs(uint32_t ms);
static uint16_t PotToSpeed(uint16_t adc);
static void OnClick(void);
static void OnDoubleClick(void);

static volatile uint8_t s_req_run;
static volatile uint8_t s_req_dir;
static uint8_t s_motor_on;

void InitTick(uint32_t HclkFreq)
{
    SysTick->LOAD = (HclkFreq / 1000U) - 1U;
    SysTick->VAL = 0U;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
}

static uint16_t PotToSpeed(uint16_t adc)
{
    uint32_t x;

    if (adc <= POT_LO)
    {
        return 0U;
    }
    if (adc >= POT_HI)
    {
        return 1000U;
    }
    x = ((uint32_t)adc - POT_LO) * 1000U / (POT_HI - POT_LO);
    return (uint16_t)x;
}

static void OnClick(void)
{
    s_req_run = 1U;
}

static void OnDoubleClick(void)
{
    s_req_dir = 1U;
}

int main(void)
{
    float telemetry[8];
    uint16_t pot;
    uint16_t speed;

    SYSCTRL_Configuration();
    InitTick(APP_HCLK_HZ);

    BSP_LED_Init();
    BSP_UART_Init();
    BSP_Potentiometer_Init();
    BSP_AS5600_Init(APP_HCLK_HZ);
    BSP_FOC_Init();
    BSP_DebugSnap_Init();
    g_force_duty = 0U;
    BSP_Button_Init();
    BSP_Button_SetClickCallback(OnClick);
    BSP_Button_SetDoubleClickCallback(OnDoubleClick);

    while (1)
    {
        const BSP_FOC_State_t *st;
        const BSP_AS5600_State_t *as;
        uint32_t cmd = BSP_DebugSnap_TakeCmd();

        if (cmd == DBG_CMD_START)
        {
            s_req_run = 1U;
        }
        else if (cmd == DBG_CMD_STOP)
        {
            if (s_motor_on != 0U)
            {
                BSP_FOC_Stop();
                BSP_LED_Off();
                s_motor_on = 0U;
            }
        }
        else if (cmd == DBG_CMD_TOGGLE_DIR)
        {
            s_req_dir = 1U;
        }

        if (s_req_run != 0U)
        {
            s_req_run = 0U;
            if (s_motor_on != 0U)
            {
                BSP_FOC_Stop();
                BSP_LED_Off();
                s_motor_on = 0U;
            }
            else
            {
                s_motor_on = 1U;
                BSP_LED_On();
                BSP_FOC_Start();
            }
        }
        if (s_req_dir != 0U)
        {
            s_req_dir = 0U;
            BSP_FOC_ToggleDirection();
        }

        pot = BSP_Potentiometer_Read();
        speed = PotToSpeed(pot);
        if (g_force_duty != 0U)
        {
            speed = g_force_duty;
            if (speed > 1000U)
            {
                speed = 1000U;
            }
        }

        BSP_AS5600_Update();
        as = BSP_AS5600_GetState();
        BSP_FOC_OnEncoder(as->raw, (as->ok != 0U) && (as->mag_ok != 0U) ? 1U : 0U);
        BSP_FOC_OnEncoderRpm(as->rpm_x10);
        BSP_FOC_OnEncoderCum(as->cum_raw);

        if (s_motor_on != 0U)
        {
            BSP_FOC_SetSpeed(speed);
            BSP_FOC_SpeedLoop();
        }

        st = BSP_FOC_GetState();
        if ((s_motor_on != 0U) && (st->running == 0U))
        {
            BSP_LED_Off();
            s_motor_on = 0U;
        }
        BSP_DebugSnap_Publish(st, pot, s_motor_on);

        /* VOFA: mode, fe_x10, rpm_ref, theta, as_raw, rpm_meas, speed_pm, pot */
        telemetry[0] = (float)st->mode;
        telemetry[1] = st->id_a;          /* fe_x10 指令 */
        telemetry[2] = st->iq_a;          /* 目标 rpm */
        telemetry[3] = st->theta;
        telemetry[4] = (float)as->raw;
        telemetry[5] = st->rpm;           /* 实测 rpm */
        telemetry[6] = (float)speed;
        telemetry[7] = (float)pot;
        BSP_UART_SendJustFloat(telemetry, 8U);
        DelayMs(2U);
    }
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

static void DelayMs(uint32_t ms)
{
    while (ms--)
    {
        while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0U)
        {
        }
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
}
#endif
