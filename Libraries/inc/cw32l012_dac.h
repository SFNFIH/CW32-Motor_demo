/*******************************************************************************
*
* 代码许可和免责信息
* 武汉芯源半导体有限公司授予您使用所有编程代码示例的非专属的版权许可，您可以由此
* 生成根据您的特定需要而定制的相似功能。根据不能被排除的任何法定保证，武汉芯源半
* 导体有限公司及其程序开发商和供应商对程序或技术支持（如果有）不提供任何明示或暗
* 含的保证或条件，包括但不限于暗含的有关适销性、适用于某种特定用途和非侵权的保证
* 或条件。
* 无论何种情形，武汉芯源半导体有限公司及其程序开发商或供应商均不对下列各项负责，
* 即使被告知其发生的可能性时，也是如此：数据的丢失或损坏；直接的、特别的、附带的
* 或间接的损害，或任何后果性经济损害；或利润、业务、收入、商誉或预期可节省金额的
* 损失。
* 某些司法辖区不允许对直接的、附带的或后果性的损害有任何的排除或限制，因此某些或
* 全部上述排除或限制可能并不适用于您。
*
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CW32L012_DAC_H__
#define __CW32L012_DAC_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* 头文件包含部分 */
#include "system_cw32l012.h"

/* 宏定义部分 */

// DAC输出管脚
#define AFx_DACOUT1_PB00()              (CW_GPIOB->ANALOG_f.PIN0 = 1U)
#define AFx_DACOUT2_PB01()              (CW_GPIOB->ANALOG_f.PIN1 = 1U)
// DAC通道选择
#define DAC_CHANNEL_1       (0x00)
#define DAC_CHANNEL_2       (0x01)

// DAC输出使能
#define DAC_OUT_DISABLE     (0x00)
#define DAC_OUT_ENABLE      (0x01)

// DAC数据对齐方式
#define DAC_ALIGN_12B_R     (0x00)
#define DAC_ALIGN_12B_L     (0x01)
#define DAC_ALIGN_8B_R      (0x02)

// DAC中断
#define DAC_IT_DMA_UNDER    (0x8000)

/* 类型定义部分 */
// DAC触发源枚举类型
typedef enum {
    DAC_TRIG_SOFTWARE = 0x0000,    // 软件触发
    DAC_TRIG_BTIM1_Trgo = 0x0001,  // 基本定时器1 TRGO
    DAC_TRIG_BTIM2_Trgo = 0x0002,  // 基本定时器2 TRGO
    DAC_TRIG_BTIM3_Trgo = 0x0003,  // 基本定时器3 TRGO
    DAC_TRIG_GTIM1_Trgo = 0x0004,  // 通用定时器1 TRGO
    DAC_TRIG_GTIM2_Trgo = 0x0005,  // 通用定时器2 TRGO
    DAC_TRIG_GTIM3_Trgo = 0x0006,  // 通用定时器3 TRGO
    DAC_TRIG_GTIM4_Trgo = 0x0007,  // 通用定时器4 TRGO
    DAC_TRIG_ATIM_Trgo = 0x0008,   // 高级定时器 TRGO
    DAC_TRIG_LPTIM_OV = 0x0009,    // 低功耗定时器溢出
    DAC_TRIG_BTIM1_ETR = 0x000A,   // 基本定时器1 ETR
    DAC_TRIG_BTIM2_ETR = 0x000B,   // 基本定时器2 ETR
    DAC_TRIG_BTIM3_ETR = 0x000C,   // 基本定时器3 ETR
    DAC_TRIG_GTIM1_ETR = 0x000D,   // 通用定时器1 ETR
    DAC_TRIG_GTIM2_ETR = 0x000E,   // 通用定时器2 ETR
    DAC_TRIG_GTIM3_ETR = 0x000F    // 通用定时器3 ETR
} DAC_TriggerSourceTypeDef;

// DAC通道1和通道2使能状态
typedef enum {
    DAC_CHANNEL_DISABLE = 0,
    DAC_CHANNEL_ENABLE = 1
} DAC_ChannelStateTypeDef;

// DAC通道1和通道2触发使能状态
typedef enum {
    DAC_TRIGGER_DISABLE = 0,
    DAC_TRIGGER_ENABLE = 1
} DAC_TriggerStateTypeDef;

// DAC通道1和通道2波形生成模式
typedef enum {
    DAC_WAVE_DISABLE = 0,   // 禁止生成波
    DAC_WAVE_NOISE = 1,     // 生成噪声波
    DAC_WAVE_TRIANGLE = 2    // 生成三角波
} DAC_WaveformModeTypeDef;

// DAC通道1和通道2DMA使能状态
typedef enum {
    DAC_DMA_DISABLE = 0,
    DAC_DMA_ENABLE = 1
} DAC_DMAStateTypeDef;

// DAC通道1和通道2DMA下溢中断使能状态
typedef enum {
    DAC_DMA_UNDERFLOW_DISABLE = 0,
    DAC_DMA_UNDERFLOW_ENABLE = 1
} DAC_DMAUnderflowStateTypeDef;

