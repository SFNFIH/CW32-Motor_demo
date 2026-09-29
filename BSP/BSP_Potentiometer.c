#include "BSP_Potentiometer.h"
#include "cw32l012_adc.h"
#include "cw32l012_gpio.h"
#include "cw32l012_sysctrl.h"

void BSP_Potentiometer_Init(void)
{
    ADC_InitTypeDef adc = {0};
    GPIO_InitTypeDef gpio = {0};

    __SYSCTRL_GPIOA_CLK_ENABLE();
    __SYSCTRL_ADC_CLK_ENABLE();

    gpio.IT = GPIO_IT_NONE;
    gpio.Mode = GPIO_MODE_ANALOG;
    gpio.Pins = GPIO_PIN_9;
    GPIO_Init(CW_GPIOA, &gpio);
    PA09_ANALOG_ENABLE();

    adc.ADC_ClkDiv = ADC_Clk_Div4;
    adc.ADC_ConvertMode = ADC_ConvertMode_Once;
    adc.ADC_SlaveMod = ADC_SlaveMode_Disable;
    adc.ADC_SQREns = ADC_SqrEns0to0;
    adc.ADC_IN0.ADC_SampTime = ADC_SampTime134Clk;
    adc.ADC_IN0.ADC_InputChannel = ADC_InputCH6;
    ADC_Init(CW_ADC2, &adc);
    ADC_Enable(CW_ADC2);
}

uint16_t BSP_Potentiometer_Read(void)
{
    uint32_t t = 200000U;

    ADC_ClearITPendingBit(CW_ADC2, ADC_IT_EOC | ADC_IT_EOS);
    ADC_SoftwareStartConvCmd(CW_ADC2, ENABLE);
    while ((ADC_GetITStatus(CW_ADC2, ADC_IT_EOS) == RESET) && (t > 0U))
    {
        t--;
    }
    ADC_ClearITPendingBit(CW_ADC2, ADC_IT_EOC | ADC_IT_EOS);
    if (t == 0U)
    {
        return 0U;
    }
    return ADC_GetConversionValue(CW_ADC2, ADC_RESULT_0);
}
