/**
 * @file    BSP_AS5600.h
 * @brief   AS5600 磁编码器 (HW I2C1: PC14=SDA, PC15=SCL)
 */
#ifndef BSP_AS5600_H
#define BSP_AS5600_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BSP_AS5600_I2C_ADDR   0x36U

typedef struct
{
    uint16_t raw;       /* 0..4095 原始角 */
    uint16_t status;    /* STATUS 寄存器 */
    int32_t  cum_raw;   /* 展开后的累计角 (raw 计数) */
    int32_t  rpm_x10;   /* RPM * 10 */
    uint8_t  ok;        /* 1=通信成功 */
    uint8_t  mag_ok;    /* STATUS.MD 磁铁检测 */
} BSP_AS5600_State_t;

void BSP_AS5600_Init(uint32_t pclk_hz);
uint8_t BSP_AS5600_Probe(void);
uint8_t BSP_AS5600_ReadRaw(uint16_t *raw);
uint8_t BSP_AS5600_ReadStatus(uint16_t *status);
/** 主循环调用: 刷新角度/转速 */
void BSP_AS5600_Update(void);
const BSP_AS5600_State_t *BSP_AS5600_GetState(void);
uint8_t BSP_AS5600_GetLastErr(void);
uint8_t BSP_AS5600_GetIdlePins(void);
uint8_t BSP_AS5600_GetFoundAddr(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_AS5600_H */
