/**
 * @file cw32l012_dac.c
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

/*
    DAC_CHANNEL1 PB00
    DAC_CHANNEL2 PB01
*/

/* Includes ------------------------------------------------------------------*/
#include "cw32l012_sysctrl.h"
#include "cw32l012_dac.h"

/* Private_TypesDefinitions --------------------------------------------------*/

/* Private_Defines -----------------------------------------------------------*/

/* Private_Variables ---------------------------------------------------------*/

/* Private_FunctionPrototypes ------------------------------------------------*/

/* Private_Functions ---------------------------------------------------------*/

/* Public_Functions ----------------------------------------------------------*/
/**
  * @brief  DAC 反初始化
  * @param  
  * @retval 
  */
void DAC_DeInit(void)
{
    __SYSCTRL_DAC_CLK_ENABLE();    
    CW_DAC->CR0 = 0;
    CW_DAC->CR1 = 0;
    CW_DAC->DOR1 = 0;
    CW_DAC->DOR2 = 0;
}

/**
  * @brief  DAC 初始化
  * @param  DAC_Channel : DAC_CHANNEL_1 or DAC_CHANNEL_2
  *         DAC_InitStruct    
  * @retval 
  */
void DAC_Init(uint32_t DAC_Channel, DAC_InitTypeDef* DAC_InitStruct)
{
    uint32_t ChannelShift = 0, RegTmp =0, RegMAMP = 0;
    
    __SYSCTRL_DAC_CLK_ENABLE();
    
    if (DAC_Channel == DAC_CHANNEL_1)
    {
        ChannelShift = 0;
        CW_DAC->CR0_f.EN1 = 0;    // 需关闭EN，才能设置
    }
    else
    {
        ChannelShift = 16;
        CW_DAC->CR0_f.EN2 = 0;
    }
    
    
    
    if (DAC_InitStruct->DAC_WaveformMode == DAC_WAVE_NOISE)
    {
        RegMAMP = (uint32_t) DAC_InitStruct->DAC_NoiseLFSRUnmaskBits;        
    }
    else if (DAC_InitStruct->DAC_WaveformMode == DAC_WAVE_TRIANGLE)
    {
        RegMAMP = (uint32_t) DAC_InitStruct->DAC_TriangleAmplitude;
    }
    else
    {
        RegMAMP = 0;
    }
    
    RegTmp = DAC_InitStruct->DAC_DMAUnderflowState << DAC_CR0_DMAUDRIE1_Pos |
             DAC_InitStruct->DAC_DMAState << DAC_CR0_DMAEN1_Pos |
             RegMAMP << DAC_CR0_MAMP1_Pos |
             DAC_InitStruct->DAC_WaveformMode << DAC_CR0_WAVE1_Pos |
             DAC_InitStruct->DAC_TriggerSource << DAC_CR0_TSEL1_Pos |
             DAC_InitStruct->DAC_TriggerState << DAC_CR0_TEN1_Pos |
             DAC_InitStruct->DAC_ChannelState << DAC_CR0_EN1_Pos;
    CW_DAC->CR0 = (CW_DAC->CR0 & (~(0x0000FFFFUL <<ChannelShift))) |
                  (RegTmp << ChannelShift);
    
}

/**
  * @brief  DAC使能控制
  * @param  DAC_Channel : DAC_CHANNEL_1 or DAC_CHANNEL_2
  *         NewState : ENABLE, DISABLE    
  * @retval 
  */
void DAC_Cmd(uint32_t DAC_Channel, FunctionalState NewState)
{
    uint32_t ChannelShift = 0;
    
    if (DAC_Channel == DAC_CHANNEL_1)
    {
        ChannelShift = 0;
    }
    else
    {
        ChannelShift = 16;
    }
    
    if (NewState == ENABLE)
    {
        CW_DAC->CR0 |= (DAC_CR0_EN1_Msk << ChannelShift);
    }
    else
    {
        CW_DAC->CR0 &= ~(DAC_CR0_EN1_Msk << ChannelShift);
    }    
}

/**
  * @brief  DAC输出控制
  * @param  DAC_Channel : DAC_CHANNEL_1 or DAC_CHANNEL_2
  *         NewState : ENABLE, DISABLE    
  * @retval 
  */
void DAC_ChannelOutCmd(uint32_t DAC_Channel, FunctionalState NewState)
{
    if (DAC_Channel == DAC_CHANNEL_1)
    {
        NewState ? (CW_DAC->CR1 |= DAC_CR1_C1OUT_Msk) : (CW_DAC->CR1 &= ~DAC_CR1_C1OUT_Msk);
    }
    else
    {
        NewState ? (CW_DAC->CR1 |= DAC_CR1_C2OUT_Msk) : (CW_DAC->CR1 &= ~DAC_CR1_C2OUT_Msk);
    }   
}

