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
#ifndef __CW32L012_EAU_H__
#define __CW32L012_EAU_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* 头文件包含部分 */
#include "system_cw32l012.h"

/* 宏定义部分 */


/* 类型定义部分 */
// EAU运算模式枚举
typedef enum {
    EAU_MODE_UNSIGNED_DIV = 0x00, // 无符号除法
    EAU_MODE_SIGNED_DIV = 0x01,  // 有符号除法
    EAU_MODE_SQRT = 0x02,        // 开方
    EAU_MODE_RESERVED = 0x03     // 保留
} EAU_Mode;

// EAU状态标志枚举
typedef enum {
    EAU_STATUS_IDLE = 0x00,      // 空闲状态
    EAU_STATUS_BUSY = 0x01,      // 忙碌状态
    EAU_STATUS_DIV_ZERO = 0x02,  // 除数为零
    EAU_STATUS_OVERFLOW = 0x04   // 有符号除法溢出
} EAU_Status;

/* 全局变量声明部分 */


/* 函数声明部分 */

// EAU驱动库函数声明
void EAU_Init(void);
void EAU_SetMode(EAU_Mode mode);
void EAU_StartOperation(uint32_t dividend, uint32_t divisor);
uint32_t EAU_GetQuotient(void);
uint32_t EAU_GetRemainder(void);
EAU_Status EAU_GetStatus(void);


#ifdef __cplusplus
}
#endif

#endif
