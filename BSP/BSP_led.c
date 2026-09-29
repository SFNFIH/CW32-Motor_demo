/**
 * @file    BSP_led.c
 * @brief   LED 驱动实现 (PC13, 低电平点亮)
 */
#include "BSP_led.h"

void BSP_LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    BSP_LED1_CLK_ENABLE();
    BSP_LED1_AF();

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.IT   = GPIO_IT_NONE;
    GPIO_InitStruct.Pins = BSP_LED1_PIN;
    GPIO_Init(BSP_LED1_PORT, &GPIO_InitStruct);

    BSP_LED_Off();
}

void BSP_LED_On(void)
{
    BSP_LED1_ON();
}

void BSP_LED_Off(void)
{
    BSP_LED1_OFF();
}

void BSP_LED_Tog(void)
{
    BSP_LED1_TOG();
}
