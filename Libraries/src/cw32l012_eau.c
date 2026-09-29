/**
 * @file cw32l012_eau.c
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
#include "cw32l012_eau.h"
#include "cw32l012_sysctrl.h"

/* Private_TypesDefinitions --------------------------------------------------*/

/* Private_Defines -----------------------------------------------------------*/

/* Private_Variables ---------------------------------------------------------*/

/* Private_FunctionPrototypes ------------------------------------------------*/

/* Private_Functions ---------------------------------------------------------*/

/* Public_Functions ----------------------------------------------------------*/

/*******************************************************************************
// 说明：EAU初始化
// 参数：
// 参数：
// 返回值：无
*******************************************************************************/
// 初始化EAU
void EAU_Init(void) {
    // 将控制和状态寄存器清零，确保初始状态
    __SYSCTRL_EAU_CLK_ENABLE();
    CW_EAU->CSR = 0x00000000;
}

// 设置EAU运算模式
void EAU_SetMode(EAU_Mode mode) {
    // 检查是否忙碌，若忙碌则不修改模式
    if ((CW_EAU->CSR & EAU_CSR_BUSY_Msk) == 0) {
        CW_EAU->CSR = (CW_EAU->CSR & ~EAU_CSR_MODE_Msk) | mode;
    }
}

// 开始运算操作
void EAU_StartOperation(uint32_t dividend, uint32_t divisor) {
    // 写入被除数/被开方数
    CW_EAU->DIVIDEND = dividend;
    // 写入除数
    CW_EAU->DIVISOR = divisor;
}

// 获取商/平方根
uint32_t EAU_GetQuotient(void) {
    return CW_EAU->QUOTIENT;
}

// 获取余数
uint32_t EAU_GetRemainder(void) {
    return CW_EAU->REMAINDER;
}

// 获取EAU状态
EAU_Status EAU_GetStatus(void) {
    EAU_Status status = EAU_STATUS_IDLE;

    if (CW_EAU->CSR & EAU_CSR_BUSY_Msk) {
        status |= EAU_STATUS_BUSY;
    }

    if (CW_EAU->CSR & EAU_CSR_ZERO_Msk) {
        status |= EAU_STATUS_DIV_ZERO;
    }

    if (CW_EAU->CSR & EAU_CSR_OVR_Msk) {
        status |= EAU_STATUS_OVERFLOW;
    }

    return status;
}

