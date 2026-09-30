/**
 * @file    BSP_BEMF.h
 * @brief   三相反电势: PA0/PA1/PA2 → ADC1 CH0/1/2
 *
 * 与电流采样共用 ADC1, 通过 Enter/Exit 互斥切换通道配置。
 * 无驱磁链: 关 PWM 后软件触发读相电压 → Clarke → Vq/ωe。
 */
#ifndef BSP_BEMF_H
#define BSP_BEMF_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern volatile uint16_t g_bemf_sample[3];

/** 仅配置 GPIO 模拟输入; 不抢 ADC1 */
void BSP_BEMF_Init(void);

/**
 * 占用 ADC1: CH0/1/2 软件触发三通道 (无 ATIM/IRQ, 不依赖 Sensorless)。
 * 调用前应停 PWM ISR; Exit 后恢复电流通道。
 */
void BSP_BEMF_Enter(void);
void BSP_BEMF_Exit(void);
uint8_t BSP_BEMF_IsActive(void);

/** 软件启动一次转换, 原始 0..4095; 1=成功 */
uint8_t BSP_BEMF_ReadRaw(uint16_t *a, uint16_t *b, uint16_t *c);

/** 相电压 (对地, V), 经分压还原; 1=成功 */
uint8_t BSP_BEMF_ReadVolt(float *va, float *vb, float *vc);

/** 去共模 + Clarke → αβ */
void BSP_BEMF_Clarke(float va, float vb, float vc, float *valpha, float *vbeta);

/** 兼容旧接口 (读缓存; Enter 后 ReadRaw 会更新缓存) */
uint16_t BSP_BEMF_ReadPhase(uint8_t phase);
void BSP_BEMF_Read(uint16_t *a, uint16_t *b, uint16_t *c);
uint16_t BSP_BEMF_ReadA(void);
uint16_t BSP_BEMF_ReadB(void);
uint16_t BSP_BEMF_ReadC(void);

/** 旧 ATIM 触发路径保留空实现, 避免链接 Sensorless */
void BSP_BEMF_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_BEMF_H */
