/**
 * @file    main.c
 * @brief   霍尔六步方波：换相全在 HALLTIM 中断，主循环只改占空比
 */
#include "main.h"
#include "BSP_UART.h"
#include "BSP_Button.h"
#include "BSP_Potentiometer.h"
#include "BSP_HallBLDC.h"
#include "BSP_led.h"
#include "cw32l012_btim.h"
#include <stdio.h>

#define APP_HCLK_HZ   96000000U
#define POT_LO        100U
#define POT_HI        4000U
#define DUTY_MIN_PM   80U     /* 千分比下限，保证能起转 */
#define DUTY_MAX_PM   700U    /* 千分比上限，留裕量 */

volatile uint32_t g_millis;

static void SYSCTRL_Configuration(void);
static void BTIM1_1ms_Init(void);
static void OnClick(void);
static void OnDoubleClick(void);
static uint16_t PotToDutyPm(uint16_t adc);

static volatile uint8_t s_req_run;
static volatile uint8_t s_req_dir;

void InitTick(uint32_t HclkFreq)
{
    SysTick->LOAD = (HclkFreq / 1000U) - 1U;
    SysTick->VAL = 0U;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
}

static uint16_t PotToDutyPm(uint16_t adc)
{
    uint32_t x;

    if (adc <= POT_LO)
    {
        return DUTY_MIN_PM;
    }
    if (adc >= POT_HI)
    {
        return DUTY_MAX_PM;
    }
    x = DUTY_MIN_PM +
        ((uint32_t)adc - POT_LO) * (DUTY_MAX_PM - DUTY_MIN_PM) /
        (POT_HI - POT_LO);
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
    uint32_t last_tx = 0U;

    SYSCTRL_Configuration();
    InitTick(APP_HCLK_HZ);

    BSP_LED_Init();
    BSP_UART_Init();
    BSP_Potentiometer_Init();
    BSP_Button_Init();
    BSP_Button_SetClickCallback(OnClick);
    BSP_Button_SetDoubleClickCallback(OnDoubleClick);
    BTIM1_1ms_Init();

    BSP_HallBLDC_Init();
    BSP_HallBLDC_SetDutyPermille(DUTY_MIN_PM);
    BSP_HallBLDC_Start();
    BSP_LED_On();

    while (1)
    {
        uint16_t duty_pm;

        /* —— 主循环唯一电机动作：改占空比 —— */
        duty_pm = PotToDutyPm(BSP_Potentiometer_Read());
        if (BSP_HallBLDC_IsRunning() != 0U)
        {
            BSP_HallBLDC_SetDutyPermille(duty_pm);
        }

        if (s_req_run != 0U)
        {
            s_req_run = 0U;
            if (BSP_HallBLDC_IsRunning() != 0U)
            {
                BSP_HallBLDC_Stop();
                BSP_LED_Off();
            }
            else
            {
                BSP_HallBLDC_SetDutyPermille(duty_pm);
                BSP_HallBLDC_Start();
                BSP_LED_On();
            }
        }

        if (s_req_dir != 0U)
        {
            s_req_dir = 0U;
            BSP_HallBLDC_ToggleDirection();
        }

        if (BSP_HallBLDC_GetFault() != 0U)
        {
            BSP_LED_Off();
        }

        if ((g_millis - last_tx) >= 200U)
        {
            char line[96];
            int n;

            last_tx = g_millis;
            n = snprintf(line, sizeof(line),
                         "run=%u hall=%u step=%u duty=%u dir=%d flt=%u\r\n",
                         (unsigned)BSP_HallBLDC_IsRunning(),
                         (unsigned)BSP_HallBLDC_GetHallState(),
                         (unsigned)BSP_HallBLDC_GetStep(),
                         (unsigned)duty_pm,
                         (int)BSP_HallBLDC_GetDirection(),
                         (unsigned)BSP_HallBLDC_GetFault());
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
