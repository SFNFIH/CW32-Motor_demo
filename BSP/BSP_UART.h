/**
 * @file    BSP_UART.h
 * @brief   UART1 (PB12 TX / PB11 RX) + JustFloat
 */
#ifndef BSP_UART_H
#define BSP_UART_H

#include <stdint.h>
#include "cw32l012_uart.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 与 VOFA 保持一致; 你当前软件里是 1152000 */
#ifndef BSP_UART_BAUD
#define BSP_UART_BAUD  115200U
#endif

void BSP_UART_Init(void);
void BSP_UART_SendByte(uint8_t ch);

/** 等价于 Arduino Serial.write(buf, len) */
void BSP_UART_Write(const void *buf, uint32_t len);

/**
 * @brief  JustFloat: 连续发送 n 个 float 的原始字节 + 帧尾 00 00 80 7F
 *         对应: Serial.write((char*)data, sizeof(float)*n);
 *               Serial.write(tail, 4);
 */
void BSP_UART_SendJustFloat(const float *data, uint8_t n);

#ifdef __cplusplus
}
#endif

#endif /* BSP_UART_H */
