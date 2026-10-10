/**
 * @file    BSP_HallBLDC.c
 * @brief   HALLTIM 捕获中断 → 读霍尔 → ATIM 六步换相
 */
#include "BSP_HallBLDC.h"
#include "BSP_MOTOR.h"
#include "cw32l012.h"
#include "cw32l012_gpio.h"
#include "cw32l012_halltim.h"
#include "cw32l012_sysctrl.h"

/*
 * 霍尔状态 (Ha<<0 | Hb<<1 | Hc<<2) → 六步序号
 * 正转表按 120° 标准相序；反转用对称表。
 * 若实际转向反了，改 Motor_Dir 或交换表。
 */
static const int8_t s_hall2step_fwd[8] = {
    -1, /* 000 */
     0, /* 001 Ha */
     2, /* 010 Hb */
     1, /* 011 */
     4, /* 100 Hc */
     5, /* 101 */
     3, /* 110 */
    -1  /* 111 */
};

static const int8_t s_hall2step_rev[8] = {
    -1,
     3, /* 001 */
     5, /* 010 */
     4, /* 011 */
     1, /* 100 */
     0, /* 101 */
     2, /* 110 */
    -1
};

static volatile uint8_t  s_run;
static volatile int8_t   s_dir = 1;      /* +1 / -1 */
static volatile uint8_t  s_hall;
static volatile uint8_t  s_step;
static volatile uint8_t  s_bad;
static volatile uint16_t s_fault;
static volatile uint32_t s_period;       /* HALLTIM WIDTH 快照 */

static void hall_gpio_init(void)
{
    GPIO_InitTypeDef g = {0};

    __SYSCTRL_GPIOB_CLK_ENABLE();

    g.Mode = GPIO_MODE_INPUT;
    g.IT = GPIO_IT_NONE;
    g.Pins = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_Init(CW_GPIOB, &g);

    PB13_AFx_HALLTIMCH1();
    PB14_AFx_HALLTIMCH2();
    PB15_AFx_HALLTIMCH3();
}

static uint8_t read_hall_filtered(void)
{
    uint8_t ha = (uint8_t)HALLTIM_GetFilteredStateCH1();
    uint8_t hb = (uint8_t)HALLTIM_GetFilteredStateCH2();
    uint8_t hc = (uint8_t)HALLTIM_GetFilteredStateCH3();
    return (uint8_t)((ha & 1U) | ((hb & 1U) << 1) | ((hc & 1U) << 2));
}

static void commute_from_hall(uint8_t hall)
{
    int8_t step;
    const int8_t *tab = (s_dir >= 0) ? s_hall2step_fwd : s_hall2step_rev;

    s_hall = hall;
    if ((hall == 0U) || (hall == 7U))
    {
        if (s_bad < 20U)
        {
            s_bad++;
        }
        if (s_bad >= 10U)
        {
            s_fault = 1U;
            BSP_MOTOR_Stop();
            s_run = 0U;
        }
        return;
    }

    s_bad = 0U;
    step = tab[hall];
    if (step < 0)
    {
        return;
    }
    s_step = (uint8_t)step;
    if (s_run != 0U)
    {
        BSP_MOTOR_CommutateStep(s_step);
    }
}

void BSP_HallBLDC_Init(void)
{
    HALLTIM_InitTypeDef ht = {0};

    hall_gpio_init();
    BSP_MOTOR_Init();

    ht.filter_enable = TRUE;
    ht.filter_length = 64U;
    ht.clock_division = HALLTIM_DIV_PCLK_DIV8;
    ht.trigger_output = HALLTIM_MMS_CHANGE;
    ht.auto_reload_value = 0x00FFFFFFUL;
    HALLTIM_Init(&ht);

    /* 霍尔信号变化中断（CAPIE = CHG） */
    CW_HALLTIM->DIER = (uint32_t)HALLTIM_DIER_CHG;
    HALLTIM_ClearStatus(HALLTIM_ISR_CHG | HALLTIM_ISR_OV | HALLTIM_ISR_MATCH);

    NVIC_ClearPendingIRQ(BTIM3_HALLTIM_IRQn);
    NVIC_SetPriority(BTIM3_HALLTIM_IRQn, 0U);
    NVIC_EnableIRQ(BTIM3_HALLTIM_IRQn);

    HALLTIM_Cmd(ENABLE);

    s_run = 0U;
    s_dir = 1;
    s_fault = 0U;
    s_bad = 0U;
    s_hall = read_hall_filtered();
    s_step = 0U;
}

void BSP_HallBLDC_Start(void)
{
    s_fault = 0U;
    s_bad = 0U;
    s_run = 1U;
    BSP_MOTOR_Start();
    /* 按当前霍尔立刻上电换相，避免空等下一个边沿 */
    commute_from_hall(read_hall_filtered());
}

void BSP_HallBLDC_Stop(void)
{
    s_run = 0U;
    BSP_MOTOR_Stop();
}

void BSP_HallBLDC_SetDutyPermille(uint16_t permille)
{
    uint32_t d;

    if (permille > 1000U)
    {
        permille = 1000U;
    }
    d = ((uint32_t)permille * (uint32_t)BSP_MOTOR_PWM_ARR) / 1000U;
    BSP_MOTOR_SetDuty((uint16_t)d);
}

void BSP_HallBLDC_SetDirection(int8_t dir)
{
    s_dir = (dir >= 0) ? 1 : -1;
    if (s_run != 0U)
    {
        commute_from_hall(read_hall_filtered());
    }
}

void BSP_HallBLDC_ToggleDirection(void)
{
    BSP_HallBLDC_SetDirection((int8_t)(-s_dir));
}

int8_t BSP_HallBLDC_GetDirection(void)
{
    return s_dir;
}

void BSP_HallBLDC_IRQHandler(void)
{
    uint32_t st = HALLTIM_GetStatus();

    if ((st & (uint32_t)HALLTIM_ISR_CHG) != 0U)
    {
        s_period = HALLTIM_GetPulseWidth();
        HALLTIM_ClearStatus(HALLTIM_ISR_CHG);
        commute_from_hall(read_hall_filtered());
    }
    if ((st & (uint32_t)HALLTIM_ISR_OV) != 0U)
    {
        HALLTIM_ClearStatus(HALLTIM_ISR_OV);
    }
    if ((st & (uint32_t)HALLTIM_ISR_MATCH) != 0U)
    {
        HALLTIM_ClearStatus(HALLTIM_ISR_MATCH);
    }
}

uint8_t BSP_HallBLDC_GetHallState(void)
{
    return s_hall;
}

uint8_t BSP_HallBLDC_GetStep(void)
{
    return s_step;
}

uint8_t BSP_HallBLDC_IsRunning(void)
{
    return s_run;
}

uint32_t BSP_HallBLDC_GetHallPeriod(void)
{
    return s_period;
}

uint16_t BSP_HallBLDC_GetFault(void)
{
    return s_fault;
}
