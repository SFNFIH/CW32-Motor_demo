/**
 * @file    BSP_MOTOR.c
 * @brief   ATIM 三相互补 PWM，六步方波：导通相 PWM/全开，浮空相关断
 */
#include "BSP_MOTOR.h"
#include "cw32l012.h"
#include "cw32l012_atim.h"
#include "cw32l012_gpio.h"
#include "cw32l012_sysctrl.h"

#define CC_ALL  (ATIM_CCER_CC1E_Msk | ATIM_CCER_CC1NE_Msk | \
                 ATIM_CCER_CC2E_Msk | ATIM_CCER_CC2NE_Msk | \
                 ATIM_CCER_CC3E_Msk | ATIM_CCER_CC3NE_Msk)

static volatile uint16_t s_duty = 0U;
static volatile uint8_t  s_step = 0xFFU;
static volatile uint8_t  s_run;

static void gpio_init(void)
{
    GPIO_InitTypeDef g = {0};

    __SYSCTRL_GPIOA_CLK_ENABLE();
    __SYSCTRL_GPIOB_CLK_ENABLE();

    g.Mode = GPIO_MODE_OUTPUT_PP;
    g.IT = GPIO_IT_NONE;
    g.Pins = GPIO_PIN_15;
    GPIO_Init(CW_GPIOA, &g);
    g.Pins = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_Init(CW_GPIOB, &g);

    PB05_AFx_ATIMCH1();
    PB06_AFx_ATIMCH2();
    PB07_AFx_ATIMCH3();
    PA15_AFx_ATIMCH1N();
    PB03_AFx_ATIMCH2N();
    PB04_AFx_ATIMCH3N();
}

static void outputs_off(void)
{
    CW_ATIM->CCER &= (uint32_t)~CC_ALL;
    ATIM_SetCompare1(0U);
    ATIM_SetCompare2(0U);
    ATIM_SetCompare3(0U);
}

void BSP_MOTOR_Init(void)
{
    ATIM_InitTypeDef tim;
    ATIM_OCInitTypeDef oc;

    gpio_init();
    __SYSCTRL_ATIM_CLK_ENABLE();
    ATIM_DeInit();

    /* 边沿对齐更适合方波六步占空控制 */
    tim.BufferState = ENABLE;
    tim.CounterAlignedMode = ATIM_COUNT_ALIGN_MODE_EDGE;
    tim.CounterDirection = ATIM_COUNTING_UP;
    tim.CounterOPMode = ATIM_OP_MODE_REPETITIVE;
    tim.Prescaler = 0U;
    tim.ReloadValue = BSP_MOTOR_PWM_ARR;
    tim.RepetitionCounter = 0U;
    ATIM_Init(&tim);

    oc.OCPolarity = ATIM_OCPOLARITY_NONINVERT;
    oc.OCMode = ATIM_OCMODE_PWM1;
    oc.OCFastMode = ATIM_OC_FAST_MODE_DISABLE;
    oc.OCInterruptState = DISABLE;
    oc.BufferState = ENABLE;
    oc.OCComplement = ENABLE;
    ATIM_OC1Init(&oc);
    ATIM_OC2Init(&oc);
    ATIM_OC3Init(&oc);

    ATIM_CH1Config(ENABLE);
    ATIM_CH2Config(ENABLE);
    ATIM_CH3Config(ENABLE);

    outputs_off();

    ATIM_SetPWMDeadtime((int16_t)BSP_MOTOR_PWM_DEADTIME,
                        (int16_t)BSP_MOTOR_PWM_DEADTIME,
                        DISABLE);

    CW_ATIM->BDTR_f.OSSR = 1U;
    CW_ATIM->BDTR_f.OSSI = 0U;
    CW_ATIM->BDTR_f.BKE = 0U;
    CW_ATIM->BDTR_f.BK2E = 0U;
    CW_ATIM->AF1_f.BKINE = 0U;

    ATIM_Cmd(ENABLE);
    ATIM_CtrlPWMOutputs(DISABLE);
    s_run = 0U;
    s_duty = 0U;
    s_step = 0xFFU;
}