/**
  * @brief  DAC软件触发
  * @param  DAC_Channel : DAC_CHANNEL_1 or DAC_CHANNEL_2
  *           
  * @retval 
  */
void DAC_SoftwareTrigger(uint32_t DAC_Channel)
{
    if (DAC_Channel == DAC_CHANNEL_1)
    {
        CW_DAC->SWTRGR_f.SWTRIG1 = 1;
    }
    else
    {
        CW_DAC->SWTRGR_f.SWTRIG2 = 1;
    }   
}

/**
  * @brief  DAC双通道软件触发控制
  * @param  
  *          
  * @retval 
  */
void DAC_DualSoftwareTrigger(void)
{
    CW_DAC->SWTRGR = 0x03;
}

/**
  * @brief  DAC输出波形类型控制
  * @param  DAC_Channel : DAC_CHANNEL_1 or DAC_CHANNEL_2         
  *         Amplitude :
  * @retval 
  */
void DAC_TriangleWaveGenerate(uint32_t DAC_Channel, DAC_TriangleAmplitudeTypeDef Amplitude)
{
    uint32_t RegBackup = CW_DAC->CR0;
    
    if (DAC_Channel == DAC_CHANNEL_1)
    {        
        CW_DAC->CR0_f.TEN1 = 0;         // 禁止时，才能配置
        CW_DAC->CR0_f.EN1 = 0;
        RegBackup = (RegBackup & (~(DAC_CR0_MAMP1_Msk|DAC_CR0_WAVE1_Msk))) | (Amplitude<<DAC_CR0_MAMP1_Pos) |\
                    (DAC_WAVE_TRIANGLE << DAC_CR0_WAVE1_Pos);    
    }
    else
    {
        CW_DAC->CR0_f.TEN2 = 0;         // 禁止时，才能配置
        CW_DAC->CR0_f.EN2 = 0;
        RegBackup = (RegBackup & (~(DAC_CR0_MAMP2_Msk|DAC_CR0_WAVE1_Msk))) | (Amplitude<<DAC_CR0_MAMP2_Pos) |\
                    (DAC_WAVE_TRIANGLE << DAC_CR0_WAVE2_Pos);        
    }
}

/**
  * @brief  DAC通道1输出幅值控制
  * @param  DAC_Align : DAC_ALIGN_12B_R or DAC_ALIGN_12B_L or DAC_ALIGN_8B_R         
  *         Data :
  * @retval 
  */
void DAC_SetChannel1Data(uint32_t DAC_Align, uint16_t Data)
{
    switch (DAC_Align)
    {
        case DAC_ALIGN_12B_R:
            CW_DAC->DHR12R1 = Data;
            break;             
        case DAC_ALIGN_12B_L:
            CW_DAC->DHR12L1 = Data;
            break;
        case DAC_ALIGN_8B_R:
            CW_DAC->DHR8R1 = Data;
            break;
        default:
            break;  
    }
}

/**
  * @brief  DAC通道2输出幅值控制
  * @param  DAC_Align : DAC_ALIGN_12B_R or DAC_ALIGN_12B_L or DAC_ALIGN_8B_R         
  *         Data :
  * @retval 
  */
void DAC_SetChannel2Data(uint32_t DAC_Align, uint16_t Data)
{
    switch (DAC_Align)
    {
        case DAC_ALIGN_12B_R:
            CW_DAC->DHR12R2 = Data;
            break;             
        case DAC_ALIGN_12B_L:
            CW_DAC->DHR12L2 = Data;
            break;
        case DAC_ALIGN_8B_R:
            CW_DAC->DHR8R2 = Data;
            break;
        default:
            break;  
    }    
}

/**
  * @brief  DAC双通道输出幅值控制
  * @param  DAC_Align : DAC_ALIGN_12B_R or DAC_ALIGN_12B_L or DAC_ALIGN_8B_R         
  *         Data1 : 通道1
  *         Data1 : 通道2
  * @retval 
  */
void DAC_SetDualChannelData(uint32_t DAC_Align, uint16_t Data1, uint16_t Data2)
{
    uint32_t data;
    
    switch (DAC_Align)
    {
        case DAC_ALIGN_12B_R:
            data = ((uint32_t)Data2 << 16U) | Data1;
            CW_DAC->DHR12RD = data;
            break;             
        case DAC_ALIGN_12B_L:
            data = ((uint32_t)Data2 << 16U) | Data1;
            CW_DAC->DHR12LD = data;
            break;
        case DAC_ALIGN_8B_R:
            data = ((uint32_t)Data2 << 8U) | Data1;
            CW_DAC->DHR8RD = data;
            break;
        default:
            break;  
    }
}

/**
  * @brief  DAC读取通道输出幅值
  * @param  DAC_Channel :  DAC_CHANNEL_1 or DAC_CHANNEL_2 
  *         
  *         
  * @retval 
  */
