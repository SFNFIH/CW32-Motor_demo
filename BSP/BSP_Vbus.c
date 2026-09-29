/**
 * @file    BSP_Vbus.c
 * @brief   PA08 ADC2_CH5 母线电压采样
 */
#include "BSP_Vbus.h"
#include "cw32l012_adc.h"
#include "cw32l012_gpio.h"
#include "cw32l012_sysctrl.h"

#define VBUS_GAIN   ((100.0f + 5.1f) / 5.1f)
#define VBUS_CH     ADC_InputCH5
#define POT_CH      ADC_InputCH6

void BSP_Vbus_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __SYSCTRL_GPIOA_CLK_ENABLE();
    __SYSCTRL_ADC_CLK_ENABLE();
    gpio.IT = GPIO_IT_NONE;
    gpio.Mode = GPIO_MODE_ANALOG;
    gpio.Pins = GPIO_PIN_8;
    GPIO_Init(CW_GPIOA, &gpio);
    PA08_ANALOG_ENABLE();
}

uint16_t BSP_Vbus_ReadRaw(void)
{
    uint32_t timeout = 4000U;
    uint16_t raw;

    CW_ADC2->SQRCFR_f.SQRCH0 = VBUS_CH;
    ADC_ClearITPendingBit(CW_ADC2, ADC_IT_EOC | ADC_IT_EOS);
    ADC_SoftwareStartConvCmd(CW_ADC2, ENABLE);
    while ((ADC_GetITStatus(CW_ADC2, ADC_IT_EOS) == RESET) && (timeout > 0U))
    {
        timeout--;
    }
    ADC_ClearITPendingBit(CW_ADC2, ADC_IT_EOC | ADC_IT_EOS);
    raw = (timeout == 0U) ? 0U : ADC_GetConversionValue(CW_ADC2, ADC_RESULT_0);
    CW_ADC2->SQRCFR_f.SQRCH0 = POT_CH;
    return raw;
}

float BSP_Vbus_ReadVolt(void)
{
    uint16_t raw = BSP_Vbus_ReadRaw();
    return ((float)raw / 4096.0f) * 5.0f * VBUS_GAIN;
}
