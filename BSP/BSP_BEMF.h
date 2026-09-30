/**
 * @file    BSP_BEMF.h
 * @brief   三相反电势: PA0/PA1/PA2 → ADC1 CH0/1/2
 *
 * 原理图: MCU_EA/EB/EC 与电流 OPA1/2OUT(PB0/1) 独立, 无冲突。
 * ADC1 统一扫描 CH0/1/2+CH8/9; 读 RESULT_0..2 即可。
 * 无驱磁链: 关 PWM 后软件触发读相电压 → Clarke → |Vαβ|/ωe。
 */
#ifndef BSP_BEMF_H
#define BSP_BEMF_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern volatile uint16_t g_bemf_sample[3];

/** 配置 PA0/1/2 模拟输入 */
void BSP_BEMF_Init(void);

/** 确保 ADC1 为统一扫描序列 (软件触发, 无 ATIM IRQ) */
void BSP_BEMF_EnsureAdc(void);

/** 软件启动一次转换, 原始 0..4095; 1=成功 */
uint8_t BSP_BEMF_ReadRaw(uint16_t *a, uint16_t *b, uint16_t *c);

/** 相电压 (对地, V), 经分压还原; 1=成功 */
uint8_t BSP_BEMF_ReadVolt(float *va, float *vb, float *vc);

/** 去共模 + Clarke → αβ */
void BSP_BEMF_Clarke(float va, float vb, float vc, float *valpha, float *vbeta);

/** 兼容旧接口 (读缓存; ReadRaw 会更新) */
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
