/**
 * @file    main.c
 * @brief   单击启停; 双击轮换 速度/角度/Imax;
 *          停机 + 电位器最低时双击 → 电机自整定 (VESC 风格 R/L/Flux)
 */
#include "main.h"
#include "BSP_UART.h"
#include "BSP_Button.h"
#include "BSP_Potentiometer.h"
#include "BSP_FOC.h"
#include "BSP_AS5600.h"
#include "BSP_DebugSnap.h"
#include "BSP_MotorDetect.h"

#define APP_HCLK_HZ     96000000U
#define POT_LO          100U
#define POT_HI          4000U
#define LED_BLINK_MS    160U
#define DETECT_POT_MAX  250U
#define DETECT_PWR_LOSS 5.0f

static void SYSCTRL_Configuration(void);
static void DelayMs(uint32_t ms);
static uint16_t PotToSpeed(uint16_t adc);
static uint16_t PotToAngleRaw(uint16_t adc);
static uint16_t PotToImaxPm(uint16_t adc);
static void OnClick(void);
static void OnDoubleClick(void);
static void UpdateLed(uint8_t motor_on);
static void RunMotorDetect(void);

static volatile uint8_t s_req_run;
static volatile uint8_t s_req_mode;
static volatile uint8_t s_req_detect;
static uint8_t s_motor_on;
static uint8_t s_detect_done;
extern volatile uint32_t g_millis;

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
    if (adc >= 4095U)
    {
        return 4095U;
    }
    return adc;
}

static uint16_t PotToImaxPm(uint16_t adc)
{
    return PotToSpeed(adc);
}

static void OnClick(void)
{
    s_req_run = 1U;
}

static void OnDoubleClick(void)
{
    s_req_mode = 1U;
}

static void UpdateLed(uint8_t motor_on)
{
    static uint32_t s_blink_ms;

    if (motor_on == 0U)
    {
        if (s_detect_done != 0U)
        {
            /* 自整定成功: 慢闪提示 */
            if ((g_millis - s_blink_ms) >= 400U)
            {
                s_blink_ms = g_millis;
                BSP_LED_Tog();
            }
            return;
        }
        BSP_LED_Off();
        return;
    }
    if (BSP_FOC_GetCtrlMode() == BSP_FOC_CTRL_CURRENT)
    {
        if ((g_millis - s_blink_ms) >= LED_BLINK_MS)
        {
            s_blink_ms = g_millis;
            BSP_LED_Tog();
        }
        return;
    }
    BSP_LED_On();
}

static void RunMotorDetect(void)
{
    int rc;
    uint32_t t;

    if (s_motor_on != 0U)
    {
        BSP_FOC_Stop();
        s_motor_on = 0U;
    }

    /* 快闪表示正在整定 */
    for (t = 0U; t < 6U; t++)
    {
        BSP_LED_Tog();
        DelayMs(40U);
    }

    rc = BSP_MotorDetect_RunAll(DETECT_PWR_LOSS);
    if (rc == BSP_DETECT_OK || BSP_MotorDetect_GetResult()->valid != 0U)
    {
        BSP_MotorDetect_Apply();
        s_detect_done = 1U;
        BSP_LED_On();
    }
    else
    {
        s_detect_done = 0U;
        /* 失败: 快速闪三下 */
        for (t = 0U; t < 6U; t++)
        {
            BSP_LED_Tog();
            DelayMs(80U);
        }
        BSP_LED_Off();
    }
}

