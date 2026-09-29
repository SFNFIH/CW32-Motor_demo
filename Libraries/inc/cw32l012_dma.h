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
#ifndef __CW32L012_DMA_H__
#define __CW32L012_DMA_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* 头文件包含部分 */
#include "system_cw32l012.h"

/* 宏定义部分 */

#define DMA_BASE_ADDRESS 0x40020000

// 定义寄存器地址
#define DMA_ISR (volatile uint32_t *)(DMA_BASE_ADDRESS + 0x00)
#define DMA_ICR (volatile uint32_t *)(DMA_BASE_ADDRESS + 0x04)
#define DMA_CSRy(y) (volatile uint32_t *)(DMA_BASE_ADDRESS + 0x00 + 32 * (y+1))
#define DMA_CNTy(y) (volatile uint32_t *)(DMA_BASE_ADDRESS + 0x04 + 32 * (y+1))
#define DMA_SRCADDRy(y) (volatile uint32_t *)(DMA_BASE_ADDRESS + 0x08 + 32 * (y+1))
#define DMA_DSTADDRy(y) (volatile uint32_t *)(DMA_BASE_ADDRESS + 0x0C + 32 * (y+1))
#define DMA_TRIGy(y) (volatile uint32_t *)(DMA_BASE_ADDRESS + 0x10 + 32 * (y+1))
#define DMA_CCNTy(y) (volatile uint32_t *)(DMA_BASE_ADDRESS + 0x14 + 32 * (y+1))
    
#define DMA_INTERRUPT_TE    DMA_CSR1_TEIE_Msk
#define DMA_INTERRUPT_TC    DMA_CSR1_TCIE_Msk

/* 类型定义部分 */
// DMA通道编号
typedef enum {
    DMA_CHANNEL_1 = 0,
    DMA_CHANNEL_2,
    DMA_CHANNEL_3,
    DMA_CHANNEL_4
} DMA_ChannelTypeDef;

// DMA数据宽度
typedef enum {
    DMA_DATA_SIZE_8BITS = 0,   // 8-bit 数据大小
    DMA_DATA_SIZE_16BITS,      // 16-bit 数据大小
    DMA_DATA_SIZE_32BITS       // 32-bit 数据大小
} DMA_DataSizeTypeDef;

// DMA传输模式
typedef enum {
    DMA_MODE_BULK = 0,         // 批量传输模式
    DMA_MODE_BLOCK            // 块传输模式
} DMA_ModeTypeDef;

// DMA触发类型
typedef enum {
    DMA_TRIGGER_SOFTWARE = 0,  // 软件触发
    DMA_TRIGGER_HARDWARE      // 硬件触发
} DMA_TriggerTypeDef;

typedef enum {
    DMA_STATUS_IDLE = 0,     // 初始状态
    DMA_STATUS_ERROR_ADDR,   // 地址超出范围
    DMA_STATUS_ERROR_STOP,   // 停止请求引起中止
    DMA_STATUS_ERROR_SRC,    // 源地址访问错误
    DMA_STATUS_ERROR_DST,    // 目的地址访问错误
    DMA_STATUS_COMPLETE      // 传输完成
} DMA_StatusTypeDef;

