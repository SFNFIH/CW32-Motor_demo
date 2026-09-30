/**
 * @file    interrupts_cw32l012.c
 * @brief   Interrupt Service Routines
 *
 * 活跃 IRQ:
 *   ATIM  — FOC SPWM (prio 1)
 *   BTIM1 — 1ms 按键 + g_millis (prio 2)
 * 其余为空实现并清标志, 防止误使能后中断风暴卡死。
 */
#include "../inc/main.h"
#include "BSP_FOC.h"
#include "BSP_Button.h"
#include "cw32l012_btim.h"
#include "cw32l012_adc.h"
#include "cw32l012_atim.h"
#include "cw32l012.h"

void NMI_Handler(void) {}

void HardFault_Handler(void)
{
    while (1) {}
}

void SVC_Handler(void) {}
void PendSV_Handler(void) {}

void WDT_IRQHandler(void) {}
void LVD_IRQHandler(void) {}
void RTC_IRQHandler(void) {}
void FLASHRAM_IRQHandler(void) {}
void SYSCTRL_IRQHandler(void) {}
void GPIOA_IRQHandler(void) {}
void GPIOB_IRQHandler(void) {}
void GPIOC_IRQHandler(void) {}
void GPIOF_IRQHandler(void) {}
void DMACH12_IRQHandler(void) {}
void DMACH34_IRQHandler(void) {}

void CORDIC_IRQHandler(void)
{
    /* 读结果清 EOC; 本工程 CORDIC 不使能 IE, 防误触发风暴 */
    (void)CW_CORDIC->X;
    (void)CW_CORDIC->Y;
    (void)CW_CORDIC->Z;
}

void ADC1_IRQHandler(void)
{
    /* 软件轮询采流, 不使能 ADC1 NVIC; 误进则清标志退出 */
    ADC_ClearITPendingAll(CW_ADC1);
}

void ATIM_IRQHandler(void)
{
    BSP_FOC_PwmIrq();
}

void VC13_IRQHandler(void) {}
void VC24_IRQHandler(void) {}
void GTIM1_IRQHandler(void) {}
void GTIM2_IRQHandler(void) {}
void GTIM34_IRQHandler(void) {}
void LPTIM_IRQHandler(void) {}
void ATIM_IRQHandlerCallBack(void) {}

void BTIM1_IRQHandler(void)
{
    if (BTIM_GetITStatus(CW_BTIM1, BTIM_IT_UPDATE) != RESET)
    {
        BTIM_ClearITPendingBit(CW_BTIM1, BTIM_IT_UPDATE);
        BSP_Button_Tick1ms();
    }
}

void BTIM2_IRQHandler(void) {}

void BTIM3_HALLTIM_IRQHandler(void) {}

void I2C1_IRQHandler(void) {}
void I2C2_IRQHandler(void) {}
void SPI1_IRQHandler(void) {}
void SPI23_IRQHandler(void) {}
void UART1_IRQHandler(void) {}
void UART2_IRQHandler(void) {}
void UART3_IRQHandler(void) {}

void ADC2_DAC_IRQHandler(void)
{
    ADC_ClearITPendingAll(CW_ADC2);
}

void CLKFAULT_IRQHandler(void) {}
