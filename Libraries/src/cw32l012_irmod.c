/**
 * @file cw32l012_irmod.c
 * @author WHXY
 * @brief
 * @version 0.1
 * @date 2024-08-07
 *
 * @copyright Copyright (c) 2021
 *
 ******************************************************************************
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
 ******************************************************************************
 */

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "cw32l012_irmod.h"


/*******************************************************************************
  * @brief  配置红外调制方式
  * @param  ModulateMod : 红外调制方式
                          @ref IRMOD_IRSW_AND_GTIM1_CH1
                          @ref IRMOD_IRSW_OR_GTIM2_CH1
                          @ref IRMOD_GTIM1_CH1_AND_GTIM3_CH1
                          @ref IRMOD_GTIM2_CH1_OR_GTIM4_CH1
                          @ref IRMOD_UART1_TXD_AND_GTIM1_CH2
                          @ref IRMOD_UART1_TXD_OR_GTIM2_CH2
                          @ref IRMOD_UART1_TXD_AND_GTIM3_CH2
                          @ref IRMOD_UART1_TXD_OR_GTIM4_CH2
                          @ref IRMOD_UART2_TXD_AND_GTIM1_CH3
                          @ref IRMOD_UART2_TXD_OR_GTIM2_CH3
                          @ref IRMOD_UART2_TXD_AND_GTIM3_CH3
                          @ref IRMOD_UART2_TXD_OR_GTIM4_CH3
                          @ref IRMOD_UART3_TXD_AND_GTIM1_CH4
                          @ref IRMOD_UART3_TXD_OR_GTIM2_CH4
                          @ref IRMOD_UART3_TXD_AND_GTIM3_CH4
                          @ref IRMOD_UART3_TXD_OR_GTIM4_CH4
  * @param  Polarity    : 红外输出极性
  * @retval None
  */
void IRMOD_Config(IRMOD_ModulateModTypeDef ModulateMod, IRMOD_PolarityTypeDef Polarity)
{
    CW_IRMOD->CR = ModulateMod;
    REGBITS_MODIFY(CW_IRMOD->CR,
                   IRMOD_CR_INV_Msk | IRMOD_CR_MOD_Msk,
                   (uint32_t)ModulateMod | (uint32_t)Polarity);
}


/*******************************************************************************
  * @brief  设置红外调制软件控制为高
  * @param  None
  * @retval None
  */
void IRMOD_IRSW_SetHigh(void)
{
    IRMOD_IRSW_SETHIGH();
}


/*******************************************************************************
  * @brief  设置红外调制软件控制为低
  * @param  None
  * @retval None
  */
void IRMOD_IRSW_SetLow(void)
{
    IRMOD_IRSW_SETLOW();
}


/*********************************END OF FILE**********************************/
