/**
 * @file    BSP_led.h
 * @brief   LED 驱动 (PC13 开漏灌电流, 低电平点亮)
 */
#ifndef BSP_LED_H
#define BSP_LED_H

#include <stdint.h>
#include "cw32l012_gpio.h"
#include "cw32l012_sysctrl.h"

#ifdef __cplusplus
extern "C" {
#endif

/* LED: PC13 → LED 阴极; 阳极经 10k 接 +5V → 低电平点亮 */
#define BSP_LED1_PIN                  GPIO_PIN_13
#define BSP_LED1_PORT                 CW_GPIOC
#define BSP_LED1_CLK_ENABLE()         __SYSCTRL_GPIOC_CLK_ENABLE()
#define BSP_LED1_AF()                 PC13_AFx_GPIO()
#define BSP_LED1_ON()                 PC13_SETLOW()
#define BSP_LED1_OFF()                PC13_SETHIGH()
#define BSP_LED1_TOG()                PC13_TOG()

/**
 * @brief  初始化 LED (PC13 推挽, 默认熄灭)
 */
void BSP_LED_Init(void);
void BSP_LED_On(void);
void BSP_LED_Off(void);
void BSP_LED_Tog(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_LED_H */
