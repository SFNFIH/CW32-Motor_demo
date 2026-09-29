/**
 * @file cw32l012_dma.c
 * @author WHXY
 * @brief
 * @version 0.1
 * @date 2024-08-07
 *
 * @copyright Copyright (c) 2021
 *
 */

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

/* Includes ------------------------------------------------------------------*/
#include "cw32l012_dma.h"
#include "cw32l012_sysctrl.h"

/* Private_TypesDefinitions --------------------------------------------------*/

/* Private_Defines -----------------------------------------------------------*/

/* Private_Variables ---------------------------------------------------------*/

/* Private_FunctionPrototypes ------------------------------------------------*/

/* Private_Functions ---------------------------------------------------------*/

/* Public_Functions ----------------------------------------------------------*/

/*******************************************************************************
// 说明：去初始化
// 参数：
// 返回值：无
*******************************************************************************/

void DMA_DeInit(DMA_ChannelTypeDef Channel)
{
    __SYSCTRL_DMA_CLK_ENABLE();
    *DMA_CSRy(Channel) = 0x00000000;
    *DMA_CNTy(Channel) = 0x00000000;
    *DMA_SRCADDRy(Channel) = 0x00000000;
    *DMA_DSTADDRy(Channel) = 0x00000000;
    *DMA_TRIGy(Channel) = 0x00000000;
    *DMA_ICR &= ~((DMA_ICR_TC1_Msk | DMA_ICR_TE1_Msk) << (4*(Channel)));    
}

/*******************************************************************************
// 说明：DMA初始化
// 参数：Channel
// 参数：DMAInitStruct
// 返回值：无
*******************************************************************************/
void DMA_Init(DMA_ChannelTypeDef Channel, DMA_InitTypeDef *DMAInitStruct)
{   
    uint32_t Csr = 0;
    /* 使能DMA的APB时钟 */
    __SYSCTRL_DMA_CLK_ENABLE();        
   
    // 配置DMA_CSRy寄存器
    Csr = DMAInitStruct->TransferMode << DMA_CSR1_TRANS_Pos |
          DMAInitStruct->SrcIncrement << DMA_CSR1_SRCINC_Pos |
          DMAInitStruct->DstIncrement << DMA_CSR1_DSTINC_Pos |
          DMAInitStruct->DataSize << DMA_CSR1_SIZE_Pos |
          DMAInitStruct->RestartEnable << DMA_CSR1_RESTART_Pos;
           
    *DMA_CSRy(Channel) = Csr;

    // 配置DMA_CNTy寄存器
    uint32_t Cnt = (DMAInitStruct->TransferCount & 0xFFFF) | ((DMAInitStruct->BlockCount & 0xF) << 16);
    *DMA_CNTy(Channel) = Cnt;

    // 配置DMA_SRCADDRy和DMA_DSTADDRy寄存器
    *DMA_SRCADDRy(Channel) = DMAInitStruct->SrcAddress;
    *DMA_DSTADDRy(Channel) = DMAInitStruct->DstAddress;

    // 配置DMA_TRIGy寄存器
    uint32_t Trig = (DMAInitStruct->TriggerType << 0) | (DMAInitStruct->TriggerSource << 2);
    *DMA_TRIGy(Channel) = Trig; 
}

// 启用DMA通道
void DMA_Cmd(DMA_ChannelTypeDef channel, FunctionalState state)
{
    state ? (*DMA_CSRy(channel) |= DMA_CSR1_EN_Msk) : (*DMA_CSRy(channel) &= ~DMA_CSR1_EN_Msk);
}

// 软件启动DMA传输
void DMA_StartTransfer(DMA_ChannelTypeDef channel)
{
    uint32_t trig_value = *DMA_TRIGy(channel);
    
    trig_value &= ~(DMA_TRIG1_SOFTSRC_Msk | DMA_TRIG1_TYPE_Msk);
    trig_value |= DMA_TRIG1_SOFTSRC_Msk;
    *DMA_TRIGy(channel) = trig_value;   
}

// 检查DMA传输状态
DMA_StatusTypeDef DMA_CheckStatus(DMA_ChannelTypeDef channel)
{
    return (DMA_StatusTypeDef)((*DMA_CSRy(channel) & DMA_CSR1_STATUS_Msk) >> DMA_CSR1_STATUS_Pos);
}

// 配置DMA触发源
void DMA_ConfigTriggerSource(DMA_ChannelTypeDef channel, DMA_TriggerTypeDef trigger_type, DMA_TriggerSourceTypeDef trigger_source)
{
    uint32_t trig_value = *DMA_TRIGy(channel);
    
    trig_value &= DMA_TRIG1_HARDSRC_Msk | DMA_TRIG1_TYPE_Msk;
    trig_value |= (trigger_type << DMA_TRIG1_TYPE_Pos) | (trigger_source << DMA_TRIG1_HARDSRC_Pos);
    *DMA_TRIGy(channel) = trig_value;
}

//配置DMA中断
void DMA_ITConfig(DMA_ChannelTypeDef channel, uint32_t interrupt_type, FunctionalState state)
{
    state ? (*DMA_CSRy(channel) |= (interrupt_type)) :(*DMA_CSRy(channel) &= ~(interrupt_type));
}

void DMA_ClearInterruptFlag(DMA_ChannelTypeDef channel, uint32_t interrupt_type)
{    
    *DMA_ICR &=  ~(interrupt_type << channel);    
}

// 配置DMA通道
void DMA_ConfigChannel(DMA_ChannelTypeDef channel, uint32_t src_addr, uint32_t dst_addr, uint32_t transfer_size, uint32_t data_size)
{
    // 先禁止DMA通道，再配置
    *DMA_CSRy(channel) &= ~DMA_CSR1_EN_Msk;
    
    // 配置源地址
    *DMA_SRCADDRy(channel) = src_addr;

    // 配置目的地址
    *DMA_DSTADDRy(channel) = dst_addr;

    // 配置传输数量
    *DMA_CNTy(channel) = (transfer_size & 0xFFFF);  // XFERCNT字段

    // 配置数据位宽
    uint32_t csr = *DMA_CSRy(channel);
    csr &= ~DMA_CSR1_SIZE_Msk;  // 清除SIZE字段
    csr |= ((data_size & DMA_CSR1_SIZE_Msk) << DMA_CSR1_SIZE_Pos);  // 设置SIZE字段
    *DMA_CSRy(channel) = csr;

    // 启用通道
    *DMA_CSRy(channel) |= DMA_CSR1_EN_Msk;  // 设置EN位    
}