// DAC通道1和通道2振幅选择器
typedef enum {
    DAC_TriangleAmplitude_1 = 0x0,    // 振幅等于 1
    DAC_TriangleAmplitude_3 = 0x1,    // 振幅等于 3
    DAC_TriangleAmplitude_7 = 0x2,    // 振幅等于 7
    DAC_TriangleAmplitude_15 = 0x3,   // 振幅等于 15
    DAC_TriangleAmplitude_31 = 0x4,   // 振幅等于 31
    DAC_TriangleAmplitude_63 = 0x5,   // 振幅等于 63
    DAC_TriangleAmplitude_127 = 0x6,  // 振幅等于 127
    DAC_TriangleAmplitude_255 = 0x7,  // 振幅等于 255
    DAC_TriangleAmplitude_511 = 0x8,  // 振幅等于 511
    DAC_TriangleAmplitude_1023 = 0x9, // 振幅等于 1023
    DAC_TriangleAmplitude_2047 = 0xA, // 振幅等于 2047
    DAC_TriangleAmplitude_4095 = 0xB  // 振幅等于 4095
} DAC_TriangleAmplitudeTypeDef;

// DAC通道1和通道2噪声选择器
typedef enum {
    DAC_LFSRUnmask_Bit0 = 0x0,       // 不屏蔽 LFSR 的位 0
    DAC_LFSRUnmask_Bits1_0 = 0x1,    // 不屏蔽 LFSR 的位 [1:0]
    DAC_LFSRUnmask_Bits2_0 = 0x2,    // 不屏蔽 LFSR 的位 [2:0]
    DAC_LFSRUnmask_Bits3_0 = 0x3,    // 不屏蔽 LFSR 的位 [3:0]
    DAC_LFSRUnmask_Bits4_0 = 0x4,    // 不屏蔽 LFSR 的位 [4:0]
    DAC_LFSRUnmask_Bits5_0 = 0x5,    // 不屏蔽 LFSR 的位 [5:0]
    DAC_LFSRUnmask_Bits6_0 = 0x6,    // 不屏蔽 LFSR 的位 [6:0]
    DAC_LFSRUnmask_Bits7_0 = 0x7,    // 不屏蔽 LFSR 的位 [7:0]
    DAC_LFSRUnmask_Bits8_0 = 0x8,    // 不屏蔽 LFSR 的位 [8:0]
    DAC_LFSRUnmask_Bits9_0 = 0x9,    // 不屏蔽 LFSR 的位 [9:0]
    DAC_LFSRUnmask_Bits10_0 = 0xA,   // 不屏蔽 LFSR 的位 [10:0]
    DAC_LFSRUnmask_Bits11_0 = 0xB    // 不屏蔽 LFSR 的位 [11:0]
} DAC_NoiseLFSRUnmaskBitsTypeDef;

typedef struct
{
    DAC_TriggerSourceTypeDef DAC_TriggerSource;
    DAC_ChannelStateTypeDef  DAC_ChannelState;
    DAC_TriggerStateTypeDef  DAC_TriggerState;
    DAC_WaveformModeTypeDef  DAC_WaveformMode;
    DAC_DMAStateTypeDef      DAC_DMAState;
    DAC_DMAUnderflowStateTypeDef DAC_DMAUnderflowState;
    DAC_TriangleAmplitudeTypeDef DAC_TriangleAmplitude;
    DAC_NoiseLFSRUnmaskBitsTypeDef DAC_NoiseLFSRUnmaskBits;                 
}DAC_InitTypeDef;

/* 全局变量声明部分 */


/* 函数声明部分 */
/** @defgroup DAC驱动函数原型
  * @{
  */
void DAC_DeInit(void);
void DAC_Init(uint32_t DAC_Channel, DAC_InitTypeDef* DAC_InitStruct);
void DAC_Cmd(uint32_t DAC_Channel, FunctionalState NewState);
void DAC_ChannelOutCmd(uint32_t DAC_Channel, FunctionalState NewState);
void DAC_SoftwareTrigger(uint32_t DAC_Channel);
void DAC_DualSoftwareTrigger(void);
void DAC_WaveGenerationCmd(uint32_t DAC_Channel, DAC_WaveformModeTypeDef DAC_Wave, FunctionalState NewState);
void DAC_SetChannel1Data(uint32_t DAC_Align, uint16_t Data);
void DAC_SetChannel2Data(uint32_t DAC_Align, uint16_t Data);
void DAC_SetDualChannelData(uint32_t DAC_Align, uint16_t Data1, uint16_t Data2);
uint16_t DAC_GetDataOutputValue(uint32_t DAC_Channel);

/* DMA management functions ***************************************************/
void DAC_DMACmd(uint32_t DAC_Channel, FunctionalState NewState);

/* Interrupts and flags management functions **********************************/
void DAC_ITConfig(uint32_t DAC_Channel, uint32_t DAC_IT, FunctionalState NewState);
FlagStatus DAC_GetFlagStatus(uint32_t DAC_Channel, uint32_t DAC_FLAG);
void DAC_ClearFlag(uint32_t DAC_Channel, uint32_t DAC_FLAG);
ITStatus DAC_GetITStatus(uint32_t DAC_Channel, uint32_t DAC_IT);
void DAC_ClearITPendingBit(uint32_t DAC_Channel, uint32_t DAC_IT);

#ifdef __cplusplus
}
#endif

#endif
