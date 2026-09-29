/**
 * @file cw32l012_cordic.c
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
#include "cw32l012_sysctrl.h"
#include "cw32l012_cordic.h"

/* Private_TypesDefinitions --------------------------------------------------*/

/* Private_Defines -----------------------------------------------------------*/

/* Private_Variables ---------------------------------------------------------*/

/* Private_FunctionPrototypes ------------------------------------------------*/

/* Private_Functions ---------------------------------------------------------*/

/* Public_Functions ----------------------------------------------------------*/

// 初始化CORDIC模块
void CORDIC_Init(cordic_init_t *init) {
    uint32_t csr_value = 0;
    
    __SYSCTRL_CORDIC_CLK_ENABLE();
    // 配置运算函数
    csr_value |= init->func << CORDIC_CSR_FUNC_Pos; 

    // 配置扩展范围
    csr_value |= init->scale << CORDIC_CSR_SCALE_Pos;

    // 配置数据格式
    csr_value |= init->format << CORDIC_CSR_FORMAT_Pos;

    // 配置迭代次数
    csr_value |= init->iter << CORDIC_CSR_ITER_Pos;

    // 配置伸缩因子补偿
    csr_value |= init->comp << CORDIC_CSR_COMP_Pos;

    // 配置中断使能
    csr_value |= init->ie << CORDIC_CSR_IE_Pos;

    // 配置DMA使能（运算完成）
    csr_value |= init->dmaeoc << CORDIC_CSR_DMAEOC_Pos;

    // 配置DMA使能（空闲状态）
    csr_value |= init->dmaidle << CORDIC_CSR_DMAIDLE_Pos;

    // 写入控制及状态寄存器
    CW_CORDIC->CSR = csr_value;
}

// 获取CORDIC模块状态
cordic_status_t CORDIC_GetStatus(void) {
    cordic_status_t status;
    uint32_t csr_value = CW_CORDIC->CSR;

    status.busy = (csr_value & CORDIC_CSR_BUSY_Msk) ? 1 : 0;
    status.eoc = (csr_value & CORDIC_CSR_EOC_Msk) ? 1 : 0;

    return status;
}

// 设置运算函数
void CORDIC_SetFunction(cordic_func_t func) {
    uint32_t csr_value = CW_CORDIC->CSR;
    csr_value &= ~CORDIC_CSR_FUNC_Msk; // 清除旧的函数配置
    csr_value |= (func & CORDIC_CSR_FUNC_Msk) << CORDIC_CSR_FUNC_Pos; // 设置新的函数配置
    CW_CORDIC->CSR = csr_value;
}

// 设置扩展范围
void CORDIC_SetScale(uint8_t scale) {
    uint32_t csr_value = CW_CORDIC->CSR;
    csr_value &= ~CORDIC_CSR_SCALE_Msk; // 清除旧的扩展范围配置
    csr_value |= (scale << CORDIC_CSR_SCALE_Pos) & CORDIC_CSR_SCALE_Msk; // 设置新的扩展范围配置
    CW_CORDIC->CSR = csr_value;
}

// 设置数据格式
void CORDIC_SetFormat(cordic_format_t format) {
    uint32_t csr_value = CW_CORDIC->CSR;
    csr_value &= ~CORDIC_CSR_FORMAT_Pos; // 清除旧的数据格式配置
    csr_value |= (format << CORDIC_CSR_FORMAT_Pos) & CORDIC_CSR_FORMAT_Msk; // 设置新的数据格式配置
    CW_CORDIC->CSR = csr_value;
}

// 设置迭代次数
void CORDIC_SetIter(uint8_t iter) {
    uint32_t csr_value = CW_CORDIC->CSR;
    csr_value &= ~CORDIC_CSR_ITER_Msk; // 清除旧的迭代次数配置
    csr_value |= (iter << CORDIC_CSR_ITER_Pos) & CORDIC_CSR_ITER_Msk; // 设置新的迭代次数配置
    CW_CORDIC->CSR = csr_value;
}

// 设置伸缩因子补偿
void CORDIC_SetComp(uint8_t comp) {
    uint32_t csr_value = CW_CORDIC->CSR;
    csr_value &= ~CORDIC_CSR_COMP_Msk; // 清除旧的伸缩因子补偿配置
    csr_value |= (comp << CORDIC_CSR_COMP_Pos) & CORDIC_CSR_COMP_Msk; // 设置新的伸缩因子补偿配置
    CW_CORDIC->CSR = csr_value;
}

// 设置中断使能
void CORDIC_SetIE(uint8_t ie) {
    uint32_t csr_value = CW_CORDIC->CSR;
    csr_value &= ~CORDIC_CSR_IE_Msk; // 清除旧的中断使能配置
    csr_value |= (ie << CORDIC_CSR_IE_Pos) & CORDIC_CSR_IE_Msk; // 设置新的中断使能配置
    CW_CORDIC->CSR = csr_value;
}

// 设置DMA使能（运算完成）
void CORDIC_SetDMAEoc(uint8_t dmaeoc) {
    uint32_t csr_value = CW_CORDIC->CSR;
    csr_value &= ~CORDIC_CSR_DMAEOC_Msk; // 清除旧的DMA使能配置
    csr_value |= (dmaeoc << CORDIC_CSR_DMAEOC_Pos) & CORDIC_CSR_DMAEOC_Msk; // 设置新的DMA使能配置
    CW_CORDIC->CSR = csr_value;
}

// 设置DMA使能（空闲状态）
void CORDIC_SetDMAIdle(uint8_t dmaidle) {
    uint32_t csr_value = CW_CORDIC->CSR;
    csr_value &= ~CORDIC_CSR_DMAIDLE_Msk; // 清除旧的DMA使能配置
    csr_value |= (dmaidle << CORDIC_CSR_DMAIDLE_Pos) & CORDIC_CSR_DMAIDLE_Msk; // 设置新的DMA使能配置
    CW_CORDIC->CSR = csr_value;
}

// 启动运算
void CORDIC_StartComputation(void) {
    // 根据文档，启动运算的方式取决于输入数据的写入
    // 这里不直接操作CSR寄存器，而是通过写入输入寄存器启动运算
}

// 将浮点数转换为Q1.31格式
int32_t float_to_q1_31(float value) {
    return (int32_t)round(value * 2147483648.0); // 2^31
}

// 将Q1.31格式转换为浮点数
float q1_31_to_float(int32_t value) {
    return (float)value / 2147483648.0; // 2^31
}

// 浮点数转换为Q1.15格式的定点数
int32_t float_to_q1_15(float value)
{
    return (int32_t)round(value *  32768.0f);
}

// Q1.15格式的定点数转换为浮点数
float q1_15_to_float(int32_t value)
{
    return (float)value / 32768.0f;
}


//=============================================================================