// DMA硬件触发源
typedef enum {
    DMA_TRIGGER_SRC_UART1_RX = 0,   // UART1 接收 Buf 非空事件
    DMA_TRIGGER_SRC_UART1_TX,       // UART1 发送 Buf 空事件
    DMA_TRIGGER_SRC_UART2_RX,       // UART2 接收 Buf 非空事件
    DMA_TRIGGER_SRC_UART2_TX,       // UART2 发送 Buf 空事件
    DMA_TRIGGER_SRC_UART3_RX,       // UART3 接收 Buf 非空事件
    DMA_TRIGGER_SRC_UART3_TX,       // UART3 发送 Buf 空事件
    DMA_TRIGGER_SRC_SPI1_RX,        // SPI1 接收 Buf 非空事件
    DMA_TRIGGER_SRC_SPI1_TX,        // SPI1 发送 Buf 空事件
    DMA_TRIGGER_SRC_SPI2_RX,        // SPI2 接收 Buf 非空事件
    DMA_TRIGGER_SRC_SPI2_TX,        // SPI2 发送 Buf 空事件
    DMA_TRIGGER_SRC_SPI3_RX,        // SPI3 接收 Buf 非空事件
    DMA_TRIGGER_SRC_SPI3_TX,        // SPI3 发送 Buf 空事件
    DMA_TRIGGER_SRC_ADC1_SEQ,       // ADC1 序列转换完成事件
    DMA_TRIGGER_SRC_ADC1_SINGLE,    // ADC1 单次转换完成事件
    DMA_TRIGGER_SRC_ADC2_SEQ,       // ADC2 序列转换完成事件
    DMA_TRIGGER_SRC_ADC2_SINGLE,    // ADC2 单次转换完成事件
    DMA_TRIGGER_SRC_DAC1_DHR_OVF,   // DAC1 DHR下溢出事件
    DMA_TRIGGER_SRC_DAC2_DHR_OVF,   // DAC2 DHR下溢出事件
    DMA_TRIGGER_SRC_HALLTIM,        // HALLTIM事件
    DMA_TRIGGER_SRC_BTIM1_UPD,      // BTIM1 更新事件
    DMA_TRIGGER_SRC_BTIM1_TRIG,     // BTIM1 触发事件
    DMA_TRIGGER_SRC_BTIM2_UPD,      // BTIM2 更新事件
    DMA_TRIGGER_SRC_BTIM2_TRIG,     // BTIM2 触发事件
    DMA_TRIGGER_SRC_BTIM3_UPD,      // BTIM3 更新事件
    DMA_TRIGGER_SRC_BTIM3_TRIG,     // BTIM3 触发事件
    DMA_TRIGGER_SRC_GTIM1_TRIG,     // GTIM1 触发事件
    DMA_TRIGGER_SRC_GTIM1_UPD,      // GTIM1 更新事件
    DMA_TRIGGER_SRC_GTIM1_CC1,      // GTIM1 捕获/比较1事件
    DMA_TRIGGER_SRC_GTIM1_CC2,      // GTIM1 捕获/比较2事件
    DMA_TRIGGER_SRC_GTIM1_CC3,      // GTIM1 捕获/比较3事件
    DMA_TRIGGER_SRC_GTIM1_CC4,      // GTIM1 捕获/比较4事件
    DMA_TRIGGER_SRC_GTIM2_TRIG,     // GTIM2 触发事件
    DMA_TRIGGER_SRC_GTIM2_UPD,      // GTIM2 更新事件
    DMA_TRIGGER_SRC_GTIM2_CC1,      // GTIM2 捕获/比较1事件
    DMA_TRIGGER_SRC_GTIM2_CC2,      // GTIM2 捕获/比较2事件
    DMA_TRIGGER_SRC_GTIM2_CC3,      // GTIM2 捕获/比较3事件
    DMA_TRIGGER_SRC_GTIM2_CC4,      // GTIM2 捕获/比较4事件
    DMA_TRIGGER_SRC_GTIM3_TRIG,     // GTIM3 触发事件
    DMA_TRIGGER_SRC_GTIM3_UPD,      // GTIM3 更新事件
    DMA_TRIGGER_SRC_GTIM3_CC1,      // GTIM3 捕获/比较1事件
    DMA_TRIGGER_SRC_GTIM3_CC2,      // GTIM3 捕获/比较2事件
    DMA_TRIGGER_SRC_GTIM3_CC3,      // GTIM3 捕获/比较3事件
    DMA_TRIGGER_SRC_GTIM3_CC4,      // GTIM3 捕获/比较4事件
    DMA_TRIGGER_SRC_GTIM4_TRIG,     // GTIM4 触发事件
    DMA_TRIGGER_SRC_GTIM4_UPD,      // GTIM4 更新事件
    DMA_TRIGGER_SRC_GTIM4_CC1,      // GTIM4 捕获/比较1事件
    DMA_TRIGGER_SRC_GTIM4_CC2,      // GTIM4 捕获/比较2事件
    DMA_TRIGGER_SRC_GTIM4_CC3,      // GTIM4 捕获/比较3事件
    DMA_TRIGGER_SRC_GTIM4_CC4,      // GTIM4 捕获/比较4事件
    DMA_TRIGGER_SRC_ATIM_UPD,       // ATIM 更新事件
    DMA_TRIGGER_SRC_ATIM_CC1,       // ATIM 捕获/比较1事件
    DMA_TRIGGER_SRC_ATIM_CC2,       // ATIM 捕获/比较2事件
    DMA_TRIGGER_SRC_ATIM_CC3,       // ATIM 捕获/比较3事件
    DMA_TRIGGER_SRC_ATIM_CC4,       // ATIM 捕获/比较4事件
    DMA_TRIGGER_SRC_ATIM_CC5,       // ATIM 捕获/比较5事件
    DMA_TRIGGER_SRC_ATIM_CC6,       // ATIM 捕获/比较6事件
    DMA_TRIGGER_SRC_ATIM_COM,       // ATIM COM事件
    DMA_TRIGGER_SRC_ATIM_TRIG,      // ATIM 触发事件
    DMA_TRIGGER_SRC_CORDIC_IDLE,    // cordic_IDLE事件
    DMA_TRIGGER_SRC_CORDIC_EOC,     // cordic_EOC事件
    DMA_TRIGGER_SRC_LPI2C1_TX,      // LPI2C1 发送事件
    DMA_TRIGGER_SRC_LPI2C1_RX,      // LPI2C1 接收事件
    DMA_TRIGGER_SRC_LPI2C2_TX,      // LPI2C2 发送事件
    DMA_TRIGGER_SRC_LPI2C2_RX       // LPI2C2 接收事件
} DMA_TriggerSourceTypeDef;

