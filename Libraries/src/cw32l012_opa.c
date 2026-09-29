/**
 * @file cw32l012_opa.c
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
    表格 1.  OPA 正向 / 反向 输入端列表:
    +--------------------------------------------------------------------------+
    |                 |        |     OPA1     |     OPA2     |
    |-----------------|--------|--------------|--------------|
    | INN             | INN1   |     PA04     |     PA05     |
    |                 | INN2   |     PA07     |     PA07     |
    |-----------------|--------|--------------|--------------|
    |                 | INP1   |     PA03     |     PA04     |
    | INP             | INP2   |     PA06     |     PA06     |
    |                 | INP3   |     PB01     |     PB00     |
    |                 | INP4   | DAC1_OUTINTL | DAC2_OUTINTL |
    +--------------------------------------------------------------------------+  

    表格 2.  OPA 输出端列表:
    +--------------------------------------------------------------------------+
    |                 |        |     OPA1     |     OPA2     |
    |-----------------|--------|--------------|--------------|
    | Output          |        |     PB00     |     PB01     |
    |-----------------|--------|--------------|--------------|
    | Internal output |        |  ADC1_CH13   |  ADC1_CH12   |
    | to ADCs         |        |  ADC2_CH13   |  ADC2_CH12   |
    |-----------------|--------|--------------|--------------|
    | output to ADCs  |        |ADC1_IN8(PB00)|ADC1_IN9(PB01)|
    | input on GPIO   |        |ADC2_IN3(PB00)|ADC2_IN4(PB01)|
    |-----------------|--------|--------------|--------------|
    | output to VCs   |        |VC1_INP3(PB00)|VC2_INP3(PB01)|
    | input on GPIO   |        |VC3_INP3(PB00)|VC4_INP3(PB01)|
    +--------------------------------------------------------------------------+
*/

/* Includes ------------------------------------------------------------------*/
#include "cw32l012_opa.h"

/* Private_TypesDefinitions --------------------------------------------------*/

/* Private_Defines -----------------------------------------------------------*/

/* Private_Variables ---------------------------------------------------------*/

/* Private_FunctionPrototypes ------------------------------------------------*/

/* Private_Functions ---------------------------------------------------------*/

/* Public_Functions ----------------------------------------------------------*/
/*******************************************************************************
  * @brief  OPA 初始化
  * @param  OPAx      : CW_OPA1/CW_OPA2, 
  *         OPA_InitStruct :见结构体的定义
  * @retval 
  * @note   需通过APBENx寄存器使能OPA的时钟后，才可调用该函数
  */
void OPA_Init(OPA_TypeDef *OPAx, OPA_InitTypeDef *OPA_InitStruct)
{
    uint32_t tmpInputP = OPA_InitStruct->InputP & OPA_INP_MASK;
    uint32_t tmpInputN = (OPA_InitStruct->WorkMode == OPA_WORKMODE_FOLLOWER) ?\
                          0UL : (OPA_InitStruct->InputN & OPA_INN_MASK);
    
    OPAx->CR = tmpInputP | tmpInputN |\
                (uint32_t)OPA_InitStruct->Bias |\
                (uint32_t)OPA_InitStruct->PgaGain |\
                (uint32_t)OPA_InitStruct->WorkMode;
    OPAx->CAL = 0;    
}

/*******************************************************************************
  * @brief  OPA 反初始化
  * @param  OPAx      : CW_OPA1/CW_OPA2
  * @retval none
  * @note   需通过APBENx寄存器使能OPA的时钟后，才可调用该函数
  */
void OPA_DeInit(OPA_TypeDef *OPAx)
{
    OPAx->CR = OPA_CR0_RESET_VALUE;
    OPAx->CAL = 0;

    // 重置OPA外设
    REGBITS_CLR(CW_SYSCTRL->APBRST2, SYSCTRL_APBRST2_OPA_Msk);
    REGBITS_SET(CW_SYSCTRL->APBRST2, SYSCTRL_APBRST2_OPA_Msk);
}

/*******************************************************************************
  * @brief  启动OPA
  * @param  OPAx      : CW_OPA1/CW_OPA2
  * @retval none
  */
void OPA_Start(OPA_TypeDef *OPAx)
{
    REGBITS_SET(OPAx->CR, OPAx_CR_EN_Msk);   
}

/*******************************************************************************
  * @brief  停止OPA
  * @param  OPAx      : CW_OPA1/CW_OPA2
  * @retval none
  */
void OPA_Stop(OPA_TypeDef *OPAx)
{
    REGBITS_CLR(OPAx->CR, OPAx_CR_EN_Msk);
}

/*******************************************************************************
  * @brief  OPA软件校正模式开始校正
  * @param  OPAx      : CW_OPA1/CW_OPA2
  * @retval none
  * @note   1. 软件校正启动后由用户控制校正时间
  * @note   2. 校正完成后需要使用 OPA_AutoZeroSoft_Stop 以停止校正
  */
void OPA_AutoZeroSoft_Start(OPA_TypeDef *OPAx)
{    
    REGBITS_MODIFY(OPAx->CAL, OPAx_CAL_SOFTTRIG_Msk,
                   OPAx_CAL_START_Msk | OPAx_CAL_CALMODE_Msk | OPAx_CAL_CALEN_Msk);
}

/*******************************************************************************
  * @brief  OPA软件校正模式结束校正
  * @param  OPAx      : CW_OPA1/CW_OPA2
  * @retval none
  * @note   需要配合OPA_AutoZeroSoft_Start 使用
  */
void OPA_AutoZeroSoft_Stop(OPA_TypeDef *OPAx)
{    
    REGBITS_CLR(OPAx->CAL, OPAx_CAL_START_Msk | OPAx_CAL_CALMODE_Msk);
    // 结束校正后需要10uS才可使用
    FirmwareDelay(SystemCoreClock / 1000000);
}

/*******************************************************************************
  * @brief  OPA触发校正模式配置
  * @param  OPAx           : CW_OPA1/CW_OPA2
  * @param  AutoZeroClk    : OPA触发校正时钟
  * @param  AutoZeroPeriod : OPA触发校正时长
  * @param  AutoZeroTrigSrc: OPA触发校正触发源
  * @retval none
  * @note   需要在调用初始化之后再调用本函数
  */
void OPA_AutoZeroTrig_Config(OPA_TypeDef *OPAx,
                                 OPA_AutoZeroClockTypeDef AutoZeroClk,
                                 OPA_AutoZeroPeriodTypeDef AutoZeroPeriod,
                                 uint32_t AutoZeroTrigSrc)
{   
    OPAx->CAL = AutoZeroTrigSrc |
                (uint32_t)AutoZeroClk |
                (uint32_t)AutoZeroPeriod |
                OPAx_CAL_CALEN_Msk;    
}


/*******************************************************************************
  * @brief  OPA触发校正模式创建软件触发校正事件
  * @param  OPAx           : CW_OPA1/CW_OPA2
  * @retval none
  * @note   需要配置为触发校正模式，且触发源为软件触发
  */
void OPA_AutoZeroTrig_SoftEvent(OPA_TypeDef *OPAx)
{    
    if ((OPAx->CAL & (OPAx_CAL_CALMODE_Msk | OPAx_CAL_CALEN_Msk)) == OPAx_CAL_CALEN_Msk)
    {
        OPA_AUTOZEROTRIG_SOFTEVENT(OPAx);        
    }   
}

