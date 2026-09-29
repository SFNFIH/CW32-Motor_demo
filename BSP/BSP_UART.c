/**
 * @file    BSP_UART.c
 * @brief   UART1 on PB12(TX)/PB11(RX) + JustFloat (同 VOFA 示例)
 */
#include "BSP_UART.h"
#include "cw32l012_sysctrl.h"
#include "cw32l012_gpio.h"
#include <stddef.h>

#if defined (__GNUC__) && !defined (__clang__)
#define __WEAK __attribute__((weak))
#else
#ifndef __WEAK
#define __WEAK __weak
#endif
#endif

void BSP_UART_Init(void)
{
    uint32_t brr;
    uint32_t pclk = SystemCoreClock;

    if (pclk == 0U)
    {
        pclk = 8000000U;
    }

    __SYSCTRL_GPIOB_CLK_ENABLE();
    __SYSCTRL_UART1_CLK_ENABLE();
    __SYSCTRL_UART1_RST_ENABLE();
    __SYSCTRL_UART1_RST_DISABLE();

    PB12_AFx_UART1TXD();
    CW_GPIOB->ANALOG &= ~(GPIO_PIN_12);

    PB11_AFx_UART1RXD();
    CW_GPIOB->ANALOG &= ~(GPIO_PIN_11);
    PB11_PUR_ENABLE();
    PB11_DIGTAL_ENABLE();

    brr = (pclk + (BSP_UART_BAUD >> 1)) / BSP_UART_BAUD;
    CW_UART1->BRRI = (uint16_t)(brr >> 4);
    CW_UART1->BRRF = (uint16_t)(brr & 0x0FU);
    CW_UART1->CR2 &= ~UARTx_CR2_SWAP_Msk;
    CW_UART1->CR1  = UARTx_CR1_TXEN_Msk | UARTx_CR1_RXEN_Msk;
}

void BSP_UART_SendByte(uint8_t ch)
{
    while ((CW_UART1->ISR & UARTx_ISR_TXE_Msk) == 0U)
    {
    }
    CW_UART1->TDR = ch;
}

void BSP_UART_Write(const void *buf, uint32_t len)
{
    const uint8_t *p = (const uint8_t *)buf;
    uint32_t i;

    if ((buf == NULL) || (len == 0U))
    {
        return;
    }

    for (i = 0U; i < len; i++)
    {
        BSP_UART_SendByte(p[i]);
    }

    /* 等最后一字节发完, 同 Arduino 一帧发完再发下一帧 */
    while ((CW_UART1->ISR & UARTx_ISR_TXBUSY_Msk) != 0U)
    {
    }
}

void BSP_UART_SendJustFloat(const float *data, uint8_t n)
{
    static const uint8_t tail[4] = { 0x00U, 0x00U, 0x80U, 0x7FU };

    if ((data == NULL) || (n == 0U))
    {
        return;
    }

    /* Serial.write((char *)data, sizeof(float) * n); */
    BSP_UART_Write(data, (uint32_t)sizeof(float) * (uint32_t)n);
    /* Serial.write(tail, 4); */
    BSP_UART_Write(tail, 4U);
}

#if defined (__GNUC__) && !defined (__clang__)
int __io_putchar(int ch);
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

__WEAK PUTCHAR_PROTOTYPE
{
    BSP_UART_SendByte((uint8_t)ch);
    return ch;
}

#if defined (__GNUC__) && !defined (__clang__)
int _write(int file, char *ptr, int len)
{
    int i;

    (void)file;
    for (i = 0; i < len; i++)
    {
        __io_putchar((int)ptr[i]);
    }
    return len;
}
#endif
