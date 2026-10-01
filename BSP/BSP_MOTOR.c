/**
 * @file    BSP_MOTOR.c
 * @brief   三相互补 PWM, 三相同时输出
 */
#include "BSP_MOTOR.h"
#include "cw32l012.h"
#include "cw32l012_atim.h"
#include "cw32l012_gpio.h"
#include "cw32l012_sysctrl.h"

static void gpio_init(void)
{
    GPIO_InitTypeDef g = {0};

    __SYSCTRL_GPIOA_CLK_ENABLE();
    __SYSCTRL_GPIOB_CLK_ENABLE();

    PB05_AFx_ATIMCH1();
    PB06_AFx_ATIMCH2();
    PB07_AFx_ATIMCH3();
    PA15_AFx_ATIMCH1N();
    PB03_AFx_ATIMCH2N();
    PB04_AFx_ATIMCH3N();

    g.Mode = GPIO_MODE_OUTPUT_PP;
    g.IT = GPIO_IT_NONE;
    g.Pins = GPIO_PIN_15;
    GPIO_Init(CW_GPIOA, &g);
    g.Pins = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_Init(CW_GPIOB, &g);
}

void BSP_MOTOR_Init(void)
{
    ATIM_InitTypeDef tim;
    ATIM_OCInitTypeDef oc;
    uint16_t mid = (uint16_t)(BSP_MOTOR_PWM_ARR / 2U);

    gpio_init();
    __SYSCTRL_ATIM_CLK_ENABLE();
    ATIM_DeInit();

    tim.BufferState = ENABLE;
    tim.CounterAlignedMode = ATIM_COUNT_ALIGN_MODE_CENTER_BOTH;
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
    CW_ATIM->CCER_f.CC1NE = 1U;
    CW_ATIM->CCER_f.CC2NE = 1U;
    CW_ATIM->CCER_f.CC3NE = 1U;

    ATIM_SetCompare1(mid);
    ATIM_SetCompare2(mid);
    ATIM_SetCompare3(mid);

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
}

void BSP_MOTOR_SetPhaseDuty(uint16_t da, uint16_t db, uint16_t dc)
{
    if (da > BSP_MOTOR_PWM_ARR) { da = BSP_MOTOR_PWM_ARR; }
    if (db > BSP_MOTOR_PWM_ARR) { db = BSP_MOTOR_PWM_ARR; }
    if (dc > BSP_MOTOR_PWM_ARR) { dc = BSP_MOTOR_PWM_ARR; }
    ATIM_SetCompare1(da);
    ATIM_SetCompare2(db);
    ATIM_SetCompare3(dc);
}

void BSP_MOTOR_Start(void)
{
    uint16_t mid = (uint16_t)(BSP_MOTOR_PWM_ARR / 2U);
    BSP_MOTOR_SetPhaseDuty(mid, mid, mid);
    ATIM_CtrlPWMOutputs(ENABLE);
}

void BSP_MOTOR_Stop(void)
{
    uint16_t mid = (uint16_t)(BSP_MOTOR_PWM_ARR / 2U);
    BSP_MOTOR_SetPhaseDuty(mid, mid, mid);
    ATIM_CtrlPWMOutputs(DISABLE);
}

void BSP_MOTOR_EnablePwmIrq(void)
{
    ATIM_ClearITPendingBit(ATIM_STATE_UIF);
    ATIM_ITConfig(ATIM_IT_UIE, ENABLE);
    NVIC_ClearPendingIRQ(ATIM_IRQn);
    NVIC_SetPriority(ATIM_IRQn, 0U);
    NVIC_EnableIRQ(ATIM_IRQn);
}

void BSP_MOTOR_DisablePwmIrq(void)
{
    ATIM_ITConfig(ATIM_IT_UIE, DISABLE);
    NVIC_DisableIRQ(ATIM_IRQn);
}
