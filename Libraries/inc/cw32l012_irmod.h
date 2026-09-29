/**
 * @file cw32l012_irmod.h
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


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CW32L012_IRMOD_H
#define __CW32L012_IRMOD_H

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "cw32l012.h"

/** @addtogroup IRMOD
  * @{
  */


/******************************************************************************/
/* Global enum type definitions                                               */
/******************************************************************************/
/** @defgroup IRMOD枚举类型定义
  * @{
  */

/**
  * @brief 红外调制方式配置
  */
typedef enum
{
    IRMOD_IRSW_AND_GTIM1_CH1      = 0x0,   // IRSW & GTIM1_CH1
    IRMOD_IRSW_OR_GTIM2_CH1       = 0x1,   // IRSW | GTIM2_CH1
    IRMOD_GTIM1_CH1_AND_GTIM3_CH1 = 0x2,   // GTIM1_CH1 & GTIM3_CH1
    IRMOD_GTIM2_CH1_OR_GTIM4_CH1  = 0x3,   // GTIM2_CH1 | GTIM4_CH1
    IRMOD_UART1_TXD_AND_GTIM1_CH2 = 0x4,   // UART1_TXD & GTIM1_CH2
    IRMOD_UART1_TXD_OR_GTIM2_CH2  = 0x5,   // UART1_TXD | GTIM2_CH2
    IRMOD_UART1_TXD_AND_GTIM3_CH2 = 0x6,   // UART1_TXD & GTIM3_CH2
    IRMOD_UART1_TXD_OR_GTIM4_CH2  = 0x7,   // UART1_TXD | GTIM4_CH2
    IRMOD_UART2_TXD_AND_GTIM1_CH3 = 0x8,   // UART2_TXD & GTIM1_CH3
    IRMOD_UART2_TXD_OR_GTIM2_CH3  = 0x9,   // UART2_TXD | GTIM2_CH3
    IRMOD_UART2_TXD_AND_GTIM3_CH3 = 0xA,   // UART2_TXD & GTIM3_CH3
    IRMOD_UART2_TXD_OR_GTIM4_CH3  = 0xB,   // UART2_TXD | GTIM4_CH3
    IRMOD_UART3_TXD_AND_GTIM1_CH4 = 0xC,   // UART3_TXD & GTIM1_CH4
    IRMOD_UART3_TXD_OR_GTIM2_CH4  = 0xD,   // UART3_TXD | GTIM2_CH4
    IRMOD_UART3_TXD_AND_GTIM3_CH4 = 0xE,   // UART3_TXD & GTIM3_CH4
    IRMOD_UART3_TXD_OR_GTIM4_CH4  = 0xF    // UART3_TXD | GTIM4_CH4
}IRMOD_ModulateModTypeDef;

/**
  * @brief IR输出极性配置
  */
typedef enum
{
    IRMOD_POLARITY_POSITIVE              = (0x0UL << IRMOD_CR_INV_Pos),/*!< 正向输出 */
    IRMOD_POLARITY_NEGATIVE              = (0x1UL << IRMOD_CR_INV_Pos) /*!< 反向输入 */
} IRMOD_PolarityTypeDef;

/**
  * @}
  */


/******************************************************************************/
/* Global struct type definitions                                             */
/******************************************************************************/


/******************************************************************************/
/* Global pre-processor symbols/macros ('#define')                            */
/******************************************************************************/


/******************************************************************************/
/* Global macro function                                                      */
/******************************************************************************/
/** @defgroup IRMOD宏函数定义
  * @{
  */

/**
  * @brief  置位 IRMOD_CR 寄存器的红外调制软件控制(IRSW)位
  * @param  None
  * @retval None
  */
#define IRMOD_IRSW_SETHIGH()         (CW_IRMOD->CR |= IRMOD_CR_IRSW_Msk)

/**
  * @brief  重置 IRMOD_CR 寄存器的红外调制软件控制(IRSW)位
  * @param  None
  * @retval None
  */
#define IRMOD_IRSW_SETLOW()          (CW_IRMOD->CR &= ~(IRMOD_CR_IRSW_Msk))

/**
  * @}
  */


/******************************************************************************/
/* Exported variables ('extern', definition in C source)                      */
/******************************************************************************/


/******************************************************************************/
/* Global function prototypes ('extern', definition in C source)              */
/******************************************************************************/
/** @defgroup IRMOD驱动函数原型
  * @{
  */

void IRMOD_Config(IRMOD_ModulateModTypeDef ModulateMod, IRMOD_PolarityTypeDef Polarity);
void IRMOD_IRSW_SetHigh(void);
void IRMOD_IRSW_SetLow(void);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */


#ifdef __cplusplus
}
#endif

#endif /* __SYM32L010_HAL_IRMOD_H */


/************************ (C) COPYRIGHT SIYIMicro *****END OF FILE*************/
