/**
 * @file    main.c
 * @brief   AS5600 速度/角度闭环: 单击启停, 双击切换速度环↔角度环
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
/** 电位器 ADC → 角度 raw 0..4095 (一圈目标; 闭环用 AS5600 绝对 cum) */
static uint16_t PotToAngleRaw(uint16_t adc);
static void OnClick(void);
static void OnDoubleClick(void);

static volatile uint8_t s_req_run;
static volatile uint8_t s_req_mode;
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

static uint16_t PotToAngleRaw(uint16_t adc)
{
    /* 0..4095 → 角度 raw 0..4095 (与 AS5600 一圈对齐); ≥4095 夹到满量程 */
    if (adc >= 4095U)
    {
        return 4095U;
    }
    return adc;
}

static void OnClick(void)
{
    s_req_run = 1U;
}

static void OnDoubleClick(void)
{
    s_req_mode = 1U;
}

int main(void)
{
    float telemetry[8];
    uint16_t pot;
    uint16_t speed;
    uint16_t ang_raw;

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
            /* 调试口复用为模式切换 (不再换向) */
            s_req_mode = 1U;
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
        if (s_req_mode != 0U)
        {
            s_req_mode = 0U;
            BSP_FOC_ToggleCtrlMode();
        }

        pot = BSP_Potentiometer_Read();
        speed = PotToSpeed(pot);
        ang_raw = PotToAngleRaw(pot);
        if (g_force_duty != 0U)
        {
            speed = g_force_duty;
            if (speed > 1000U)
            {
                speed = 1000U;
            }
            /* 强制占空比时也映射到角度 raw, 便于脚本测角度环 */
            ang_raw = (uint16_t)(((uint32_t)speed * 4095UL) / 1000UL);
        }

        BSP_AS5600_Update();
        as = BSP_AS5600_GetState();
        BSP_FOC_OnEncoder(as->raw, (as->ok != 0U) && (as->mag_ok != 0U) ? 1U : 0U);
        BSP_FOC_OnEncoderRpm(as->rpm_x10);
        BSP_FOC_OnEncoderCum(as->cum_raw);

        if (s_motor_on != 0U)
        {
            if (BSP_FOC_GetCtrlMode() == BSP_FOC_CTRL_ANGLE)
            {
                BSP_FOC_SetAngleRaw(ang_raw);
            }
            else
            {
                BSP_FOC_SetSpeed(speed);
            }
            BSP_FOC_SpeedLoop();
        }

        st = BSP_FOC_GetState();
        if ((s_motor_on != 0U) && (st->running == 0U))
        {
            BSP_LED_Off();
            s_motor_on = 0U;
        }
        BSP_DebugSnap_Publish(st, pot, s_motor_on);

        /* VOFA: mode, id, iq, theta, as_raw, rpm, ctrl/speed, pot
         * 速度环: id=fe, iq=rpm_ref, omega 在 id 旁用 ctrl 通道
         * 角度环: id=amp, iq=目标角°, omega=实测角° → 通道6 放 ctrl_mode */
        telemetry[0] = (float)st->mode;
        telemetry[1] = st->id_a;
        telemetry[2] = st->iq_a;
        telemetry[3] = st->theta;
        telemetry[4] = (float)as->raw;
        telemetry[5] = st->rpm;
        telemetry[6] = (BSP_FOC_GetCtrlMode() == BSP_FOC_CTRL_ANGLE) ?
                       st->omega_e : (float)speed;
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