int main(void)
{
    float telemetry[10];
    uint16_t pot;
    uint16_t speed;
    uint16_t ang_raw;
    uint16_t imax_pm;
    uint8_t ctrl;

    SYSCTRL_Configuration();
    InitTick(APP_HCLK_HZ);

    BSP_LED_Init();
    BSP_UART_Init();
    BSP_Potentiometer_Init();
    BSP_AS5600_Init(APP_HCLK_HZ);
    BSP_FOC_Init();
    BSP_MotorDetect_Init();
    BSP_DebugSnap_Init();
    g_force_duty = 0U;
    BSP_Button_Init();
    BSP_Button_SetClickCallback(OnClick);
    BSP_Button_SetDoubleClickCallback(OnDoubleClick);

    while (1)
    {
        const BSP_FOC_State_t *st;
        const BSP_AS5600_State_t *as;
        const BSP_MotorDetect_Result_t *det;
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
            s_req_mode = 1U;
        }
        else if (cmd == DBG_CMD_DETECT)
        {
            s_req_detect = 1U;
        }

        if (s_req_detect != 0U)
        {
            s_req_detect = 0U;
            RunMotorDetect();
        }

        if (s_req_run != 0U)
        {
            s_req_run = 0U;
            s_detect_done = 0U;
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
            pot = BSP_Potentiometer_Read();
            /* 停机 + 电位器拧到最低: 双击触发自整定 */
            if ((s_motor_on == 0U) && (pot < DETECT_POT_MAX))
            {
                s_req_detect = 1U;
            }
            else
            {
                BSP_FOC_ToggleCtrlMode();
            }
        }

        pot = BSP_Potentiometer_Read();
        speed = PotToSpeed(pot);
        ang_raw = PotToAngleRaw(pot);
        imax_pm = PotToImaxPm(pot);
        if (g_force_duty != 0U)
        {
            speed = g_force_duty;
            if (speed > 1000U)
            {
                speed = 1000U;
            }
            ang_raw = (uint16_t)(((uint32_t)speed * 4095UL) / 1000UL);
            imax_pm = speed;
        }

        BSP_AS5600_Update();
        as = BSP_AS5600_GetState();
        BSP_FOC_OnEncoder(as->raw, (as->ok != 0U) && (as->mag_ok != 0U) ? 1U : 0U);
        BSP_FOC_OnEncoderRpm(as->rpm_x10);
        BSP_FOC_OnEncoderCum(as->cum_raw);

        ctrl = BSP_FOC_GetCtrlMode();
        if (ctrl == BSP_FOC_CTRL_CURRENT)
        {
            BSP_FOC_SetImaxPm(imax_pm);
        }
        if (s_motor_on != 0U)
        {
            if (ctrl == BSP_FOC_CTRL_ANGLE)
            {
                BSP_FOC_SetAngleRaw(ang_raw);
            }
            else if (ctrl == BSP_FOC_CTRL_SPEED)
            {
                BSP_FOC_SetSpeed(speed);
            }
            BSP_FOC_SpeedLoop();
        }
        else
        {
            BSP_FOC_PollCurrent();
        }

        st = BSP_FOC_GetState();
        if ((s_motor_on != 0U) && (st->running == 0U))
        {
            BSP_LED_Off();
            s_motor_on = 0U;
        }
        UpdateLed(s_motor_on);
        BSP_DebugSnap_Publish(st, pot, s_motor_on);

        det = BSP_MotorDetect_GetResult();
        telemetry[0] = (float)st->mode;
        telemetry[1] = st->id_a;
        telemetry[2] = st->iq_a;
        telemetry[3] = st->theta;
        telemetry[4] = (float)as->raw;
        telemetry[5] = st->rpm;
        if (ctrl == BSP_FOC_CTRL_CURRENT)
        {
            telemetry[6] = (float)BSP_FOC_GetImaxPm();
        }
        else if (ctrl == BSP_FOC_CTRL_ANGLE)
        {
            telemetry[6] = st->omega_e;
        }
        else
        {
            telemetry[6] = (float)speed;
        }
        telemetry[7] = (float)pot;
        if ((s_motor_on == 0U) && (det->valid != 0U))
        {
            /* 停机且已整定: ch8=Rs ch9=Ls; 另发 flux/kp/enc + LdLq diff */
            telemetry[8] = det->rs_ohm;
            telemetry[9] = det->ls_uh;
            if (det->enc_ok != 0U)
            {
                telemetry[3] = det->enc_offset_deg;
                telemetry[1] = det->enc_ratio;
                telemetry[2] = (float)det->enc_inverted;
            }
            telemetry[5] = det->flux_wb * 1000.0f; /* mWb */
            telemetry[6] = det->kp;
            telemetry[4] = det->ld_lq_diff_h * 1.0e6f; /* (Lq-Ld) µH */
        }
        else
        {
            telemetry[8] = st->i_lim_a;
            telemetry[9] = st->i_meas_a;
        }
        BSP_UART_SendJustFloat(telemetry, 10U);
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
