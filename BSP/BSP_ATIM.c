#include "main.h"
#include "BSP_ATIM.h"
#include "cw32l012_atim.h"

void BSP_ATIM_Init(void)
{
    __SYSCTRL_ATIM_CLK_ENABLE();
    __SYSCTRL_GPIOA_CLK_ENABLE();
    __SYSCTRL_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct;
    ATIM_InitTypeDef ATIM_InitStruct;
    ATIM_OCInitTypeDef ATIM_OCInitStruct;

    PB05_AFx_ATIMCH1();
    PB06_AFx_ATIMCH2();
    PB07_AFx_ATIMCH3();

    PA15_AFx_ATIMCH1N();
    PB03_AFx_ATIMCH2N();
    PB04_AFx_ATIMCH3N();

    GPIO_InitStruct.IT = GPIO_IT_NONE;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pins = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_Init(CW_GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.IT = GPIO_IT_NONE;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pins = GPIO_PIN_15;
    GPIO_Init(CW_GPIOA, &GPIO_InitStruct);

        // 对ATIM进行计数的基本设置
    ATIM_InitStruct.BufferState = DISABLE;     //不启用ARR的缓存功能
    ATIM_InitStruct.CounterAlignedMode = ATIM_COUNT_ALIGN_MODE_CENTER_BOTH;    //
    ATIM_InitStruct.CounterDirection = ATIM_COUNTING_UP;
    ATIM_InitStruct.CounterOPMode = ATIM_OP_MODE_REPETITIVE;
    ATIM_InitStruct.Prescaler = 0; //1分频，计数时钟96MHz
    ATIM_InitStruct.ReloadValue = 2400 - 1;  // 单边溢出周期25us， 中心对齐共50us，PWM周期20kHz
    ATIM_InitStruct.RepetitionCounter = 0;
    ATIM_Init(&ATIM_InitStruct);

    ATIM_OCInitStruct.BufferState = DISABLE;
    ATIM_OCInitStruct.OCComplement = ENABLE;
    ATIM_OCInitStruct.OCFastMode = DISABLE;
    ATIM_OCInitStruct.OCInterruptState = ENABLE;
    ATIM_OCInitStruct.OCMode = ATIM_OCMODE_PWM1;
    ATIM_OCInitStruct.OCPolarity = ATIM_OCPOLARITY_NONINVERT;
    ATIM_OC1Init(&ATIM_OCInitStruct);
    ATIM_OC2Init(&ATIM_OCInitStruct);
    ATIM_OC3Init(&ATIM_OCInitStruct);

    ATIM_SetCompare1(ATIM_InitStruct.ReloadValue >> 3);
    ATIM_SetCompare2(ATIM_InitStruct.ReloadValue >> 2);
    ATIM_SetCompare3(3*ATIM_InitStruct.ReloadValue >> 3);

    ATIM_SetPWMDeadtime(20, 40, ENABLE);    // 前死区为20个单位，后死区为40个单位，死区计算见用户手册
    ATIM_CH1Config(ENABLE);
    ATIM_CH2Config(ENABLE);
    ATIM_CH3Config(ENABLE);

    ATIM_CtrlPWMOutputs(ENABLE);
    ATIM_Cmd(ENABLE);

    __disable_irq();
    NVIC_EnableIRQ(ATIM_IRQn);
    __enable_irq();
}