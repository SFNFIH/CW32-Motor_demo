/**
 * @file    BSP_BEMF.c
 * @brief   三相反电势软件采样 + ADC1 与电流通道互斥
 */
#include "BSP_BEMF.h"
#include "BSP_Current.h"
#include "BSP_motor_params.h"
#include "cw32l012_adc.h"
#include "cw32l012_gpio.h"
#include "cw32l012_sysctrl.h"

#include <stddef.h>

volatile uint16_t g_bemf_sample[3];
static uint8_t s_active;

#ifndef BEMF_DIV_GAIN
#define BEMF_DIV_GAIN   ((100.0f + 5.1f) / 5.1f)
#endif

void BSP_BEMF_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __SYSCTRL_GPIOA_CLK_ENABLE();
    __SYSCTRL_ADC_CLK_ENABLE();

    gpio.IT = GPIO_IT_NONE;
    gpio.Mode = GPIO_MODE_ANALOG;
    gpio.Pins = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
    GPIO_Init(CW_GPIOA, &gpio);
    PA00_ANALOG_ENABLE();
    PA01_ANALOG_ENABLE();
    PA02_ANALOG_ENABLE();

    s_active = 0U;
    g_bemf_sample[0] = 0U;
    g_bemf_sample[1] = 0U;
    g_bemf_sample[2] = 0U;
}

void BSP_BEMF_Enter(void)
{
    ADC_InitTypeDef adc = {0};

    /* 关掉可能的外部触发 / IRQ, 独占 ADC1 */
    NVIC_DisableIRQ(ADC1_IRQn);
    ADC_ITConfig(CW_ADC1, ADC_IT_EOS, DISABLE);
    ADC_ExtTrigCfg(CW_ADC1, ADC_TRIG_ATIMOC4REFC, DISABLE);
    ADC_ClearITPendingAll(CW_ADC1);

    adc.ADC_ClkDiv = ADC_Clk_Div8;
    adc.ADC_ConvertMode = ADC_ConvertMode_Once;
    adc.ADC_SlaveMod = ADC_SlaveMode_Disable;
    adc.ADC_SQREns = ADC_SqrEns0to2;
    adc.ADC_IN0.ADC_InputChannel = ADC_InputCH0;
    adc.ADC_IN0.ADC_SampTime = ADC_SampTime54Clk;
    adc.ADC_IN1.ADC_InputChannel = ADC_InputCH1;
    adc.ADC_IN1.ADC_SampTime = ADC_SampTime54Clk;
    adc.ADC_IN2.ADC_InputChannel = ADC_InputCH2;
    adc.ADC_IN2.ADC_SampTime = ADC_SampTime54Clk;
    adc.ADC_IN3 = adc.ADC_IN0;
    adc.ADC_IN4 = adc.ADC_IN0;
    adc.ADC_IN5 = adc.ADC_IN0;
    adc.ADC_IN6 = adc.ADC_IN0;
    adc.ADC_IN7 = adc.ADC_IN0;
    ADC_Init(CW_ADC1, &adc);
    ADC_Enable(CW_ADC1);
    s_active = 1U;
}

void BSP_BEMF_Exit(void)
{
    if (s_active == 0U)
    {
        return;
    }
    s_active = 0U;
    BSP_Current_ReconfigAdc();
}

uint8_t BSP_BEMF_IsActive(void)
{
    return s_active;
}

uint8_t BSP_BEMF_ReadRaw(uint16_t *a, uint16_t *b, uint16_t *c)
{
    uint32_t n = 400U;

    if (s_active == 0U)
    {
        return 0U;
    }

    ADC_ClearITPendingBit(CW_ADC1, ADC_IT_EOC | ADC_IT_EOS);
    ADC_SoftwareStartConvCmd(CW_ADC1, ENABLE);
    while ((ADC_GetITStatus(CW_ADC1, ADC_IT_EOS) == RESET) && (n > 0U))
    {
        n--;
    }
    ADC_ClearITPendingBit(CW_ADC1, ADC_IT_EOC | ADC_IT_EOS);
    if (n == 0U)
    {
        return 0U;
    }

    g_bemf_sample[0] = ADC_GetConversionValue(CW_ADC1, ADC_RESULT_0);
    g_bemf_sample[1] = ADC_GetConversionValue(CW_ADC1, ADC_RESULT_1);
    g_bemf_sample[2] = ADC_GetConversionValue(CW_ADC1, ADC_RESULT_2);
    if (a != 0) { *a = g_bemf_sample[0]; }
    if (b != 0) { *b = g_bemf_sample[1]; }
    if (c != 0) { *c = g_bemf_sample[2]; }
    return 1U;
}

uint8_t BSP_BEMF_ReadVolt(float *va, float *vb, float *vc)
{
    uint16_t a;
    uint16_t b;
    uint16_t c;
    float scale;

    if (BSP_BEMF_ReadRaw(&a, &b, &c) == 0U)
    {
        return 0U;
    }
    /* Vphase = raw/4096 * Vref * divider */
    scale = (CUR_VREF_V / 4096.0f) * BEMF_DIV_GAIN;
    if (va != 0) { *va = (float)a * scale; }
    if (vb != 0) { *vb = (float)b * scale; }
    if (vc != 0) { *vc = (float)c * scale; }
    return 1U;
}

void BSP_BEMF_Clarke(float va, float vb, float vc, float *valpha, float *vbeta)
{
    float vavg = (va + vb + vc) * (1.0f / 3.0f);
    float van = va - vavg;
    float vbn = vb - vavg;
    float vcn = vc - vavg;

    if (valpha != 0)
    {
        *valpha = van;
    }
    if (vbeta != 0)
    {
        /* β = (vb_n - vc_n) / √3 */
        *vbeta = (vbn - vcn) * 0.57735026919f;
    }
}

void BSP_BEMF_IRQHandler(void)
{
    /* 无驱磁链用软件触发; ATIM/Sensorless 路径不在此工程启用 */
    if (CW_ADC1->ISR_f.EOS != 0U)
    {
        ADC_ClearITPendingAll(CW_ADC1);
    }
}

uint16_t BSP_BEMF_ReadPhase(uint8_t phase)
{
    if (phase > 2U) { phase = 2U; }
    return g_bemf_sample[phase];
}

void BSP_BEMF_Read(uint16_t *a, uint16_t *b, uint16_t *c)
{
    if (a != 0) { *a = g_bemf_sample[0]; }
    if (b != 0) { *b = g_bemf_sample[1]; }
    if (c != 0) { *c = g_bemf_sample[2]; }
}

uint16_t BSP_BEMF_ReadA(void) { return g_bemf_sample[0]; }
uint16_t BSP_BEMF_ReadB(void) { return g_bemf_sample[1]; }
uint16_t BSP_BEMF_ReadC(void) { return g_bemf_sample[2]; }