// DMA传输状态标志
typedef enum {
    DMA_STATUS_TC = 0x01,  // 传输完成标志
    DMA_STATUS_TE = 0x02   // 传输错误标志
} DMA_TransferStatusTypeDef;

// DMA地址增量模式
typedef enum {
    DMA_ADDRESS_FIXED = 0,   // 地址固定
    DMA_ADDRESS_INCREMENT    // 地址自增
} DMA_AddressModeTypeDef;

//
typedef struct {    
    // DMA_CSRy寄存器相关配置
    boolean_t RestartEnable;                        // 自动重传使能（0：禁用，1：启用）
    DMA_DataSizeTypeDef DataSize;                   // 数据位宽
    DMA_AddressModeTypeDef SrcIncrement;            // 源地址增量方式
    DMA_AddressModeTypeDef DstIncrement;            // 目的地址增量方式
    DMA_ModeTypeDef TransferMode;                   // 传输模式（批量或块传输）
//    boolean_t TransferErrorInterruptEnable;         // 传输错误中断使能（0：禁用，1：启用）
//    boolean_t TransferCompleteInterruptEnable;      // 传输完成中断使能（0：禁用，1：启用）

    // DMA_CNTy寄存器相关配置
    uint16_t TransferCount;                         // 待传输的任务数量
    uint8_t BlockCount;                             // 每个传输任务的数据块数量

    // DMA_SRCADDRy和DMA_DSTADDRy寄存器相关配置
    uint32_t SrcAddress;                            // 源地址
    uint32_t DstAddress;                            // 目的地址

    // DMA_TRIGy寄存器相关配置
    DMA_TriggerTypeDef TriggerType;                 // 触发类型（软件或硬件触发）
    DMA_TriggerSourceTypeDef TriggerSource;         // 硬件触发源
} DMA_InitTypeDef;
/* 全局变量声明部分 */


/* 函数声明部分 */
void DMA_DeInit(DMA_ChannelTypeDef Channel);
// DMA初始化
void DMA_Init(DMA_ChannelTypeDef Channel, DMA_InitTypeDef *DMAInitStruct);
// 启用DMA通道
void DMA_Cmd(DMA_ChannelTypeDef channel, FunctionalState state);
// 启动DMA传输
void DMA_StartTransfer(DMA_ChannelTypeDef channel);
// 检查DMA传输状态
DMA_StatusTypeDef DMA_CheckStatus(DMA_ChannelTypeDef channel);
// 配置DMA触发源
void DMA_ConfigTriggerSource(DMA_ChannelTypeDef channel, DMA_TriggerTypeDef trigger_type, DMA_TriggerSourceTypeDef trigger_source);
//
void DMA_ITConfig(DMA_ChannelTypeDef channel, uint32_t interrupt_type, FunctionalState state);

void DMA_ClearInterruptFlag(DMA_ChannelTypeDef channel, uint32_t interrupt_type);

// 配置DMA通道
void DMA_ConfigChannel(DMA_ChannelTypeDef channel, uint32_t src_addr, uint32_t dst_addr, uint32_t transfer_size, uint32_t data_size);

#ifdef __cplusplus
}
#endif

#endif
