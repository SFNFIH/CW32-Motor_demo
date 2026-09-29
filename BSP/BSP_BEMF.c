/**
 * @file    BSP_BEMF.c
 * @brief   ATIM CH4 触发三通道 BEMF 采样 (参考 ADC1_Configuration)
 */
#include "BSP_BEMF.h"
#include "BSP_Sensorless.h"
#include <stddef.h>

volatile uint16_t g_bemf_sample[3];

void BSP_BEMF_Init(void)
{
    ADC_InitTypeDef adc = {0};
    GPIO_InitTypeDef gpio = {0};

    __SYSCTRL_GPIOA_CLK_ENABLE();
    __SYSCTRL_ADC_CLK_ENABLE();

    gpio.IT   = GPIO_IT_NONE;
    gpio.Mode = GPIO_MODE_ANALOG;
    gpio.Pins = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
    GPIO_Init(CW_GPIOA, &gpio);

    PA00_ANALOG_ENABLE();
    PA01_ANALOG_ENABLE();
    PA02_ANALOG_ENABLE();

    adc.ADC_ClkDiv      = ADC_Clk_Div2; /* 96 MHz, 同参考 */
    adc.ADC_ConvertMode = ADC_ConvertMode_Once;
    adc.ADC_SlaveMod    = ADC_SlaveMode_Disable;
    adc.ADC_SQREns      = ADC_SqrEns0to2;

    adc.ADC_IN0.ADC_InputChannel = ADC_InputCH0;
    adc.ADC_IN0.ADC_SampTime     = ADC_SampTime54Clk;
    adc.ADC_IN1.ADC_InputChannel = ADC_InputCH1;
    adc.ADC_IN1.ADC_SampTime     = ADC_SampTime54Clk;
    adc.ADC_IN2.ADC_InputChannel = ADC_InputCH2;
    adc.ADC_IN2.ADC_SampTime     = ADC_SampTime54Clk;

    ADC_Init(BSP_BEMF_ADC, &adc);
    ADC_ClearITPendingAll(BSP_BEMF_ADC);
    ADC_ITConfig(BSP_BEMF_ADC, ADC_IT_EOS, ENABLE);

    /* 与参考 ADC_TRIG_ATIMCC4 同 bit: OC4REFC */
    ADC_ExtTrigCfg(BSP_BEMF_ADC, ADC_TRIG_ATIMOC4REFC, ENABLE);

    NVIC_SetPriority(ADC1_IRQn, 1U);
    NVIC_EnableIRQ(ADC1_IRQn);
    ADC_Enable(BSP_BEMF_ADC);
}

void BSP_BEMF_IRQHandler(void)
{
    if (CW_ADC1->ISR_f.EOS == 0U)
    {
        return;
    }
    ADC_ClearITPendingAll(CW_ADC1);

    ADC_GetSqr0Result(CW_ADC1, (uint16_t *)&g_bemf_sample[0]);
    ADC_GetSqr1Result(CW_ADC1, (uint16_t *)&g_bemf_sample[1]);
    ADC_GetSqr2Result(CW_ADC1, (uint16_t *)&g_bemf_sample[2]);

    BSP_Sensorless_OnAdcSample();
}

uint16_t BSP_BEMF_ReadPhase(uint8_t phase)
{
    if (phase > 2U)
    {
        phase = 2U;
    }
    return g_bemf_sample[phase];
}

void BSP_BEMF_Read(uint16_t *a, uint16_t *b, uint16_t *c)
{
    if (a != NULL) { *a = g_bemf_sample[0]; }
    if (b != NULL) { *b = g_bemf_sample[1]; }
    if (c != NULL) { *c = g_bemf_sample[2]; }
}

uint16_t BSP_BEMF_ReadA(void) { return g_bemf_sample[0]; }
uint16_t BSP_BEMF_ReadB(void) { return g_bemf_sample[1]; }
uint16_t BSP_BEMF_ReadC(void) { return g_bemf_sample[2]; }
