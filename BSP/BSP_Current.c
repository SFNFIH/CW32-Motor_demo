/**
 * @file    BSP_Current.c
 * @brief   片内运放独立模式 + ADC1
 *
 * 原理图 (Doc P3): BEMF PA0/1/2 与电流 PB0/1 独立引脚, 无硬件冲突。
 *
 * 两种 ADC 配置 (对照 VESC LONGEST_ZERO / 定时触发采流):
 *  - FULL : CH0/1/2 + CH8/9, 给 BEMF 无驱磁链
 *  - FAST : 仅 CH8/9, 短采样+较快 ADC 时钟, 在中心对齐峰 (CNT≈ARR)
 *           低侧导通窗内完成转换 (~2–4µs), 避免 5 通道 ~30µs 扫出 LS 窗
 *           导致 R 偏大约 2× / HFI di 失真。
 */
#include "BSP_Current.h"
#include "BSP_MOTOR.h"
#include "BSP_motor_params.h"
#include "cw32l012_adc.h"
#include "cw32l012_gpio.h"
#include "cw32l012_opa.h"
#include "cw32l012_sysctrl.h"
#include "cw32l012.h"

#define ADC_MODE_FULL   0U
#define ADC_MODE_FAST   1U

static float s_off_a = 2048.0f;
static float s_off_b = 2048.0f;
static float s_scale;
static uint8_t s_adc_mode = 0xFFU;

static void adc_irq_off(void)
{
    NVIC_DisableIRQ(ADC1_IRQn);
    ADC_ITConfig(CW_ADC1, ADC_IT_EOS, DISABLE);
    ADC_ITConfig(CW_ADC1, ADC_IT_EOC, DISABLE);
    ADC_ExtTrigCfg(CW_ADC1, ADC_TRIG_ATIMOC4REFC, DISABLE);
    ADC_ClearITPendingAll(CW_ADC1);
    NVIC_ClearPendingIRQ(ADC1_IRQn);
}

/** 等到 ATIM 计数接近 ARR (中心对齐峰, 三相 LS 最可能同时导通). 1=ok */
static uint8_t wait_ls_window(void)
{
    const uint32_t thr = ((uint32_t)BSP_MOTOR_PWM_ARR * 15UL) / 16UL;
    uint32_t guard = 400000UL;
    uint32_t cnt;
    uint32_t prev = 0xFFFFUL;

    /*
     * 等 CNT 上行穿过 thr→ARR 附近再采。若已在下行则等到下一周期,
     * 尽量把 2 通道转换落在峰顶 LS 窗内。
     */
    while (guard > 0U)
    {
        cnt = CW_ATIM->CNT & 0xFFFFUL;
        if ((cnt >= thr) && (cnt >= prev))
        {
            return 1U;
        }
        prev = cnt;
        guard--;
    }
    return 0U;
}

static uint8_t adc_wait_eos(uint32_t n)
{
    ADC_ClearITPendingBit(CW_ADC1, ADC_IT_EOC | ADC_IT_EOS);
    ADC_SoftwareStartConvCmd(CW_ADC1, ENABLE);
    while ((ADC_GetITStatus(CW_ADC1, ADC_IT_EOS) == RESET) && (n > 0U))
    {
        n--;
    }
    ADC_ClearITPendingBit(CW_ADC1, ADC_IT_EOC | ADC_IT_EOS);
    return (n != 0U) ? 1U : 0U;
}

static void config_fast_current(void)
{
    ADC_InitTypeDef adc = {0};
    ADC_ChannelTypeDef ch;

    if (s_adc_mode == ADC_MODE_FAST)
    {
        return;
    }

    adc_irq_off();

    /* 仅 Ia/Ib: 短采样 + Div4 → 2ch ≈ 2–3µs @ 96MHz HCLK */
    ch.ADC_SampTime = ADC_SampTime12Clk;
    ch.ADC_InputChannel = ADC_InputCH8;
    adc.ADC_IN0 = ch;
    ch.ADC_InputChannel = ADC_InputCH9;
    adc.ADC_IN1 = ch;
    adc.ADC_IN2 = adc.ADC_IN0;
    adc.ADC_IN3 = adc.ADC_IN0;
    adc.ADC_IN4 = adc.ADC_IN0;
    adc.ADC_IN5 = adc.ADC_IN0;
    adc.ADC_IN6 = adc.ADC_IN0;
    adc.ADC_IN7 = adc.ADC_IN0;

    adc.ADC_ClkDiv = ADC_Clk_Div4;
    adc.ADC_ConvertMode = ADC_ConvertMode_Once;
    adc.ADC_SlaveMod = ADC_SlaveMode_Disable;
    adc.ADC_SQREns = ADC_SqrEns0to1;
    ADC_Init(CW_ADC1, &adc);
    ADC_Enable(CW_ADC1);
    s_adc_mode = ADC_MODE_FAST;
}

static uint8_t adc_pair_fast(uint16_t *a, uint16_t *b)
{
    config_fast_current();
    if (wait_ls_window() == 0U)
    {
        return 0U;
    }
    if (adc_wait_eos(8000U) == 0U)
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

    /* 偏置在 PWM 关断时采; 仍走 FAST 路径保持同一增益/通道映射 */
    for (i = 0U; i < 64U; i++)
    {
        uint16_t a;
        uint16_t b;
        if (adc_pair_fast(&a, &b) != 0U)
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
    if (adc_pair_fast(&a, &b) == 0U)
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

    s_adc_mode = 0xFFU;
    BSP_Current_ReconfigAdc();
    config_fast_current();

    /* I = (adc - offset) * Vref / 4096 / (Rshunt * gain) */
    s_scale = CUR_VREF_V / (4096.0f * CUR_SHUNT_OHM * CUR_AMP_GAIN);
}

void BSP_Current_ReconfigAdc(void)
{
    ADC_InitTypeDef adc = {0};
    ADC_ChannelTypeDef ch;

    adc_irq_off();

    /* Doc: EA/EB/EC→PA0/1/2(CH0/1/2), Ia/Ib→PB0/1(CH8/9) — 满序列给 BEMF */
    ch.ADC_SampTime = ADC_SampTime54Clk;
    ch.ADC_InputChannel = ADC_InputCH0;
    adc.ADC_IN0 = ch;
    ch.ADC_InputChannel = ADC_InputCH1;
    adc.ADC_IN1 = ch;
    ch.ADC_InputChannel = ADC_InputCH2;
    adc.ADC_IN2 = ch;
    ch.ADC_InputChannel = ADC_InputCH8;
    adc.ADC_IN3 = ch;
    ch.ADC_InputChannel = ADC_InputCH9;
    adc.ADC_IN4 = ch;
    adc.ADC_IN5 = adc.ADC_IN0;
    adc.ADC_IN6 = adc.ADC_IN0;
    adc.ADC_IN7 = adc.ADC_IN0;

    adc.ADC_ClkDiv = ADC_Clk_Div8;
    adc.ADC_ConvertMode = ADC_ConvertMode_Once;
    adc.ADC_SlaveMod = ADC_SlaveMode_Disable;
    adc.ADC_SQREns = ADC_SqrEns0to4;
    ADC_Init(CW_ADC1, &adc);
    ADC_Enable(CW_ADC1);
    s_adc_mode = ADC_MODE_FULL;
}
