/**
 * @file cw32l012_wwdt.h
 * @brief 此文件包含 CW32L012 窗口看门狗 (WWDT) 外设的标准外设库头文件声明。
 * @version 0.1
 * @date {%当前日期%}
 *
 * @copyright Copyright (c) {%当前年份%}
 */
#ifndef CW32L012_WWDT_H
#define CW32L012_WWDT_H

#include "cw32l012.h"
#include "cw32l012_sysctrl.h"

#define WWDT_ClearPOVFlag()            {CW_WWDT->SR = 0;}
/**
  * @brief  WWDT Prescaler divider
  */
typedef enum
{
    WWDT_PRESCALER_DIV4096  = 0x0,  /*!< PCLK/4096 */
    WWDT_PRESCALER_DIV8192  = 0x1,  /*!< PCLK/8192 */
    WWDT_PRESCALER_DIV16384 = 0x2,  /*!< PCLK/16384 */
    WWDT_PRESCALER_DIV32768 = 0x3,  /*!< PCLK/32768 */
    WWDT_PRESCALER_DIV65536 = 0x4,  /*!< PCLK/65536 */
    WWDT_PRESCALER_DIV131072 = 0x5, /*!< PCLK/131072 */
    WWDT_PRESCALER_DIV262144 = 0x6, /*!< PCLK/262144 */
    WWDT_PRESCALER_DIV524288 = 0x7  /*!< PCLK/524288 */
} WWDT_PrescalerTypeDef;

/**
  * @brief  WWDT Init structure definition
  */
typedef struct
{
    WWDT_PrescalerTypeDef Prescaler;  /*!< 预分频值 */
    uint8_t WindowValue;              /*!< 窗口值，范围0-127 */
    uint8_t CounterValue;             /*!< 计数器初始值，范围0-127 */
    FunctionalState PreOvInt;         /*!< 提前唤醒中断使能 */
} WWDT_InitTypeDef;

// 修改初始化函数声明
void WWDT_Init(const WWDT_InitTypeDef *WWDT_InitStruct);

// 喂狗操作
void WWDT_Feed(uint8_t ReloadCnt);

#endif