void BSP_MOTOR_SetDuty(uint16_t duty)
{
    if (duty > BSP_MOTOR_PWM_ARR)
    {
        duty = BSP_MOTOR_PWM_ARR;
    }
    s_duty = duty;

    /*
     * 主循环只改占空比：刷新当前导通相的 CCR，不碰 CCER。
     * 六步换相（改桥臂开关组合）仅由 HALLTIM 中断调用 CommutateStep。
     */
    if ((s_run == 0U) || (s_step > 5U))
    {
        return;
    }
    switch (s_step)
    {
    case 0U:
    case 1U:
        ATIM_SetCompare1(duty);
        break;
    case 2U:
    case 3U:
        ATIM_SetCompare2(duty);
        break;
    default:
        ATIM_SetCompare3(duty);
        break;
    }
}

uint16_t BSP_MOTOR_GetDuty(void)
{
    return s_duty;
}

void BSP_MOTOR_CommutateStep(uint8_t step)
{
    uint16_t duty = s_duty;
    uint32_t ccer;

    if (step > 5U)
    {
        return;
    }
    s_step = step;

    if (s_run == 0U)
    {
        return;
    }

    /* 先关断全部输出，再按步配置，避免桥臂直通毛刺 */
    CW_ATIM->CCER &= (uint32_t)~CC_ALL;

    ATIM_SetCompare1(0U);
    ATIM_SetCompare2(0U);
    ATIM_SetCompare3(0U);

    switch (step)
    {
    case 0U: /* A+ B- */
        ATIM_SetCompare1(duty);
        ATIM_SetCompare2(0U);
        ccer = ATIM_CCER_CC1E_Msk | ATIM_CCER_CC1NE_Msk |
               ATIM_CCER_CC2E_Msk | ATIM_CCER_CC2NE_Msk;
        break;
    case 1U: /* A+ C- */
        ATIM_SetCompare1(duty);
        ATIM_SetCompare3(0U);
        ccer = ATIM_CCER_CC1E_Msk | ATIM_CCER_CC1NE_Msk |
               ATIM_CCER_CC3E_Msk | ATIM_CCER_CC3NE_Msk;
        break;
    case 2U: /* B+ C- */
        ATIM_SetCompare2(duty);
        ATIM_SetCompare3(0U);
        ccer = ATIM_CCER_CC2E_Msk | ATIM_CCER_CC2NE_Msk |
               ATIM_CCER_CC3E_Msk | ATIM_CCER_CC3NE_Msk;
        break;
    case 3U: /* B+ A- */
        ATIM_SetCompare2(duty);
        ATIM_SetCompare1(0U);
        ccer = ATIM_CCER_CC2E_Msk | ATIM_CCER_CC2NE_Msk |
               ATIM_CCER_CC1E_Msk | ATIM_CCER_CC1NE_Msk;
        break;
    case 4U: /* C+ A- */
        ATIM_SetCompare3(duty);
        ATIM_SetCompare1(0U);
        ccer = ATIM_CCER_CC3E_Msk | ATIM_CCER_CC3NE_Msk |
               ATIM_CCER_CC1E_Msk | ATIM_CCER_CC1NE_Msk;
        break;
    default: /* 5: C+ B- */
        ATIM_SetCompare3(duty);
        ATIM_SetCompare2(0U);
        ccer = ATIM_CCER_CC3E_Msk | ATIM_CCER_CC3NE_Msk |
               ATIM_CCER_CC2E_Msk | ATIM_CCER_CC2NE_Msk;
        break;
    }

    CW_ATIM->CCER |= ccer;
}

void BSP_MOTOR_Start(void)
{
    s_run = 1U;
    ATIM_CtrlPWMOutputs(ENABLE);
}

void BSP_MOTOR_Stop(void)
{
    s_run = 0U;
    outputs_off();
    ATIM_CtrlPWMOutputs(DISABLE);
}
