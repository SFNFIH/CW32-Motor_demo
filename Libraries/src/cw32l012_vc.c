/**
 * @file cw32l012_vc.c
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


/******************************************************************************
 * Include files
 ******************************************************************************/
#include "cw32l012_sysctrl.h"
#include "cw32l012_vc.h"

/******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/


/******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/


/******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/


/*****************************************************************************
 * Function implementation - global ('extern') and local ('static')
 *****************************************************************************/
 /**
 * @brief
 *          初始化VC模块
 *
 * @param   VCx:
 *          @arg CW_VC1, @arg CW_VC2, @arg CW_VC3, @arg CW_VC4
 *       
 * @param   VC_InitStruct:
 *          
 */
void VC_Init(VC_TypeDef *VCx, VC_InitTypeDef* VC_InitStruct)
{    
    __SYSCTRL_VC_CLK_ENABLE();
    // 配置参考电压
    VC_SetRefVoltage(VCx, VC_InitStruct->RefVoltage, VC_InitStruct->div);
    
    // 配置输入信号
    VC_SetInputSignals(VCx, VC_InitStruct->inputPositive, VC_InitStruct->inputNegative);
    
    // 配置中断模式
    VC_SetInterruptMode(VCx, VC_InitStruct->interruptMode);
    
    // 配置滤波器
    VC_SetFilterConfig(VCx, &VC_InitStruct->filterConfig);
    
    // 配置其他控制寄存器
    VCx->CR0_f.POL = VC_InitStruct->polarityInvert;
    VCx->CR0_f.HYS = VC_InitStruct->hysteresisEnable;
    VCx->CR0_f.RESP = VC_InitStruct->responseSpeed;
}


/**
 * @brief
 *          VC模块开启或关闭
 *
 * @param   VCx:
 *          @arg CW_VC1, @arg CW_VC2, @arg CW_VC3, @arg CW_VC4
 *       
 * @param   state: EANBLE DISABLE
 *          
 */
void VC_Cmd(VC_TypeDef *VCx, FunctionalState state)
{
    state ? (VCx->CR0_f.EN = 1) : (VCx->CR0_f.EN = 0);
}

/**
 * @brief
 *          配置参考电压
 *
 * @param   VCx:
 *          @arg CW_VC1, @arg CW_VC2, @arg CW_VC3, @arg CW_VC4
 *       
 * @param   RefVoltage:
 *
 * @param   div:
 *          
 */

void VC_SetRefVoltage(VC_TypeDef *VCx, VC_RefVoltage RefVoltage, VC_Divider div)
{
    if ((uint32_t)VCx == VC1_BASE || (uint32_t)VCx == VC2_BASE)
    {
        CW_VC12REF->REF = (RefVoltage << VC12REF_DIV_EN_Pos) | div;
    }
    else if ((uint32_t)VCx == VC3_BASE || (uint32_t)VCx == VC4_BASE)
    {
        CW_VC34REF->REF = (RefVoltage << VC12REF_DIV_EN_Pos) | div;
    }
}

/**
 * @brief
 *          配置输入信号
 *
 * @param   VCx:
 *          @arg CW_VC1, @arg CW_VC2, @arg CW_VC3, @arg CW_VC4
 *       
 * @param   inputPositive:
 *
 * @param   inputNegative:
 *          
 */
void VC_SetInputSignals(VC_TypeDef *VCx, VC_PosInputSignal inputPositive, VC_NegInputSignal inputNegative)
{
    uint32_t cr0 = VCx->CR0;
    cr0 = (cr0 & ~VCx_CR0_INP_Msk) | (inputPositive << VCx_CR0_INP_Pos); // 设置正端输入
    cr0 = (cr0 & ~VCx_CR0_INN_Msk) | (inputNegative << VCx_CR0_INN_Pos);   // 设置负端输入
    VCx->CR0 = cr0;    
}

/**
 * @brief
 *          配置中断模式
 *
 * @param   VCx:
 *          @arg CW_VC1, @arg CW_VC2, @arg CW_VC3, @arg CW_VC4
 *       
 * @param   mode:
 *      
 */
void VC_SetInterruptMode(VC_TypeDef *VCx, VC_InterruptMode mode)
{
    uint32_t cr1 = VCx->CR1;
    
    if (mode == VC_INT_DISABLE)
    {
        VCx->CR0_f.IE = 0;
    }
    else
    {
        VCx->CR0_f.IE = 1;
        cr1 = (cr1 & ~(0x3 << 5)) | (mode << 5); // 设置中断模式
        VCx->CR1 = cr1;
    }
}

/**
 * @brief
 *          设置滤波器配置
 *
 * @param   VCx:
 *          @arg CW_VC1, @arg CW_VC2, @arg CW_VC3, @arg CW_VC4
 *       
 * @param   config:
 *      
 */
void VC_SetFilterConfig(VC_TypeDef *VCx, VC_FilterConfig *config)
{
    uint32_t cr1 = VCx->CR1;
    
    cr1 = (cr1 & ~VCx_CR1_FLTCLK_Msk) | (config->FilterClock << VCx_CR1_FLTCLK_Pos);
    cr1 = (cr1 & ~VCx_CR1_FLTTIME_Msk) | (config->FilterTime << VCx_CR1_FLTTIME_Pos);
    VCx->CR1 = cr1;
}

/**
 * @brief
 *          获取中断标志
 *
 * @param   VCx:
 *          @arg CW_VC1, @arg CW_VC2, @arg CW_VC3, @arg CW_VC4
 *       
 *      
 */
uint8_t VC_GetInterruptFlag(VC_TypeDef *VCx)
{
    return VCx->SR_f.INTF;
}

/**
 * @brief
 *          清除中断标志
 *
 * @param   VCx:
 *          @arg CW_VC1, @arg CW_VC2, @arg CW_VC3, @arg CW_VC4
 *       
 *      
 */
void VC_ClearInterruptFlag(VC_TypeDef *VCx)
{
    VCx->SR_f.INTF = 0;
}

/**
 * @brief
 *          获取数字滤波器输出
 *
 * @param   VCx:
 *          @arg CW_VC1, @arg CW_VC2, @arg CW_VC3, @arg CW_VC4
 *       
 *      
 */
uint8_t VC_GetFilterOutput(VC_TypeDef *VCx)
{
    return VCx->SR_f.FLTV;
}

/**
 * @brief
 *          BLANK窗口配置
 *
 * @param   VCx:
 *          @arg CW_VC1, @arg CW_VC2, @arg CW_VC3, @arg CW_VC4
 *       
 *      
 */
void VC_SetBlankWindow(VC_TypeDef *VCx, VC_BlankWindowCfg *config)
{
    uint32_t cr1 = VCx->CR1;
    
    cr1 = (cr1 & ~VCx_CR1_BLANKLVL_Msk) | (config->BlankOutput << VCx_CR1_BLANKLVL_Pos);
    cr1 = (cr1 & ~VCx_CR1_BLANKTIME_Msk) | (config->BlankTime << VCx_CR1_BLANKTIME_Pos);
    VCx->CR1 = cr1;
    
    VCx->CR2 = config->BlankTrigSource;
}

//@} // VcGroup


/******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/