uint16_t DAC_GetDataOutputValue(uint32_t DAC_Channel)
{
    uint16_t ret;
    
    if (DAC_Channel == DAC_CHANNEL_1)
    {
        ret = CW_DAC->DOR1 & 0x0FFF;
    }
    else if (DAC_Channel == DAC_CHANNEL_2)
    {
        ret = CW_DAC->DOR2 & 0x0FFF;
    }
    return ret;
}

/* DMA management functions ***************************************************/
/**
  * @brief  DAC的DMA功能控制
  * @param  DAC_Channel :   DAC_CHANNEL_1 or DAC_CHANNEL_2        
  *         NewState : ENABLE or DISABLE
  *        
  * @retval 
  */
void DAC_DMACmd(uint32_t DAC_Channel, FunctionalState NewState)
{
    if (DAC_Channel == DAC_CHANNEL_1)
    {
        NewState ? (CW_DAC->CR0_f.DMAEN1 = 1) : (CW_DAC->CR0_f.DMAEN1 = 0);
    }
    else
    {
        NewState ? (CW_DAC->CR0_f.DMAEN2 = 1) : (CW_DAC->CR0_f.DMAEN2 = 0);
    }
}

/* Interrupts and flags management functions **********************************/
/**
  * @brief  DAC的中断功能配置
  * @param  DAC_Channel :   DAC_CHANNEL_1 or DAC_CHANNEL_2
  *         DAC_IT : DAC_IT_DMA_UNDER
  *         NewState : ENABLE or DISABLE
  *        
  * @retval 
  */
void DAC_ITConfig(uint32_t DAC_Channel, uint32_t DAC_IT, FunctionalState NewState)
{
    if (DAC_Channel == DAC_CHANNEL_1)
    {
        NewState ? (CW_DAC->CR0_f.DMAUDRIE1 = 1) : (CW_DAC->CR0_f.DMAUDRIE1 = 0);
    }
    else
    {
        NewState ? (CW_DAC->CR0_f.DMAUDRIE2 = 1) : (CW_DAC->CR0_f.DMAUDRIE2 = 0);
    }
}

/**
  * @brief  DAC的标志位读取
  * @param  DAC_Channel :   DAC_CHANNEL_1 or DAC_CHANNEL_2
  *         DAC_FLAG : DAC_IT_DMA_UNDER 
  *        
  * @retval 
  */
FlagStatus DAC_GetFlagStatus(uint32_t DAC_Channel, uint32_t DAC_FLAG)
{
    FlagStatus ret;
    if (DAC_Channel == DAC_CHANNEL_1)
    {
        if (DAC_FLAG == DAC_IT_DMA_UNDER)
        {
            ret =  (CW_DAC->DOR1 & DAC_FLAG) ? SET :RESET;
        }            
    }
    else
    {
        if (DAC_FLAG == DAC_IT_DMA_UNDER)
        {
            ret =  (CW_DAC->DOR2 & DAC_FLAG) ? SET :RESET;
        }         
    }
    return ret;    
}

/**
  * @brief  DAC的标志清除
  * @param  DAC_Channel :   DAC_CHANNEL_1 or DAC_CHANNEL_2
  *         DAC_FLAG : DAC_IT_DMA_UNDER 
  *        
  * @retval 
  */
void DAC_ClearFlag(uint32_t DAC_Channel, uint32_t DAC_FLAG)
{
    if (DAC_Channel == DAC_CHANNEL_1)
    {
        if (DAC_FLAG == DAC_IT_DMA_UNDER)
        {
            CW_DAC->DOR1_f.DMAUDR = 0;
        }            
    }
    else
    {
        if (DAC_FLAG == DAC_IT_DMA_UNDER)
        {
            CW_DAC->DOR2_f.DMAUDR = 0;
        }         
    }    
}

/**
  * @brief  DAC的中断标志读取
  * @param  DAC_Channel :   DAC_CHANNEL_1 or DAC_CHANNEL_2
  *         DAC_FLAG : DAC_IT_DMA_UNDER 
  *        
  * @retval 
  */
ITStatus DAC_GetITStatus(uint32_t DAC_Channel, uint32_t DAC_IT)
{
    return DAC_GetFlagStatus(DAC_Channel, DAC_IT);
}

/**
  * @brief  DAC的中断标志清除
  * @param  DAC_Channel :   DAC_CHANNEL_1 or DAC_CHANNEL_2
  *         DAC_FLAG : DAC_IT_DMA_UNDER 
  *        
  * @retval 
  */
void DAC_ClearITPendingBit(uint32_t DAC_Channel, uint32_t DAC_IT)
{
    DAC_ClearFlag(DAC_Channel, DAC_IT);
}
