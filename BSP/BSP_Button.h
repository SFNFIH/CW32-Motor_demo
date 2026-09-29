/**
 * @file    BSP_Button.h
 * @brief   按键驱动 (PB10, 低电平有效); 由 1ms 节拍扫描
 */
#ifndef BSP_BUTTON_H
#define BSP_BUTTON_H

#include <stdint.h>
#include "cw32l012_gpio.h"
#include "cw32l012_sysctrl.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BSP_BTN_PIN                   GPIO_PIN_10
#define BSP_BTN_PORT                  CW_GPIOB
#define BSP_BTN_CLK_ENABLE()          __SYSCTRL_GPIOB_CLK_ENABLE()
#define BSP_BTN_READ()                PB10_GETVALUE()

#define BSP_BTN_SCAN_MS               10U
#define BSP_BTN_DEBOUNCE_MS           20U
#define BSP_BTN_DBLCLICK_MS           300U

typedef void (*BSP_Button_Callback_t)(void);

void BSP_Button_Init(void);
void BSP_Button_SetClickCallback(BSP_Button_Callback_t cb);
void BSP_Button_SetDoubleClickCallback(BSP_Button_Callback_t cb);

/** 每 1ms 调用一次 (BTIM1) */
void BSP_Button_Tick1ms(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_BUTTON_H */
