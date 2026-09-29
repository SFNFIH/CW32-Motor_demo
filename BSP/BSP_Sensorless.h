/**
 * @file    BSP_Sensorless.h
 * @brief   无感六步: 开环强拖 → 反电势过零闭环 (参考 OPENLOOP CONTROL OK)
 */
#ifndef BSP_SENSORLESS_H
#define BSP_SENSORLESS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    SLS_ST_IDLE = 0,
    SLS_ST_ALIGN,
    SLS_ST_RAMP,   /* 开环强拖 */
    SLS_ST_RUN,    /* 过零闭环 */
    SLS_ST_FAULT
} SensorlessState_t;

void BSP_Sensorless_Init(void);
void BSP_Sensorless_Start(uint16_t duty);
void BSP_Sensorless_Stop(void);
void BSP_Sensorless_SetDuty(uint16_t duty);

void BSP_Sensorless_SetDirection(int8_t dir);
int8_t BSP_Sensorless_GetDirection(void);
void BSP_Sensorless_ToggleDirection(void);

/** BTIM1 1ms */
void BSP_Sensorless_Tick1ms(void);
/** BTIM3 退磁/延时换相 */
void BSP_Sensorless_Btim3IRQ(void);
/** ADC EOS 过零检测 */
void BSP_Sensorless_OnAdcSample(void);

SensorlessState_t BSP_Sensorless_GetState(void);
uint8_t  BSP_Sensorless_GetStep(void);
uint16_t BSP_Sensorless_GetBemf(void);
uint16_t BSP_Sensorless_GetMid(void);
uint16_t BSP_Sensorless_GetPeriod(void);
uint16_t BSP_Sensorless_GetMissCnt(void);
uint16_t BSP_Sensorless_GetZcCnt(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_SENSORLESS_H */
