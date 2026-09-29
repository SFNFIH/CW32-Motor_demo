/**
 * @file cw32l012_halltim.c
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
#include "cw32l012_halltim.h"
#include "cw32l012_sysctrl.h"

/* Private_TypesDefinitions --------------------------------------------------*/

/* Private_Defines -----------------------------------------------------------*/

/* Private_Variables ---------------------------------------------------------*/

/* Private_FunctionPrototypes ------------------------------------------------*/

/* Private_Functions ---------------------------------------------------------*/

/* Public_Functions ----------------------------------------------------------*/

/*******************************************************************************
// 说明：初始化
// 参数：
// 参数：
// 返回值：无
*******************************************************************************/
// 初始化EAU
// 初始化HALLTIM模块
void HALLTIM_Init(HALLTIM_InitTypeDef *init) 
{
    __SYSCTRL_HALLTIM_CLK_ENABLE();
    CW_HALLTIM->CR = (init->filter_enable << HALLTIM_CR_FLT1EN_Pos) |
                      init->clock_division | init->trigger_output |
                      (init->filter_length << HALLTIM_CR_FLT2LEN_Pos);    
    
    // 配置自动重装载值
    CW_HALLTIM->ARR = init->auto_reload_value;    
}


// 启用/禁用HALLTIM模块
void HALLTIM_Cmd(FunctionalState state)
{
    state ? (CW_HALLTIM->CR_f.EN = 1) : (CW_HALLTIM->CR_f.EN = 0);
}



// 设置滤波宽度
void HALLTIM_SetFilterLength(uint32_t length) 
{
    CW_HALLTIM->CR_f.FLT2LEN = length;
}

// 设置触发输出模式
void HALLTIM_SetMode(HALLTIM_MMS mode)
{
    CW_HALLTIM->CR = (CW_HALLTIM->CR & ~HALLTIM_CR_MMS_Msk) | mode;
}

// 设置时钟分频
void HALLTIM_SetClockDivider(HALLTIM_DIV div) 
{
    CW_HALLTIM->CR = (CW_HALLTIM->CR & ~HALLTIM_CR_DIV_Msk) | div;
}

// 生成软件触发信号
void HALLTIM_GenerateSoftTrigger(void) 
{
    CW_HALLTIM->CR_f.SOFTCAP = 1;
}

// 设置自动重装载值
void HALLTIM_SetAutoReloadValue(uint32_t value) 
{
    CW_HALLTIM->ARR = value;
}

// 获取计数值
uint32_t HALLTIM_GetCounterValue(void) 
{
    return CW_HALLTIM->CNT;
}

// 获取脉冲宽度
uint32_t HALLTIM_GetPulseWidth(void) 
{
    return CW_HALLTIM->WIDTH;
}

// 设置比较值
void HALLTIM_SetCompareValue(uint32_t value) 
{
    CW_HALLTIM->CCR = value;
}

// 获取状态标志
uint32_t HALLTIM_GetStatus(void) 
{
    return CW_HALLTIM->ISR;
}

// 清除状态标志
void HALLTIM_ClearStatus(uint32_t flags) 
{
    CW_HALLTIM->ICR = ~flags;
}

// 获取未滤波的HALL信号状态
uint32_t HALLTIM_GetRawStateCH1(void) {
    return CW_HALLTIM->STATE_f.CH1S;
}
uint32_t HALLTIM_GetRawStateCH2(void) {
    return CW_HALLTIM->STATE_f.CH2S;
}
uint32_t HALLTIM_GetRawStateCH3(void) {
    return CW_HALLTIM->STATE_f.CH3S;
}

// 获取滤波后的HALL信号状态
uint32_t HALLTIM_GetFilteredStateCH1(void) {
    return CW_HALLTIM->STATE_f.CH1F;
}
uint32_t HALLTIM_GetFilteredStateCH2(void) {
    return CW_HALLTIM->STATE_f.CH2F;
}
uint32_t HALLTIM_GetFilteredStateCH3(void) {
    return CW_HALLTIM->STATE_f.CH3F;
}

