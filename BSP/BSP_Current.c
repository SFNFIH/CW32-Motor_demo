/**
 * @file    BSP_Current.c
 * @brief   片内运放独立模式 (外接 20 倍反相), ADC1 读 PB0/PB1
 */
#include "BSP_Current.h"
#include "BSP_BEMF.h"
#include "BSP_motor_params.h"
#include "cw32l012_adc.h"
#include "cw32l012_gpio.h"
#include "cw32l012_opa.h"
#include "cw32l012_sysctrl.h"

static float s_off_a = 2048.0f;
static float s_off_b = 2048.0f;
static float s_scale;

static uint8_t adc_pair(uint16_t *a, uint16_t *b)
{
    uint32_t n = 80U;

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
    *a = ADC_GetConversionValue(CW_ADC1, ADC_RESULT_0);
    *b = ADC_GetConversionValue(CW_ADC1, ADC_RESULT_1);
    return 1U;
}

void BSP_Current_Calibrate(void)
{
    uint32_t i;
    float sa = 0.0f;
    float sb = 0.0f;
    uint32_t n = 0U;

    for (i = 0U; i < 64U; i++)
    {
        uint16_t a;
        uint16_t b;
        if (adc_pair(&a, &b) != 0U)
        {
            sa += (float)a;
            sb += (float)b;
            n++;
        }
    }
    if (n > 0U)
    {
        s_off_a = sa / (float)n;
        s_off_b = sb / (float)n;
    }
}

uint8_t BSP_Current_Read(float *ia, float *ib)
{
    uint16_t a;
    uint16_t b;

    if ((ia == 0) || (ib == 0))
    {
        return 0U;
    }
    /* BEMF 占用 ADC1 时拒绝读电流, 避免通道错乱 */
    if (BSP_BEMF_IsActive() != 0U)
    {
        return 0U;
    }
    if (adc_pair(&a, &b) == 0U)
    {
        return 0U;
    }
    *ia = ((float)a - s_off_a) * s_scale;
    *ib = ((float)b - s_off_b) * s_scale;
    return 1U;
}

void BSP_Current_Init(void)
{
    OPA_InitTypeDef opa = {0};

    __SYSCTRL_GPIOA_CLK_ENABLE();
    __SYSCTRL_GPIOB_CLK_ENABLE();
    __SYSCTRL_OPA_CLK_ENABLE();
    __SYSCTRL_ADC_CLK_ENABLE();

    AFx_OPA1INP2_PA06();
    AFx_OPA1INN2_PA07();
    AFx_OPA1OUT_PB00();
    AFx_OPA2INP1_PA04();
    AFx_OPA2INN1_PA05();
    AFx_OPA2OUT_PB01();

    opa.Bias = OPA_BIAS_8UA_4US;
    opa.WorkMode = OPA_WORKMODE_STANDALONE;
    opa.PgaGain = OPA_PGA_GAIN2;
    opa.InputP = OPA_INPUT_INP2;
    opa.InputN = OPA_INPUT_INN2;
    OPA_Init(CW_OPA1, &opa);
    OPA_Start(CW_OPA1);

    opa.InputP = OPA_INPUT_INP1;
    opa.InputN = OPA_INPUT_INN1;
    OPA_Init(CW_OPA2, &opa);
    OPA_Start(CW_OPA2);

    BSP_Current_ReconfigAdc();

    /* I = (adc - offset) * Vref / 4096 / (Rshunt * gain) */
    s_scale = CUR_VREF_V / (4096.0f * CUR_SHUNT_OHM * CUR_AMP_GAIN);
}

void BSP_Current_ReconfigAdc(void)
{
    ADC_InitTypeDef adc = {0};
    ADC_ChannelTypeDef ch;

    NVIC_DisableIRQ(ADC1_IRQn);
    ADC_ITConfig(CW_ADC1, ADC_IT_EOS, DISABLE);
    ADC_ExtTrigCfg(CW_ADC1, ADC_TRIG_ATIMOC4REFC, DISABLE);
    ADC_ClearITPendingAll(CW_ADC1);

    ch.ADC_SampTime = ADC_SampTime54Clk;
    ch.ADC_InputChannel = ADC_InputCH8;

    adc.ADC_ClkDiv = ADC_Clk_Div8;
    adc.ADC_ConvertMode = ADC_ConvertMode_Once;
    adc.ADC_SlaveMod = ADC_SlaveMode_Disable;
    adc.ADC_SQREns = ADC_SqrEns0to1;
    adc.ADC_IN0 = ch;
    ch.ADC_InputChannel = ADC_InputCH9;
    adc.ADC_IN1 = ch;
    adc.ADC_IN2 = adc.ADC_IN0;
    adc.ADC_IN3 = adc.ADC_IN0;
    adc.ADC_IN4 = adc.ADC_IN0;
    adc.ADC_IN5 = adc.ADC_IN0;
    adc.ADC_IN6 = adc.ADC_IN0;
    adc.ADC_IN7 = adc.ADC_IN0;
    ADC_Init(CW_ADC1, &adc);
    ADC_Enable(CW_ADC1);
}
