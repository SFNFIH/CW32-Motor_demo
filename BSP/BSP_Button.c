/**
 * @file    BSP_Button.c
 * @brief   PB10 按键: 1ms 节拍扫描, 单击/双击
 */
#include "BSP_Button.h"
#include <stddef.h>

#define BTN_DEBOUNCE_TICKS   (BSP_BTN_DEBOUNCE_MS / BSP_BTN_SCAN_MS)
#define BTN_DBLCLICK_TICKS   (BSP_BTN_DBLCLICK_MS / BSP_BTN_SCAN_MS)
#define BTN_SCAN_DIV         BSP_BTN_SCAN_MS

#define BTN_PRESSED          0U
#define BTN_RELEASED         1U

static BSP_Button_Callback_t s_click_cb;
static BSP_Button_Callback_t s_dblclick_cb;

static uint8_t  s_stable;
static uint8_t  s_last_raw;
static uint8_t  s_debounce_cnt;
static uint8_t  s_click_cnt;
static uint16_t s_wait_ticks;
static uint8_t  s_scan_div;

static void Button_GPIO_Init(void)
{
    GPIO_InitTypeDef gpio;

    BSP_BTN_CLK_ENABLE();
    gpio.Pins = BSP_BTN_PIN;
    gpio.Mode = GPIO_MODE_INPUT_PULLUP;
    gpio.IT   = GPIO_IT_NONE;
    GPIO_Init(BSP_BTN_PORT, &gpio);
}

static void Button_Scan(void)
{
    uint8_t raw = (BSP_BTN_READ() != 0U) ? BTN_RELEASED : BTN_PRESSED;

    if (raw != s_last_raw)
    {
        s_last_raw = raw;
        s_debounce_cnt = 0U;
    }
    else if (s_debounce_cnt < BTN_DEBOUNCE_TICKS)
    {
        s_debounce_cnt++;
        if (s_debounce_cnt >= BTN_DEBOUNCE_TICKS)
        {
            uint8_t prev = s_stable;
            s_stable = raw;
            if ((prev == BTN_PRESSED) && (s_stable == BTN_RELEASED))
            {
                s_click_cnt++;
                if (s_click_cnt == 1U)
                {
                    s_wait_ticks = BTN_DBLCLICK_TICKS;
                }
                else if (s_click_cnt >= 2U)
                {
                    s_click_cnt = 0U;
                    s_wait_ticks = 0U;
                    if (s_dblclick_cb != NULL)
                    {
                        s_dblclick_cb();
                    }
                }
            }
        }
    }

    if (s_wait_ticks > 0U)
    {
        s_wait_ticks--;
        if ((s_wait_ticks == 0U) && (s_click_cnt == 1U))
        {
            s_click_cnt = 0U;
            if (s_click_cb != NULL)
            {
                s_click_cb();
            }
        }
    }
}

void BSP_Button_Init(void)
{
    s_click_cb     = NULL;
    s_dblclick_cb  = NULL;
    s_stable       = BTN_RELEASED;
    s_last_raw     = BTN_RELEASED;
    s_debounce_cnt = BTN_DEBOUNCE_TICKS;
    s_click_cnt    = 0U;
    s_wait_ticks   = 0U;
    s_scan_div     = 0U;
    Button_GPIO_Init();
}

void BSP_Button_SetClickCallback(BSP_Button_Callback_t cb)
{
    s_click_cb = cb;
}

void BSP_Button_SetDoubleClickCallback(BSP_Button_Callback_t cb)
{
    s_dblclick_cb = cb;
}

void BSP_Button_Tick1ms(void)
{
    s_scan_div++;
    if (s_scan_div >= BTN_SCAN_DIV)
    {
        s_scan_div = 0U;
        Button_Scan();
    }
}
