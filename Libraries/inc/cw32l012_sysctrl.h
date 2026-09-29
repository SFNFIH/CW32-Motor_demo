/**
 * @file cw32l012_sysctrl.h
 * @author WHXY
 * @brief
 * @version 0.1
 * @date 2024-08-07
 *
 * @copyright Copyright (c) 2022
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CW32L012_SYSCTRL_H
#define __CW32L012_SYSCTRL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "system_cw32l012.h"
#include "cw32l012_gpio.h"

typedef struct {   
    uint32_t SourceFreq;
    uint32_t PLL_OutFreq;    
    uint16_t ClockSource;
    uint16_t WaitCycle;
}PLL_InitTypeDef;
//==================================================================================
#define HSIOSC_VALUE                               ((uint32_t)96000000)                 /*!< Value of the Internal oscillator in Hz*/
#define HSE_VALUE                                  ((uint32_t)16000000)                 /*!< Value of the External oscillator in Hz */

#define LSI_VALUE                                  ((uint32_t)32800)                    /*!< Value of the External oscillator in Hz */
#define LSE_VALUE                                  ((uint32_t)32768)                    /*!< Value of the External oscillator in Hz */

#define LSE_PIN_PORT                               CW_GPIOC
#define LSE_PIN_IN                                 GPIO_PIN_14
#define LSE_PIN_OUT                                GPIO_PIN_15

#define HSE_PIN_PORT                               CW_GPIOF
#define HSE_PIN_IN                                 GPIO_PIN_0
#define HSE_PIN_OUT                                GPIO_PIN_1

#define PCLK_OUT_PIN_PORT                          CW_GPIOA
#define PCLK_OUT_PIN                               GPIO_PIN_3
#define PCLK_OUT_PIN_AF                            4

#define MCO_PIN_PORT                               CW_GPIOA
#define MCO_PIN                                    GPIO_PIN_4
#define MCO_PIN_AF                                 4
/* SYSCTRL CR0 --------------------------------------------------------*/
#define SYSCTRL_KEY                                (0x5A5A0000U)
#define SYSCTRL_KEY_MASK                           (0xFFFF0000U)
#define SYSCTRL_KEY_MASK_INV                       (0x0000FFFFU)

#define SYSCTRL_HCLK_DIV1                          (0x0UL << SYSCTRL_CR0_HCLKPRS_Pos)   /*!< SYSCLK not divided */
#define SYSCTRL_HCLK_DIV2                          (0x1UL << SYSCTRL_CR0_HCLKPRS_Pos)   /*!< SYSCLK divided by 2 */
#define SYSCTRL_HCLK_DIV4                          (0x2UL << SYSCTRL_CR0_HCLKPRS_Pos)   /*!< SYSCLK divided by 4 */
#define SYSCTRL_HCLK_DIV8                          (0x3UL << SYSCTRL_CR0_HCLKPRS_Pos)   /*!< SYSCLK divided by 8 */
#define SYSCTRL_HCLK_DIV16                         (0x4UL << SYSCTRL_CR0_HCLKPRS_Pos)   /*!< SYSCLK divided by 16 */
#define SYSCTRL_HCLK_DIV32                         (0x5UL << SYSCTRL_CR0_HCLKPRS_Pos)   /*!< SYSCLK divided by 32 */
#define SYSCTRL_HCLK_DIV64                         (0x6UL << SYSCTRL_CR0_HCLKPRS_Pos)   /*!< SYSCLK divided by 64 */
#define SYSCTRL_HCLK_DIV128                        (0x7ul << SYSCTRL_CR0_HCLKPRS_Pos)   /*!< SYSCLK divided by 128 */
#define IS_SYSCTRL_HCLKPRS(INSTANCE)               (((INSTANCE) == SYSCTRL_HCLK_DIV1)              ||\
                                                   ((INSTANCE) == SYSCTRL_HCLK_DIV2)               ||\
                                                   ((INSTANCE) == SYSCTRL_HCLK_DIV4)               ||\
                                                   ((INSTANCE) == SYSCTRL_HCLK_DIV8)               ||\
                                                   ((INSTANCE) == SYSCTRL_HCLK_DIV16)              ||\
                                                   ((INSTANCE) == SYSCTRL_HCLK_DIV32)              ||\
                                                   ((INSTANCE) == SYSCTRL_HCLK_DIV64)              ||\
                                                   ((INSTANCE) == SYSCTRL_HCLK_DIV128))

#define SYSCTRL_PCLK_DIV1                          (0x0UL << SYSCTRL_CR0_PCLKPRS_Pos)   /*!< HCLK not divided */
#define SYSCTRL_PCLK_DIV2                          (0x1UL << SYSCTRL_CR0_PCLKPRS_Pos)   /*!< HCLK divided by 2 */
#define SYSCTRL_PCLK_DIV4                          (0x2UL << SYSCTRL_CR0_PCLKPRS_Pos)   /*!< HCLK divided by 4 */
#define SYSCTRL_PCLK_DIV8                          (0x3UL << SYSCTRL_CR0_PCLKPRS_Pos)   /*!< HCLK divided by 8 */
#define IS_SYSCTRL_PCLKPRS(INSTANCE)               (((INSTANCE) == SYSCTRL_PCLK_DIV1)              ||\
                                                   ((INSTANCE) == SYSCTRL_PCLK_DIV2)               ||\
                                                   ((INSTANCE) == SYSCTRL_PCLK_DIV4)               ||\
                                                   ((INSTANCE) == SYSCTRL_PCLK_DIV8))

#define SYSCTRL_SYSCLKSRC_HSI                      (0x00000000U)                        /*!< HSI selected as system clock */
#define SYSCTRL_SYSCLKSRC_HSE                      (0x00000001U)                        /*!< HSE selected as system clock */
#define SYSCTRL_SYSCLKSRC_PLL                      (0x00000002U)                        /*!< LSI selected as system clock */
#define SYSCTRL_SYSCLKSRC_LSI                      (0x00000003U)                        /*!< LSI selected as system clock */
#define SYSCTRL_SYSCLKSRC_LSE                      (0x00000004U)                        /*!< LSE selected as system clock */
 
#define IS_SYSCTRL_SYSCLKSRC(INSTANCE)             (((INSTANCE) == SYSCTRL_SYSCLKSRC_HSI)          ||\
                                                   ((INSTANCE) == SYSCTRL_SYSCLKSRC_HSE)           ||\
                                                   ((INSTANCE) == SYSCTRL_SYSCLKSRC_LSI)           ||\
                                                   ((INSTANCE) == SYSCTRL_SYSCLKSRC_LSE))

/* SYSCTRL CR1 --------------------------------------------------------*/
#define SYSCTRL_HSIEN                              bv0
#define SYSCTRL_HSEEN                              bv1
#define SYSCTRL_PLLEN                              bv2
#define SYSCTRL_LSIEN                              bv3
#define SYSCTRL_LSEEN                              bv4
#define SYSCTRL_LSELOCKEN                          bv5
#define SYSCTRL_LSELOCKDIS                         0x00UL
#define IS_SYSCTRL_LSELOCK_STATUS(INSTANCE)        (((INSTANCE) == SYSCTRL_LSELOCKEN)          ||\
                                                   ((INSTANCE) == SYSCTRL_LSELOCKDIS))

#define SYSCTRL_LSECCSEN                            bv6
#define SYSCTRL_HSECCSEN                            bv7
#define SYSCTRL_CLKCCSEN                            bv8
#define IS_SYSCTRL_CCS_STATUS(INSTANCE)            (((INSTANCE) == SYSCTRL_LSECCSEN)           ||\
                                                   ((INSTANCE) == SYSCTRL_HSECCSEN)            ||\
                                                   ((INSTANCE) == SYSCTRL_CLKCCSEN))

/* SYSCTRL CR2 --------------------------------------------------------*/
#define SYSCTRL_RAMBRKEN                        bv15
#define SYSCTRL_LVDBRKEN                        bv14
#define SYSCTRL_DSBRKEN                         bv13
#define SYSCTRL_LOCKUPBRKEN                     bv12
#define SYSCTRL_LSEBRKEN                        bv11
#define SYSCTRL_HSEBRKEN                        bv10
//#define SYSCTRL_RTC_LOWPOWEREN                  bv7
#define SYSCTRL_FLASHWAIT_24M                   (0x00UL)
#define SYSCTRL_FLASHWAIT_48M                   (0x01UL << SYSCTRL_CR2_FLASHWAIT_Pos)
#define SYSCTRL_FLASHWAIT_72M                   (0x02UL << SYSCTRL_CR2_FLASHWAIT_Pos)
#define SYSCTRL_FLASHWAIT_96M                   (0x03UL << SYSCTRL_CR2_FLASHWAIT_Pos)
#define IS_SYSCTRL_FLASHWAIT_VALUE(VAL)         (((VAL) == SYSCTRL_FLASHWAIT_24M) ||\
                                                 ((VAL) == SYSCTRL_FLASHWAIT_48M) ||\
                                                 ((VAL) == SYSCTRL_FLASHWAIT_72M) ||\
                                                 ((VAL) == SYSCTRL_FLASHWAIT_96M))

#define SYSCTRL_WAKEUPCLKEN                    bv3
#define SYSCTRL_WAKEUPCLKDIS                   0x00UL
#define IS_SYSCTRL_WAKEUPCLK_STATUS(INSTANCE)  (((INSTANCE) == SYSCTRL_WAKEUPCLKEN)          ||\
                                                ((INSTANCE) == SYSCTRL_WAKEUPCLKDIS))

#define SYSCTRL_LOCKUPEN                       bv2
#define SYSCTRL_LOCKUPDIS                      0x00UL
#define IS_SYSCTRL_LOCKUP_STATUS(INSTANCE)     (((INSTANCE) == SYSCTRL_LOCKUPEN)          ||\
                                               ((INSTANCE) == SYSCTRL_LOCKUPDIS))

#define SYSCTRL_SWDIOEN                        0x00UL
#define SYSCTRL_SWDIODIS                       bv1
#define IS_SYSCTRL_SWDIO_STATUS(INSTANCE)      (((INSTANCE) == SYSCTRL_SWDIOEN)          ||\
                                               ((INSTANCE) == SYSCTRL_SWDIODIS))
                                               
//#define SYSCTRL_RSTIOEN                        0x00UL
//#define SYSCTRL_RSTIODIS                       bv0

/* SYSCTRL HSI --------------------------------------------------------*/
#define SYSCTRL_HSI_TRIMCODEADDR                       (0x001007C0U)   //


#define SYSCTRL_HSIOSC_DIV1                            ((uint32_t)(1UL << SYSCTRL_HSI_DIV_Pos))
#define SYSCTRL_HSIOSC_DIV2                            ((uint32_t)(2UL << SYSCTRL_HSI_DIV_Pos))
#define SYSCTRL_HSIOSC_DIV3                            ((uint32_t)(3UL << SYSCTRL_HSI_DIV_Pos))
#define SYSCTRL_HSIOSC_DIV4                            ((uint32_t)(4UL << SYSCTRL_HSI_DIV_Pos))
#define SYSCTRL_HSIOSC_DIV5                            ((uint32_t)(5UL << SYSCTRL_HSI_DIV_Pos))
#define SYSCTRL_HSIOSC_DIV6                            ((uint32_t)(6UL << SYSCTRL_HSI_DIV_Pos))
#define SYSCTRL_HSIOSC_DIV7                            ((uint32_t)(7UL << SYSCTRL_HSI_DIV_Pos))
#define SYSCTRL_HSIOSC_DIV8                            ((uint32_t)(8UL << SYSCTRL_HSI_DIV_Pos))
#define SYSCTRL_HSIOSC_DIV9                            ((uint32_t)(9UL << SYSCTRL_HSI_DIV_Pos))
#define SYSCTRL_HSIOSC_DIV10                           ((uint32_t)(10UL << SYSCTRL_HSI_DIV_Pos))
#define SYSCTRL_HSIOSC_DIV12                           ((uint32_t)(11UL << SYSCTRL_HSI_DIV_Pos))
#define SYSCTRL_HSIOSC_DIV16                           ((uint32_t)(12UL << SYSCTRL_HSI_DIV_Pos))
#define SYSCTRL_HSIOSC_DIV20                           ((uint32_t)(13UL << SYSCTRL_HSI_DIV_Pos))
#define SYSCTRL_HSIOSC_DIV24                           ((uint32_t)(14UL << SYSCTRL_HSI_DIV_Pos))
#define SYSCTRL_HSIOSC_DIV28                           ((uint32_t)(15UL << SYSCTRL_HSI_DIV_Pos))
#define SYSCTRL_HSIOSC_DIV32                           ((uint32_t)(0UL))
#define HSIOSC_TO_HSI96MHZ                             SYSCTRL_HSIOSC_DIV1
#define HSIOSC_TO_HSI48MHZ                             SYSCTRL_HSIOSC_DIV2
#define HSIOSC_TO_HSI32MHZ                             SYSCTRL_HSIOSC_DIV3
#define HSIOSC_TO_HSI24MHZ                             SYSCTRL_HSIOSC_DIV4
#define HSIOSC_TO_HSI19P2MHZ                           SYSCTRL_HSIOSC_DIV5
#define HSIOSC_TO_HSI16MHZ                             SYSCTRL_HSIOSC_DIV6
#define HSIOSC_TO_HSI13P7MHZ                           SYSCTRL_HSIOSC_DIV7
#define HSIOSC_TO_HSI12MHZ                             SYSCTRL_HSIOSC_DIV8
#define HSIOSC_TO_HSI10P6MHZ                           SYSCTRL_HSIOSC_DIV9
#define HSIOSC_TO_HSI9P6MHZ                            SYSCTRL_HSIOSC_DIV10
#define HSIOSC_TO_HSI8MHZ                              SYSCTRL_HSIOSC_DIV12
#define HSIOSC_TO_HSI6MHZ                              SYSCTRL_HSIOSC_DIV16
#define HSIOSC_TO_HSI4P8MHZ                            SYSCTRL_HSIOSC_DIV20
#define HSIOSC_TO_HSI4MHZ                              SYSCTRL_HSIOSC_DIV24
#define HSIOSC_TO_HSI3P4MHZ                            SYSCTRL_HSIOSC_DIV28
#define HSIOSC_TO_HSI3MHZ                              SYSCTRL_HSIOSC_DIV32

#define IS_SYSCTRL_HSIOSC_DIV(INSTANCE)                (((INSTANCE) == SYSCTRL_HSIOSC_DIV1)         ||\
                                                        ((INSTANCE) == SYSCTRL_HSIOSC_DIV2)          ||\
                                                        ((INSTANCE) == SYSCTRL_HSIOSC_DIV3)          ||\
                                                        ((INSTANCE) == SYSCTRL_HSIOSC_DIV4)          ||\
                                                        ((INSTANCE) == SYSCTRL_HSIOSC_DIV5)          ||\
                                                        ((INSTANCE) == SYSCTRL_HSIOSC_DIV6)          ||\
                                                        ((INSTANCE) == SYSCTRL_HSIOSC_DIV7)          ||\
                                                        ((INSTANCE) == SYSCTRL_HSIOSC_DIV8)          ||\
                                                        ((INSTANCE) == SYSCTRL_HSIOSC_DIV9)          ||\
                                                        ((INSTANCE) == SYSCTRL_HSIOSC_DIV10)         ||\
                                                        ((INSTANCE) == SYSCTRL_HSIOSC_DIV12)          ||\
                                                        ((INSTANCE) == SYSCTRL_HSIOSC_DIV16)         ||\
                                                        ((INSTANCE) == SYSCTRL_HSIOSC_DIV20)          ||\
                                                        ((INSTANCE) == SYSCTRL_HSIOSC_DIV24)         ||\
                                                        ((INSTANCE) == SYSCTRL_HSIOSC_DIV28)          ||\
                                                        ((INSTANCE) == SYSCTRL_HSIOSC_DIV32))

/* SYSCTRL LSI --------------------------------------------------------*/
#define SYSCTRL_LSI_TRIMCODEADDR                       (0x001007C2U)     //

#define SYSCTRL_LSI_WAITCYCLE_6                        ((uint32_t)(0x0UL << SYSCTRL_LSI_WAITCYCLE_Pos))
#define SYSCTRL_LSI_WAITCYCLE_18                       ((uint32_t)(0x1UL << SYSCTRL_LSI_W13ITCYCLE_Pos))
#define SYSCTRL_LSI_WAITCYCLE_66                       ((uint32_t)(0x2UL << SYSCTRL_LSI_WAITCYCLE_Pos))
#define SYSCTRL_LSI_WAITCYCLE_258                      ((uint32_t)(0x3UL << SYSCTRL_LSI_WAITCYCLE_Pos))


/* SYSCTRL HSE --------------------------------------------------------*/
#define SYSCTRL_HSE_FLT_CLOSE                         (0U)
#define SYSCTRL_HSE_FLT_OPEN                          (1U << SYSCTRL_HSE_DIGFLT_Pos)

#define SYSCTRL_HSE_PDRIVER_LEVEL0                       ((uint32_t)(0x0UL << SYSCTRL_HSE_PDRIVER_Pos))
#define SYSCTRL_HSE_PDRIVER_LEVEL1                       ((uint32_t)(0x1UL << SYSCTRL_HSE_PDRIVER_Pos))
#define SYSCTRL_HSE_PDRIVER_LEVEL2                       ((uint32_t)(0x2UL << SYSCTRL_HSE_PDRIVER_Pos))
#define SYSCTRL_HSE_PDRIVER_LEVEL3                       ((uint32_t)(0x3UL << SYSCTRL_HSE_PDRIVER_Pos))
#define SYSCTRL_HSE_PDRIVER_LEVEL4                       ((uint32_t)(0x4UL << SYSCTRL_HSE_PDRIVER_Pos))
#define SYSCTRL_HSE_PDRIVER_LEVEL5                       ((uint32_t)(0x5UL << SYSCTRL_HSE_PDRIVER_Pos))
#define SYSCTRL_HSE_PDRIVER_LEVEL6                       ((uint32_t)(0x6UL << SYSCTRL_HSE_PDRIVER_Pos))
#define SYSCTRL_HSE_PDRIVER_LEVEL7                       ((uint32_t)(0x7UL << SYSCTRL_HSE_PDRIVER_Pos))
#define SYSCTRL_HSE_PDRIVER_LEVEL8                       ((uint32_t)(0x8UL << SYSCTRL_HSE_PDRIVER_Pos))
#define SYSCTRL_HSE_PDRIVER_LEVEL9                       ((uint32_t)(0x9UL << SYSCTRL_HSE_PDRIVER_Pos))
#define SYSCTRL_HSE_PDRIVER_LEVEL10                      ((uint32_t)(0xAUL << SYSCTRL_HSE_PDRIVER_Pos))
#define SYSCTRL_HSE_PDRIVER_LEVEL11                      ((uint32_t)(0xBUL << SYSCTRL_HSE_PDRIVER_Pos))
#define SYSCTRL_HSE_PDRIVER_LEVEL12                      ((uint32_t)(0xCUL << SYSCTRL_HSE_PDRIVER_Pos))
#define SYSCTRL_HSE_PDRIVER_LEVEL13                      ((uint32_t)(0xDUL << SYSCTRL_HSE_PDRIVER_Pos))
#define SYSCTRL_HSE_PDRIVER_LEVEL14                      ((uint32_t)(0xEUL << SYSCTRL_HSE_PDRIVER_Pos))
#define SYSCTRL_HSE_PDRIVER_LEVEL15                      ((uint32_t)(0xFUL << SYSCTRL_HSE_PDRIVER_Pos))


#define SYSCTRL_HSE_DETCNT_4M                            ((uint32_t)(2000UL << SYSCTRL_HSE_DETCNT_Pos))
#define SYSCTRL_HSE_DETCNT_8M                            ((uint32_t)(1000UL << SYSCTRL_HSE_DETCNT_Pos))
#define SYSCTRL_HSE_DETCNT_12M                           ((uint32_t)(666UL << SYSCTRL_HSE_DETCNT_Pos))
#define SYSCTRL_HSE_DETCNT_16M                           ((uint32_t)(500UL << SYSCTRL_HSE_DETCNT_Pos))
#define SYSCTRL_HSE_DETCNT_24M                           ((uint32_t)(333UL << SYSCTRL_HSE_DETCNT_Pos))
#define SYSCTRL_HSE_DETCNT_32M                           ((uint32_t)(250UL << SYSCTRL_HSE_DETCNT_Pos))
#define SYSCTRL_HSE_DETCNT_48M                           ((uint32_t)(167UL << SYSCTRL_HSE_DETCNT_Pos))
#define SYSCTRL_HSE_DETCNT_(X)M                          ((uint32_t)(8000/X) << SYSCTRL_HSE_DETCNT_Pos)

#define SYSCTRL_HEX_ENPOL_HIGH                           (0x00000000U)
#define SYSCTRL_HEX_ENPOL_LOW                            (bv7)

#define SYSCTRL_HSE_MODE_OSC                             (0x00000000U)
#define SYSCTRL_HSE_MODE_CLK                        bv6
#define IS_SYSCTRL_HSE_MODE(INSTANCE)               (((INSTANCE) == SYSCTRL_HSE_MODE_OSC) ||\
                                                     ((INSTANCE) == SYSCTRL_HSE_MODE_CLK))


#define SYSCTRL_HSE_WAITCYCLE_8192                       (0x00000000U)
#define SYSCTRL_HSE_WAITCYCLE_32768                      ((uint32_t)(0x1UL << SYSCTRL_HSE_WAITCYCLE_Pos))
#define SYSCTRL_HSE_WAITCYCLE_131072                     ((uint32_t)(0x2UL << SYSCTRL_HSE_WAITCYCLE_Pos))
#define SYSCTRL_HSE_WAITCYCLE_262144                     ((uint32_t)(0x3UL << SYSCTRL_HSE_WAITCYCLE_Pos))

#define SYSCTRL_HSE_DRIVER_LEVEL0                        ((uint32_t)(0x0UL << SYSCTRL_HSE_DRIVER_Pos))
#define SYSCTRL_HSE_DRIVER_LEVEL1                        ((uint32_t)(0x1UL << SYSCTRL_HSE_DRIVER_Pos))
#define SYSCTRL_HSE_DRIVER_LEVEL2                        ((uint32_t)(0x2UL << SYSCTRL_HSE_DRIVER_Pos))
#define SYSCTRL_HSE_DRIVER_LEVEL3                        ((uint32_t)(0x3UL << SYSCTRL_HSE_DRIVER_Pos))
#define SYSCTRL_HSE_DRIVER_LEVEL4                        ((uint32_t)(0x4UL << SYSCTRL_HSE_DRIVER_Pos))
#define SYSCTRL_HSE_DRIVER_LEVEL5                        ((uint32_t)(0x5UL << SYSCTRL_HSE_DRIVER_Pos))
#define SYSCTRL_HSE_DRIVER_LEVEL6                        ((uint32_t)(0x6UL << SYSCTRL_HSE_DRIVER_Pos))
#define SYSCTRL_HSE_DRIVER_LEVEL7                        ((uint32_t)(0x7UL << SYSCTRL_HSE_DRIVER_Pos))
#define SYSCTRL_HSE_DRIVER_LEVEL8                        ((uint32_t)(0x8UL << SYSCTRL_HSE_DRIVER_Pos))
#define SYSCTRL_HSE_DRIVER_LEVEL9                        ((uint32_t)(0x9UL << SYSCTRL_HSE_DRIVER_Pos))
#define SYSCTRL_HSE_DRIVER_LEVEL10                       ((uint32_t)(0xAUL << SYSCTRL_HSE_DRIVER_Pos))
#define SYSCTRL_HSE_DRIVER_LEVEL11                       ((uint32_t)(0xBUL << SYSCTRL_HSE_DRIVER_Pos))
#define SYSCTRL_HSE_DRIVER_LEVEL12                       ((uint32_t)(0xCUL << SYSCTRL_HSE_DRIVER_Pos))
#define SYSCTRL_HSE_DRIVER_LEVEL13                       ((uint32_t)(0xDUL << SYSCTRL_HSE_DRIVER_Pos))
#define SYSCTRL_HSE_DRIVER_LEVEL14                       ((uint32_t)(0xEUL << SYSCTRL_HSE_DRIVER_Pos))
#define SYSCTRL_HSE_DRIVER_LEVEL15                       ((uint32_t)(0xFUL << SYSCTRL_HSE_DRIVER_Pos))


/* SYSCTRL LSE --------------------------------------------------------*/
#define SYSCTRL_LSE_PINLOCKEN                            (bv17)
#define SYSCTRL_LSE_PINLOCKDIS                           (0UL)

//#define SYSCTRL_LSE_RESTRIM_35M                       0UL
//#define SYSCTRL_LSE_RESTRIM_25M                        (1UL  << SYSCTRL_LSE_RESTRIM_Pos)
//#define SYSCTRL_LSE_RESTRIM_20M                        (1UL  << SYSCTRL_LSE_RESTRIM_Pos)
//#define SYSCTRL_LSE_RESTRIM_10M                        (1UL  << SYSCTRL_LSE_RESTRIM_Pos)

//#define SYSCTRL_LSE_CAP_ENHANCE                      0UL
//#define SYSCTRL_LSE_CAP_NORMAL                         (1UL << SYSCTRL_LSE_CPEN_Pos)

//#define SYSCTRL_LSE_COMP_40nA                        0UL
//#define SYSCTRL_LSE_COMP_80nA                        bv12

#define SYSCTRL_LSE_PDRIVER_LEVEL0                       ((uint32_t)(0x0UL << SYSCTRL_LSE_PDRIVER_Pos))
#define SYSCTRL_LSE_PDRIVER_LEVEL1                       ((uint32_t)(0x1UL << SYSCTRL_LSE_PDRIVER_Pos))
#define SYSCTRL_LSE_PDRIVER_LEVEL2                       ((uint32_t)(0x2UL << SYSCTRL_LSE_PDRIVER_Pos))
#define SYSCTRL_LSE_PDRIVER_LEVEL3                       ((uint32_t)(0x3UL << SYSCTRL_LSE_PDRIVER_Pos))
#define SYSCTRL_LSE_PDRIVER_LEVEL4                       ((uint32_t)(0x4UL << SYSCTRL_LSE_PDRIVER_Pos))
#define SYSCTRL_LSE_PDRIVER_LEVEL5                       ((uint32_t)(0x5UL << SYSCTRL_LSE_PDRIVER_Pos))
#define SYSCTRL_LSE_PDRIVER_LEVEL6                       ((uint32_t)(0x6UL << SYSCTRL_LSE_PDRIVER_Pos))
#define SYSCTRL_LSE_PDRIVER_LEVEL7                       ((uint32_t)(0x7UL << SYSCTRL_LSE_PDRIVER_Pos))
#define SYSCTRL_LSE_PDRIVER_LEVEL8                       ((uint32_t)(0x8UL << SYSCTRL_LSE_PDRIVER_Pos))
#define SYSCTRL_LSE_PDRIVER_LEVEL9                       ((uint32_t)(0x9UL << SYSCTRL_LSE_PDRIVER_Pos))
#define SYSCTRL_LSE_PDRIVER_LEVEL10                      ((uint32_t)(0xAUL << SYSCTRL_LSE_PDRIVER_Pos))
#define SYSCTRL_LSE_PDRIVER_LEVEL11                      ((uint32_t)(0xBUL << SYSCTRL_LSE_PDRIVER_Pos))
#define SYSCTRL_LSE_PDRIVER_LEVEL12                      ((uint32_t)(0xCUL << SYSCTRL_LSE_PDRIVER_Pos))
#define SYSCTRL_LSE_PDRIVER_LEVEL13                      ((uint32_t)(0xDUL << SYSCTRL_LSE_PDRIVER_Pos))
#define SYSCTRL_LSE_PDRIVER_LEVEL14                      ((uint32_t)(0xEUL << SYSCTRL_LSE_PDRIVER_Pos))
#define SYSCTRL_LSE_PDRIVER_LEVEL15                      ((uint32_t)(0xFUL << SYSCTRL_LSE_PDRIVER_Pos))

//#define SYSCTRL_LSE_ANAFLT_DIS                      bv7

#define SYSCTRL_LSE_MODE_OSC                             (0x00000000U)
#define SYSCTRL_LSE_MODE_CLK                             (bv6)
#define IS_SYSCTRL_LSE_MODE(INSTANCE)                    (((INSTANCE) == SYSCTRL_LSE_MODE_OSC)         ||\
                                                         ((INSTANCE) == SYSCTRL_LSE_MODE_CLK))

#define SYSCTRL_LSE_WAITCYCLE_256                        (0x00000000U)
#define SYSCTRL_LSE_WAITCYCLE_1024                       ((uint32_t)(1UL << SYSCTRL_LSE_WAITCYCLE_Pos))
#define SYSCTRL_LSE_WAITCYCLE_4096                       ((uint32_t)(2UL << SYSCTRL_LSE_WAITCYCLE_Pos))
#define SYSCTRL_LSE_WAITCYCLE_16384                      ((uint32_t)(3UL << SYSCTRL_LSE_WAITCYCLE_Pos))


#define SYSCTRL_LSE_DRIVER_LEVEL0                        (0x00000000U)
#define SYSCTRL_LSE_DRIVER_LEVEL1                        (0x00000001U)
#define SYSCTRL_LSE_DRIVER_LEVEL2                        (0x00000002U)
#define SYSCTRL_LSE_DRIVER_LEVEL3                        (0x00000003U)
#define SYSCTRL_LSE_DRIVER_LEVEL4                        (0x00000004U)
#define SYSCTRL_LSE_DRIVER_LEVEL5                        (0x00000005U)
#define SYSCTRL_LSE_DRIVER_LEVEL6                        (0x00000006U)
#define SYSCTRL_LSE_DRIVER_LEVEL7                        (0x00000007U)
#define SYSCTRL_LSE_DRIVER_LEVEL8                        (0x00000008U)
#define SYSCTRL_LSE_DRIVER_LEVEL9                        (0x00000009U)
#define SYSCTRL_LSE_DRIVER_LEVEL10                       (0x0000000aU)
#define SYSCTRL_LSE_DRIVER_LEVEL11                       (0x0000000bU)
#define SYSCTRL_LSE_DRIVER_LEVEL12                       (0x0000000cU)
#define SYSCTRL_LSE_DRIVER_LEVEL13                       (0x0000000dU)
#define SYSCTRL_LSE_DRIVER_LEVEL14                       (0x0000000eU)
#define SYSCTRL_LSE_DRIVER_LEVEL15                       (0x0000000fU)

#define IS_SYSCTRL_LSE_DRIVER(DRIVER)                  ((DRIVER) < 0x10)

/* SYSCTRL PLL  --------------------------------------------------------*/
#define SYSCTRL_PLL_WAIT_CYCLE_128                      (0U << SYSCTRL_PLL_WAITCYCLE_Pos)
#define SYSCTRL_PLL_WAIT_CYCLE_256                      (1U << SYSCTRL_PLL_WAITCYCLE_Pos)
#define SYSCTRL_PLL_WAIT_CYCLE_512                      (2U << SYSCTRL_PLL_WAITCYCLE_Pos)
#define SYSCTRL_PLL_WAIT_CYCLE_1024                     (3U << SYSCTRL_PLL_WAITCYCLE_Pos)
#define SYSCTRL_PLL_WAIT_CYCLE_2048                     (4U << SYSCTRL_PLL_WAITCYCLE_Pos)
#define SYSCTRL_PLL_WAIT_CYCLE_4096                     (5U << SYSCTRL_PLL_WAITCYCLE_Pos)
#define SYSCTRL_PLL_WAIT_CYCLE_8192                     (6U << SYSCTRL_PLL_WAITCYCLE_Pos)
#define SYSCTRL_PLL_WAIT_CYCLE_16384                    (7U << SYSCTRL_PLL_WAITCYCLE_Pos)

#define SYSCTRL_PLL_DIV1                                 (0U << SYSCTRL_PLL_DIV_Pos)
#define SYSCTRL_PLL_DIV2                                 (1U << SYSCTRL_PLL_DIV_Pos)
#define SYSCTRL_PLL_DIV3                                 (2U << SYSCTRL_PLL_DIV_Pos)
#define SYSCTRL_PLL_DIV4                                 (3U << SYSCTRL_PLL_DIV_Pos)
#define SYSCTRL_PLL_DIV5                                 (4U << SYSCTRL_PLL_DIV_Pos)
#define SYSCTRL_PLL_DIV6                                 (5U << SYSCTRL_PLL_DIV_Pos)
#define SYSCTRL_PLL_DIV7                                 (6U << SYSCTRL_PLL_DIV_Pos)
#define SYSCTRL_PLL_DIV8                                 (7U << SYSCTRL_PLL_DIV_Pos)
#define SYSCTRL_PLL_DIV9                                 (8U << SYSCTRL_PLL_DIV_Pos)
#define SYSCTRL_PLL_DIV10                                (9U << SYSCTRL_PLL_DIV_Pos)
#define SYSCTRL_PLL_DIV11                                (10U << SYSCTRL_PLL_DIV_Pos)
#define SYSCTRL_PLL_DIV12                                (11U << SYSCTRL_PLL_DIV_Pos)
#define SYSCTRL_PLL_DIV13                                (12U << SYSCTRL_PLL_DIV_Pos)
#define SYSCTRL_PLL_DIV14                                (13U << SYSCTRL_PLL_DIV_Pos)
#define SYSCTRL_PLL_DIV15                                (14U << SYSCTRL_PLL_DIV_Pos)
#define SYSCTRL_PLL_DIV16                                (15U << SYSCTRL_PLL_DIV_Pos)

#define SYSCTRL_PLL_SOURCE_HSI_48M                       (0U << SYSCTRL_PLL_SOURCE_Pos)
#define SYSCTRL_PLL_SOURCE_HSE                           (1U << SYSCTRL_PLL_SOURCE_Pos)

/* SYSCTRL ISR  --------------------------------------------------------*/
#define SYSCTRL_FLAG_LSESTABLE                         bv15
#define SYSCTRL_FLAG_LSISTABLE                         bv14
#define SYSCTRL_FLAG_PLLSTABLE                         bv13
#define SYSCTRL_FLAG_HSESTABLE                         bv12
#define SYSCTRL_FLAG_HSISTABLE                         bv11
#define IS_SYSCTRL_STABLEFLAG(FLAG)                    ((FLAG) == SYSCTRL_FLAG_HSISTABLE || \
                                                        (FLAG) == SYSCTRL_FLAG_HSESTABLE || \
                                                        (FLAG) == SYSCTRL_FLAG_LSESTABLE || \
                                                        (FLAG) == SYSCTRL_FLAG_LSISTABLE || \
                                                        (FLAG) == SYSCTRL_FLAG_PLLSTABLE)

#define SYSCTRL_FLAG_HSEFAULT                          bv8
#define SYSCTRL_FLAG_LSEFAULT                          bv7
#define SYSCTRL_FLAG_HSEFAIL                           bv6
#define SYSCTRL_FLAG_LSEFAIL                           bv5
#define SYSCTRL_FLAG_LSERDY                            bv4
#define SYSCTRL_FLAG_LSIRDY                            bv3
#define SYSCTRL_FLAG_PLLRDY                            bv2
#define SYSCTRL_FLAG_HSERDY                            bv1
#define SYSCTRL_FLAG_HSIRDY                            bv0

/* SYSCTRL IER ICR  --------------------------------------------------------*/
#define SYSCTRL_IT_HSEFAULT                            (bv8)
#define SYSCTRL_IT_LSEFAULT                            (bv7)
#define SYSCTRL_IT_HSEFAIL                             (bv6)
#define SYSCTRL_IT_LSEFAIL                             (bv5)
#define SYSCTRL_IT_LSERDY                              (bv4)
#define SYSCTRL_IT_LSIRDY                              (bv3)
#define SYSCTRL_IT_PLLRDY                              (bv2)
#define SYSCTRL_IT_HSERDY                              (bv1)
#define SYSCTRL_IT_HSIRDY                              (bv0)
#define IS_SYSCTRL_IT(IT)                              ((((IT) & 0xFFFFFFE0) == 0x0UL) && ((IT) != 0x0UL))
#define IS_SYSCTRL_GET_IT(IT)                          (((IT) == SYSCTRL_IT_HSEFAULT) || \
                                                        ((IT) == SYSCTRL_IT_LSEFAULT) || \
                                                        ((IT) == SYSCTRL_IT_HSEFAIL) || \
                                                        ((IT) == SYSCTRL_IT_LSEFAIL) || \
                                                        ((IT) == SYSCTRL_IT_LSERDY) ||\
                                                        ((IT) == SYSCTRL_IT_LSIRDY) ||\
                                                        ((IT) == SYSCTRL_IT_PLLRDY) ||\
                                                        ((IT) == SYSCTRL_IT_HSERDY) ||\
                                                        ((IT) == SYSCTRL_IT_HSIRDY))

/* SYSCTRL RESETFLAG  --------------------------------------------------------*/
#define SYSCTRL_RESETFLAG_POR                            bv0
#define SYSCTRL_RESETFLAG_LVD                            bv3
#define SYSCTRL_RESETFLAG_IWDT                           bv4
#define SYSCTRL_RESETFLAG_WWDT                           bv5
#define SYSCTRL_RESETFLAG_PIN                            bv6
#define SYSCTRL_RESETFLAG_LOCKUP                         bv8
#define SYSCTRL_RESETFLAG_SYSRESETREQ                    bv9
#define SYSCTRL_RESETFLAG_ALL                            (bv9|bv8|bv6|bv5|bv4|bv3|bv0)
#define IS_SYSCTRL_RSTFLAG(FLAG)                         ((FLAG) == SYSCTRL_RESETFLAG_POR || \
                                                          (FLAG) == SYSCTRL_RESETFLAG_LVD || \
                                                          (FLAG) == SYSCTRL_RESETFLAG_IWDT || \
                                                          (FLAG) == SYSCTRL_RESETFLAG_WWDT || \
                                                          (FLAG) == SYSCTRL_RESETFLAG_PIN || \
                                                          (FLAG) == SYSCTRL_RESETFLAG_LOCKUP || \
                                                          (FLAG) == SYSCTRL_RESETFLAG_SYSRESETREQ || \
                                                          (FLAG) == SYSCTRL_RESETFLAG_ALL)

/* SYSCTRL AHB  --------------------------------------------------------*/
#define SYSCTRL_AHB_PERIPH_CORDIC                      bv8
#define SYSCTRL_AHB_PERIPH_GPIOF                       bv7
#define SYSCTRL_AHB_PERIPH_GPIOC                       bv6
#define SYSCTRL_AHB_PERIPH_GPIOB                       bv5
#define SYSCTRL_AHB_PERIPH_GPIOA                       bv4
#define SYSCTRL_AHB_PERIPH_EAU                         bv3
#define SYSCTRL_AHB_PERIPH_CRC                         bv2
#define SYSCTRL_AHB_PERIPH_FLASH                       bv1
#define SYSCTRL_AHB_PERIPH_DMA                         bv0
#define IS_SYSCTRL_AHB_PERIPH(PERIPH)                  ((((PERIPH) & 0xFFFFFE00 ) == 0x00) && ((PERIPH) != 0x00))


#define __SYSCTRL_CORDIC_CLK_ENABLE()                  REGBITS_SET(CW_SYSCTRL->AHBEN, (SYSCTRL_KEY | SYSCTRL_AHB_PERIPH_CORDIC))
#define __SYSCTRL_GPIOF_CLK_ENABLE()                   REGBITS_SET(CW_SYSCTRL->AHBEN, (SYSCTRL_KEY | SYSCTRL_AHB_PERIPH_GPIOF))
#define __SYSCTRL_GPIOC_CLK_ENABLE()                   REGBITS_SET(CW_SYSCTRL->AHBEN, (SYSCTRL_KEY | SYSCTRL_AHB_PERIPH_GPIOC))
#define __SYSCTRL_GPIOB_CLK_ENABLE()                   REGBITS_SET(CW_SYSCTRL->AHBEN, (SYSCTRL_KEY | SYSCTRL_AHB_PERIPH_GPIOB))
#define __SYSCTRL_GPIOA_CLK_ENABLE()                   REGBITS_SET(CW_SYSCTRL->AHBEN, (SYSCTRL_KEY | SYSCTRL_AHB_PERIPH_GPIOA))
#define __SYSCTRL_EAU_CLK_ENABLE()                     REGBITS_SET(CW_SYSCTRL->AHBEN, (SYSCTRL_KEY | SYSCTRL_AHB_PERIPH_EAU))
#define __SYSCTRL_CRC_CLK_ENABLE()                     REGBITS_SET(CW_SYSCTRL->AHBEN, (SYSCTRL_KEY | SYSCTRL_AHB_PERIPH_CRC))
#define __SYSCTRL_FLASH_CLK_ENABLE()                   REGBITS_SET(CW_SYSCTRL->AHBEN, (SYSCTRL_KEY | SYSCTRL_AHB_PERIPH_FLASH))
#define __SYSCTRL_DMA_CLK_ENABLE()                     REGBITS_SET(CW_SYSCTRL->AHBEN, (SYSCTRL_KEY | SYSCTRL_AHB_PERIPH_DMA))

#define __SYSCTRL_CORDIC_CLK_DISABLE()                 (CW_SYSCTRL->AHBEN = (SYSCTRL_KEY | (CW_SYSCTRL->AHBEN & (~SYSCTRL_AHB_PERIPH_CORDIC))))
#define __SYSCTRL_GPIOF_CLK_DISABLE()                  (CW_SYSCTRL->AHBEN = (SYSCTRL_KEY | (CW_SYSCTRL->AHBEN & (~SYSCTRL_AHB_PERIPH_GPIOF))))
#define __SYSCTRL_GPIOC_CLK_DISABLE()                  (CW_SYSCTRL->AHBEN = (SYSCTRL_KEY | (CW_SYSCTRL->AHBEN & (~SYSCTRL_AHB_PERIPH_GPIOC))))
#define __SYSCTRL_GPIOB_CLK_DISABLE()                  (CW_SYSCTRL->AHBEN = (SYSCTRL_KEY | (CW_SYSCTRL->AHBEN & (~SYSCTRL_AHB_PERIPH_GPIOB))))
#define __SYSCTRL_GPIOA_CLK_DISABLE()                  (CW_SYSCTRL->AHBEN = (SYSCTRL_KEY | (CW_SYSCTRL->AHBEN & (~SYSCTRL_AHB_PERIPH_GPIOA))))
#define __SYSCTRL_EAU_CLK_DISABLE()                    (CW_SYSCTRL->AHBEN = (SYSCTRL_KEY | (CW_SYSCTRL->AHBEN & (~SYSCTRL_AHB_PERIPH_EAU))))
#define __SYSCTRL_CRC_CLK_DISABLE()                    (CW_SYSCTRL->AHBEN = (SYSCTRL_KEY | (CW_SYSCTRL->AHBEN & (~SYSCTRL_AHB_PERIPH_CRC))))
#define __SYSCTRL_FLASH_CLK_DISABLE()                  (CW_SYSCTRL->AHBEN = (SYSCTRL_KEY | (CW_SYSCTRL->AHBEN & (~SYSCTRL_AHB_PERIPH_FLASH))))
#define __SYSCTRL_DMA_CLK_DISABLE()                    (CW_SYSCTRL->AHBEN = (SYSCTRL_KEY | (CW_SYSCTRL->AHBEN & (~SYSCTRL_AHB_PERIPH_DMA))))

/* SYSCTRL APB1  --------------------------------------------------------*/
#define SYSCTRL_APB1_PERIPH_SPI3                       bv14
#define SYSCTRL_APB1_PERIPH_SPI2                       bv13
#define SYSCTRL_APB1_PERIPH_GTIM4                      bv12
#define SYSCTRL_APB1_PERIPH_GTIM3                      bv11
#define SYSCTRL_APB1_PERIPH_UART3                      bv8
#define SYSCTRL_APB1_PERIPH_GTIM2                      bv7
#define SYSCTRL_APB1_PERIPH_GTIM1                      bv6
#define SYSCTRL_APB1_PERIPH_ATIM                       bv5
#define SYSCTRL_APB1_PERIPH_UART2                      bv4
#define SYSCTRL_APB1_PERIPH_UART1                      bv3
#define SYSCTRL_APB1_PERIPH_SPI1                       bv2
#define SYSCTRL_APB1_PERIPH_VC                         bv1
#define SYSCTRL_APB1_PERIPH_ADC                        bv0
#define IS_SYSCTRL_APB1_PERIPH(PERIPH)                 ((((PERIPH) & 0xFFFF8600) == 0x00) && ((PERIPH) != 0x00))

#define __SYSCTRL_SPI3_CLK_ENABLE()                    REGBITS_SET(CW_SYSCTRL->APBEN1, (SYSCTRL_KEY | SYSCTRL_APB1_PERIPH_SPI3))
#define __SYSCTRL_SPI2_CLK_ENABLE()                    REGBITS_SET(CW_SYSCTRL->APBEN1, (SYSCTRL_KEY | SYSCTRL_APB1_PERIPH_SPI2))
#define __SYSCTRL_GTIM4_CLK_ENABLE()                   REGBITS_SET(CW_SYSCTRL->APBEN1, (SYSCTRL_KEY | SYSCTRL_APB1_PERIPH_GTIM4))
#define __SYSCTRL_GTIM3_CLK_ENABLE()                   REGBITS_SET(CW_SYSCTRL->APBEN1, (SYSCTRL_KEY | SYSCTRL_APB1_PERIPH_GTIM3))
#define __SYSCTRL_UART3_CLK_ENABLE()                   REGBITS_SET(CW_SYSCTRL->APBEN1, (SYSCTRL_KEY | SYSCTRL_APB1_PERIPH_UART3))
#define __SYSCTRL_GTIM2_CLK_ENABLE()                   REGBITS_SET(CW_SYSCTRL->APBEN1, (SYSCTRL_KEY | SYSCTRL_APB1_PERIPH_GTIM2))
#define __SYSCTRL_GTIM1_CLK_ENABLE()                   REGBITS_SET(CW_SYSCTRL->APBEN1, (SYSCTRL_KEY | SYSCTRL_APB1_PERIPH_GTIM1))
#define __SYSCTRL_ATIM_CLK_ENABLE()                    REGBITS_SET(CW_SYSCTRL->APBEN1, (SYSCTRL_KEY | SYSCTRL_APB1_PERIPH_ATIM))
#define __SYSCTRL_UART2_CLK_ENABLE()                   REGBITS_SET(CW_SYSCTRL->APBEN1, (SYSCTRL_KEY | SYSCTRL_APB1_PERIPH_UART2))
#define __SYSCTRL_UART1_CLK_ENABLE()                   REGBITS_SET(CW_SYSCTRL->APBEN1, (SYSCTRL_KEY | SYSCTRL_APB1_PERIPH_UART1))
#define __SYSCTRL_SPI1_CLK_ENABLE()                    REGBITS_SET(CW_SYSCTRL->APBEN1, (SYSCTRL_KEY | SYSCTRL_APB1_PERIPH_SPI1))
#define __SYSCTRL_VC_CLK_ENABLE()                      REGBITS_SET(CW_SYSCTRL->APBEN1, (SYSCTRL_KEY | SYSCTRL_APB1_PERIPH_VC))
#define __SYSCTRL_ADC_CLK_ENABLE()                     REGBITS_SET(CW_SYSCTRL->APBEN1, (SYSCTRL_KEY | SYSCTRL_APB1_PERIPH_ADC)) 

#define __SYSCTRL_SPI3_CLK_DISABLE()                   (CW_SYSCTRL->APBEN1 = (SYSCTRL_KEY | (CW_SYSCTRL->APBEN1 & (~SYSCTRL_APB1_PERIPH_SPI3))))
#define __SYSCTRL_SPI2_CLK_DISABLE()                   (CW_SYSCTRL->APBEN1 = (SYSCTRL_KEY | (CW_SYSCTRL->APBEN1 & (~SYSCTRL_APB1_PERIPH_SPI2))))
#define __SYSCTRL_GTIM4_CLK_DISABLE()                  (CW_SYSCTRL->APBEN1 = (SYSCTRL_KEY | (CW_SYSCTRL->APBEN1 & (~SYSCTRL_APB1_PERIPH_GTIM4))))
#define __SYSCTRL_GTIM3_CLK_DISABLE()                  (CW_SYSCTRL->APBEN1 = (SYSCTRL_KEY | (CW_SYSCTRL->APBEN1 & (~SYSCTRL_APB1_PERIPH_GTIM3))))
#define __SYSCTRL_UART3_CLK_DISABLE()                  (CW_SYSCTRL->APBEN1 = (SYSCTRL_KEY | (CW_SYSCTRL->APBEN1 & (~SYSCTRL_APB1_PERIPH_UART3))))
#define __SYSCTRL_GTIM2_CLK_DISABLE()                  (CW_SYSCTRL->APBEN1 = (SYSCTRL_KEY | (CW_SYSCTRL->APBEN1 & (~SYSCTRL_APB1_PERIPH_GTIM2))))
#define __SYSCTRL_GTIM1_CLK_DISABLE()                  (CW_SYSCTRL->APBEN1 = (SYSCTRL_KEY | (CW_SYSCTRL->APBEN1 & (~SYSCTRL_APB1_PERIPH_GTIM1))))
#define __SYSCTRL_ATIM_CLK_DISABLE()                   (CW_SYSCTRL->APBEN1 = (SYSCTRL_KEY | (CW_SYSCTRL->APBEN1 & (~SYSCTRL_APB1_PERIPH_ATIM))))
#define __SYSCTRL_UART2_CLK_DISABLE()                  (CW_SYSCTRL->APBEN1 = (SYSCTRL_KEY | (CW_SYSCTRL->APBEN1 & (~SYSCTRL_APB1_PERIPH_UART2))))
#define __SYSCTRL_UART1_CLK_DISABLE()                  (CW_SYSCTRL->APBEN1 = (SYSCTRL_KEY | (CW_SYSCTRL->APBEN1 & (~SYSCTRL_APB1_PERIPH_UART1))))
#define __SYSCTRL_SPI1_CLK_DISABLE()                   (CW_SYSCTRL->APBEN1 = (SYSCTRL_KEY | (CW_SYSCTRL->APBEN1 & (~SYSCTRL_APB1_PERIPH_SPI1))))
#define __SYSCTRL_VC_CLK_DISABLE()                     (CW_SYSCTRL->APBEN1 = (SYSCTRL_KEY | (CW_SYSCTRL->APBEN1 & (~SYSCTRL_APB1_PERIPH_VC))))
#define __SYSCTRL_ADC_CLK_DISABLE()                    (CW_SYSCTRL->APBEN1 = (SYSCTRL_KEY | (CW_SYSCTRL->APBEN1 & (~SYSCTRL_APB1_PERIPH_ADC))))


/* SYSCTRL APB2  --------------------------------------------------------*/
#define SYSCTRL_APB2_PERIPH_HALLTIM                    bv12
#define SYSCTRL_APB2_PERIPH_I2C2                       bv11
#define SYSCTRL_APB2_PERIPH_DAC                        bv10
#define SYSCTRL_APB2_PERIPH_OPA                        bv9
#define SYSCTRL_APB2_PERIPH_LPTIM                      bv7
#define SYSCTRL_APB2_PERIPH_I2C1                       bv6
#define SYSCTRL_APB2_PERIPH_WWDT                       bv5
#define SYSCTRL_APB2_PERIPH_IWDT                       bv4
#define SYSCTRL_APB2_PERIPH_BTIM123                    bv2
#define SYSCTRL_APB2_PERIPH_RTC                        bv1
#define IS_SYSCTRL_APB2_PERIPH(PERIPH)                 ((((PERIPH) & 0xFFFFC109) == 0x00) && ((PERIPH) != 0x00))

#define __SYSCTRL_HALLTIM_CLK_ENABLE()                   REGBITS_SET(CW_SYSCTRL->APBEN2, (SYSCTRL_KEY | SYSCTRL_APB2_PERIPH_HALLTIM))
#define __SYSCTRL_I2C2_CLK_ENABLE()                      REGBITS_SET(CW_SYSCTRL->APBEN2, (SYSCTRL_KEY | SYSCTRL_APB2_PERIPH_I2C2))
#define __SYSCTRL_DAC_CLK_ENABLE()                       REGBITS_SET(CW_SYSCTRL->APBEN2, (SYSCTRL_KEY | SYSCTRL_APB2_PERIPH_DAC))
#define __SYSCTRL_OPA_CLK_ENABLE()                       REGBITS_SET(CW_SYSCTRL->APBEN2, (SYSCTRL_KEY | SYSCTRL_APB2_PERIPH_OPA))
#define __SYSCTRL_LPTIM_CLK_ENABLE()                     REGBITS_SET(CW_SYSCTRL->APBEN2, (SYSCTRL_KEY | SYSCTRL_APB2_PERIPH_LPTIM))
#define __SYSCTRL_I2C1_CLK_ENABLE()                      REGBITS_SET(CW_SYSCTRL->APBEN2, (SYSCTRL_KEY | SYSCTRL_APB2_PERIPH_I2C1))
#define __SYSCTRL_WWDT_CLK_ENABLE()                      REGBITS_SET(CW_SYSCTRL->APBEN2, (SYSCTRL_KEY | SYSCTRL_APB2_PERIPH_WWDT))
#define __SYSCTRL_IWDT_CLK_ENABLE()                      REGBITS_SET(CW_SYSCTRL->APBEN2, (SYSCTRL_KEY | SYSCTRL_APB2_PERIPH_IWDT))
#define __SYSCTRL_BTIM123_CLK_ENABLE()                   REGBITS_SET(CW_SYSCTRL->APBEN2, (SYSCTRL_KEY | SYSCTRL_APB2_PERIPH_BTIM123))
#define __SYSCTRL_RTC_CLK_ENABLE()                       REGBITS_SET(CW_SYSCTRL->APBEN2, (SYSCTRL_KEY | SYSCTRL_APB2_PERIPH_RTC))

#define __SYSCTRL_HALLTIM_CLK_DISABLE()                (CW_SYSCTRL->APBEN2 = (SYSCTRL_KEY | (CW_SYSCTRL->APBEN2 & (~SYSCTRL_APB2_PERIPH_HALLTIM))))
#define __SYSCTRL_I2C2_CLK_DISABLE()                   (CW_SYSCTRL->APBEN2 = (SYSCTRL_KEY | (CW_SYSCTRL->APBEN2 & (~SYSCTRL_APB2_PERIPH_I2C2))))
#define __SYSCTRL_DAC_CLK_DISABLE()                    (CW_SYSCTRL->APBEN2 = (SYSCTRL_KEY | (CW_SYSCTRL->APBEN2 & (~SYSCTRL_APB2_PERIPH_DAC))))
#define __SYSCTRL_OPA_CLK_DISABLE()                    (CW_SYSCTRL->APBEN2 = (SYSCTRL_KEY | (CW_SYSCTRL->APBEN2 & (~SYSCTRL_APB2_PERIPH_OPA))))
#define __SYSCTRL_LPTIM_CLK_DISABLE()                  (CW_SYSCTRL->APBEN2 = (SYSCTRL_KEY | (CW_SYSCTRL->APBEN2 & (~SYSCTRL_APB2_PERIPH_LPTIM))))
#define __SYSCTRL_I2C1_CLK_DISABLE()                   (CW_SYSCTRL->APBEN2 = (SYSCTRL_KEY | (CW_SYSCTRL->APBEN2 & (~SYSCTRL_APB2_PERIPH_I2C1))))
#define __SYSCTRL_WWDT_CLK_DISABLE()                   (CW_SYSCTRL->APBEN2 = (SYSCTRL_KEY | (CW_SYSCTRL->APBEN2 & (~SYSCTRL_APB2_PERIPH_WWDT))))
#define __SYSCTRL_IWDT_CLK_DISABLE()                   (CW_SYSCTRL->APBEN2 = (SYSCTRL_KEY | (CW_SYSCTRL->APBEN2 & (~SYSCTRL_APB2_PERIPH_IWDT))))
#define __SYSCTRL_BTIM123_CLK_DISABLE()                (CW_SYSCTRL->APBEN2 = (SYSCTRL_KEY | (CW_SYSCTRL->APBEN2 & (~SYSCTRL_APB2_PERIPH_BTIM123))))
#define __SYSCTRL_RTC_CLK_DISABLE()                    (CW_SYSCTRL->APBEN2 = (SYSCTRL_KEY | (CW_SYSCTRL->APBEN2 & (~SYSCTRL_APB2_PERIPH_RTC))))

/* SYSCTRL AHB RST --------------------------------------------------------*/
#define __SYSCTRL_CORDIC_RST_ENABLE()                  REGBITS_CLR(CW_SYSCTRL->AHBRST, (SYSCTRL_AHB_PERIPH_CORDIC))
#define __SYSCTRL_GPIOF_RST_ENABLE()                   REGBITS_CLR(CW_SYSCTRL->AHBRST, (SYSCTRL_AHB_PERIPH_GPIOF))
#define __SYSCTRL_GPIOC_RST_ENABLE()                   REGBITS_CLR(CW_SYSCTRL->AHBRST, (SYSCTRL_AHB_PERIPH_GPIOC))
#define __SYSCTRL_GPIOB_RST_ENABLE()                   REGBITS_CLR(CW_SYSCTRL->AHBRST, (SYSCTRL_AHB_PERIPH_GPIOB))
#define __SYSCTRL_GPIOA_RST_ENABLE()                   REGBITS_CLR(CW_SYSCTRL->AHBRST, (SYSCTRL_AHB_PERIPH_GPIOA))
#define __SYSCTRL_EAU_RST_ENABLE()                     REGBITS_CLR(CW_SYSCTRL->AHBRST, (SYSCTRL_AHB_PERIPH_EAU))
#define __SYSCTRL_CRC_RST_ENABLE()                     REGBITS_CLR(CW_SYSCTRL->AHBRST, (SYSCTRL_AHB_PERIPH_CRC))
#define __SYSCTRL_FLASH_RST_ENABLE()                   REGBITS_CLR(CW_SYSCTRL->AHBRST, (SYSCTRL_AHB_PERIPH_FLASH))
#define __SYSCTRL_DMA_RST_ENABLE()                     REGBITS_CLR(CW_SYSCTRL->AHBRST, (SYSCTRL_AHB_PERIPH_DMA))

#define __SYSCTRL_CORDIC_RST_DISABLE()                  REGBITS_SET(CW_SYSCTRL->AHBRST, (SYSCTRL_AHB_PERIPH_CORDIC))
#define __SYSCTRL_GPIOF_RST_DISABLE()                   REGBITS_SET(CW_SYSCTRL->AHBRST, (SYSCTRL_AHB_PERIPH_GPIOF))
#define __SYSCTRL_GPIOC_RST_DISABLE()                   REGBITS_SET(CW_SYSCTRL->AHBRST, (SYSCTRL_AHB_PERIPH_GPIOC))
#define __SYSCTRL_GPIOB_RST_DISABLE()                   REGBITS_SET(CW_SYSCTRL->AHBRST, (SYSCTRL_AHB_PERIPH_GPIOB))
#define __SYSCTRL_GPIOA_RST_DISABLE()                   REGBITS_SET(CW_SYSCTRL->AHBRST, (SYSCTRL_AHB_PERIPH_GPIOA))
#define __SYSCTRL_EAU_RST_DISABLE()                     REGBITS_SET(CW_SYSCTRL->AHBRST, (SYSCTRL_AHB_PERIPH_EAU))
#define __SYSCTRL_CRC_RST_DISABLE()                     REGBITS_SET(CW_SYSCTRL->AHBRST, (SYSCTRL_AHB_PERIPH_CRC))
#define __SYSCTRL_FLASH_RST_DISABLE()                   REGBITS_SET(CW_SYSCTRL->AHBRST, (SYSCTRL_AHB_PERIPH_FLASH))
#define __SYSCTRL_DMA_RST_DISABLE()                     REGBITS_SET(CW_SYSCTRL->AHBRST, (SYSCTRL_AHB_PERIPH_DMA))

/* SYSCTRL APB1 RST --------------------------------------------------------*/
#define __SYSCTRL_SPI3_RST_ENABLE()                    REGBITS_CLR(CW_SYSCTRL->APBRST1, (SYSCTRL_APB1_PERIPH_SPI3))
#define __SYSCTRL_SPI2_RST_ENABLE()                    REGBITS_CLR(CW_SYSCTRL->APBRST1, (SYSCTRL_APB1_PERIPH_SPI2))
#define __SYSCTRL_GTIM4_RST_ENABLE()                   REGBITS_CLR(CW_SYSCTRL->APBRST1, (SYSCTRL_APB1_PERIPH_GTIM4))
#define __SYSCTRL_GTIM3_RST_ENABLE()                   REGBITS_CLR(CW_SYSCTRL->APBRST1, (SYSCTRL_APB1_PERIPH_GTIM3))
#define __SYSCTRL_UART3_RST_ENABLE()                   REGBITS_CLR(CW_SYSCTRL->APBRST1, (SYSCTRL_APB1_PERIPH_UART3))
#define __SYSCTRL_GTIM2_RST_ENABLE()                   REGBITS_CLR(CW_SYSCTRL->APBRST1, (SYSCTRL_APB1_PERIPH_GTIM2))
#define __SYSCTRL_GTIM1_RST_ENABLE()                   REGBITS_CLR(CW_SYSCTRL->APBRST1, (SYSCTRL_APB1_PERIPH_GTIM1))
#define __SYSCTRL_ATIM_RST_ENABLE()                    REGBITS_CLR(CW_SYSCTRL->APBRST1, (SYSCTRL_APB1_PERIPH_ATIM))
#define __SYSCTRL_UART2_RST_ENABLE()                   REGBITS_CLR(CW_SYSCTRL->APBRST1, (SYSCTRL_APB1_PERIPH_UART2))
#define __SYSCTRL_UART1_RST_ENABLE()                   REGBITS_CLR(CW_SYSCTRL->APBRST1, (SYSCTRL_APB1_PERIPH_UART1))
#define __SYSCTRL_SPI1_RST_ENABLE()                    REGBITS_CLR(CW_SYSCTRL->APBRST1, (SYSCTRL_APB1_PERIPH_SPI1))
#define __SYSCTRL_VC_RST_ENABLE()                      REGBITS_CLR(CW_SYSCTRL->APBRST1, (SYSCTRL_APB1_PERIPH_VC))
#define __SYSCTRL_ADC_RST_ENABLE()                     REGBITS_CLR(CW_SYSCTRL->APBRST1, (SYSCTRL_APB1_PERIPH_ADC)) 

#define __SYSCTRL_SPI3_RST_DISABLE()                    REGBITS_SET(CW_SYSCTRL->APBRST1, (SYSCTRL_APB1_PERIPH_SPI3))
#define __SYSCTRL_SPI2_RST_DISABLE()                    REGBITS_SET(CW_SYSCTRL->APBRST1, (SYSCTRL_APB1_PERIPH_SPI2))
#define __SYSCTRL_GTIM4_RST_DISABLE()                   REGBITS_SET(CW_SYSCTRL->APBRST1, (SYSCTRL_APB1_PERIPH_GTIM4))
#define __SYSCTRL_GTIM3_RST_DISABLE()                   REGBITS_SET(CW_SYSCTRL->APBRST1, (SYSCTRL_APB1_PERIPH_GTIM3))
#define __SYSCTRL_UART3_RST_DISABLE()                   REGBITS_SET(CW_SYSCTRL->APBRST1, (SYSCTRL_APB1_PERIPH_UART3))
#define __SYSCTRL_GTIM2_RST_DISABLE()                   REGBITS_SET(CW_SYSCTRL->APBRST1, (SYSCTRL_APB1_PERIPH_GTIM2))
#define __SYSCTRL_GTIM1_RST_DISABLE()                   REGBITS_SET(CW_SYSCTRL->APBRST1, (SYSCTRL_APB1_PERIPH_GTIM1))
#define __SYSCTRL_ATIM_RST_DISABLE()                    REGBITS_SET(CW_SYSCTRL->APBRST1, (SYSCTRL_APB1_PERIPH_ATIM))
#define __SYSCTRL_UART2_RST_DISABLE()                   REGBITS_SET(CW_SYSCTRL->APBRST1, (SYSCTRL_APB1_PERIPH_UART2))
#define __SYSCTRL_UART1_RST_DISABLE()                   REGBITS_SET(CW_SYSCTRL->APBRST1, (SYSCTRL_APB1_PERIPH_UART1))
#define __SYSCTRL_SPI1_RST_DISABLE()                    REGBITS_SET(CW_SYSCTRL->APBRST1, (SYSCTRL_APB1_PERIPH_SPI1))
#define __SYSCTRL_VC_RST_DISABLE()                      REGBITS_SET(CW_SYSCTRL->APBRST1, (SYSCTRL_APB1_PERIPH_VC))
#define __SYSCTRL_ADC_RST_DISABLE()                     REGBITS_SET(CW_SYSCTRL->APBRST1, (SYSCTRL_APB1_PERIPH_ADC)) 

/* SYSCTRL APB2 RST --------------------------------------------------------*/
#define __SYSCTRL_HALLTIM_RST_ENABLE()                   REGBITS_CLR(CW_SYSCTRL->APBRST2, (SYSCTRL_APB2_PERIPH_HALLTIM))
#define __SYSCTRL_I2C2_RST_ENABLE()                      REGBITS_CLR(CW_SYSCTRL->APBRST2, (SYSCTRL_APB2_PERIPH_I2C2))
#define __SYSCTRL_DAC_RST_ENABLE()                       REGBITS_CLR(CW_SYSCTRL->APBRST2, (SYSCTRL_APB2_PERIPH_DAC))
#define __SYSCTRL_OPA_RST_ENABLE()                       REGBITS_CLR(CW_SYSCTRL->APBRST2, (SYSCTRL_APB2_PERIPH_OPA))
#define __SYSCTRL_LPTIM_RST_ENABLE()                     REGBITS_CLR(CW_SYSCTRL->APBRST2, (SYSCTRL_APB2_PERIPH_LPTIM))
#define __SYSCTRL_I2C1_RST_ENABLE()                      REGBITS_CLR(CW_SYSCTRL->APBRST2, (SYSCTRL_APB2_PERIPH_I2C1))
#define __SYSCTRL_WWDT_RST_ENABLE()                      REGBITS_CLR(CW_SYSCTRL->APBRST2, (SYSCTRL_APB2_PERIPH_WWDT))
#define __SYSCTRL_IWDT_RST_ENABLE()                      REGBITS_CLR(CW_SYSCTRL->APBRST2, (SYSCTRL_APB2_PERIPH_IWDT))
#define __SYSCTRL_BTIM123_RST_ENABLE()                   REGBITS_CLR(CW_SYSCTRL->APBRST2, (SYSCTRL_APB2_PERIPH_BTIM123))
#define __SYSCTRL_RTC_RST_ENABLE()                       REGBITS_CLR(CW_SYSCTRL->APBRST2, (SYSCTRL_APB2_PERIPH_RTC))

#define __SYSCTRL_HALLTIM_RST_DISABLE()                   REGBITS_SET(CW_SYSCTRL->APBRST2, (SYSCTRL_APB2_PERIPH_HALLTIM))
#define __SYSCTRL_I2C2_RST_DISABLE()                      REGBITS_SET(CW_SYSCTRL->APBRST2, (SYSCTRL_APB2_PERIPH_I2C2))
#define __SYSCTRL_DAC_RST_DISABLE()                       REGBITS_SET(CW_SYSCTRL->APBRST2, (SYSCTRL_APB2_PERIPH_DAC))
#define __SYSCTRL_OPA_RST_DISABLE()                       REGBITS_SET(CW_SYSCTRL->APBRST2, (SYSCTRL_APB2_PERIPH_OPA))
#define __SYSCTRL_LPTIM_RST_DISABLE()                     REGBITS_SET(CW_SYSCTRL->APBRST2, (SYSCTRL_APB2_PERIPH_LPTIM))
#define __SYSCTRL_I2C1_RST_DISABLE()                      REGBITS_SET(CW_SYSCTRL->APBRST2, (SYSCTRL_APB2_PERIPH_I2C1))
#define __SYSCTRL_WWDT_RST_DISABLE()                      REGBITS_SET(CW_SYSCTRL->APBRST2, (SYSCTRL_APB2_PERIPH_WWDT))
#define __SYSCTRL_IWDT_RST_DISABLE()                      REGBITS_SET(CW_SYSCTRL->APBRST2, (SYSCTRL_APB2_PERIPH_IWDT))
#define __SYSCTRL_BTIM123_RST_DISABLE()                   REGBITS_SET(CW_SYSCTRL->APBRST2, (SYSCTRL_APB2_PERIPH_BTIM123))
#define __SYSCTRL_RTC_RST_DISABLE()                       REGBITS_SET(CW_SYSCTRL->APBRST2, (SYSCTRL_APB2_PERIPH_RTC))




#define SYSCTRL_MCO_DIV1                                (0x00U)
#define SYSCTRL_MCO_DIV2                                ((uint8_t)(0x01U << SYSCTRL_MCO_DIV_Pos))
#define SYSCTRL_MCO_DIV4                                ((uint8_t)(0x02U << SYSCTRL_MCO_DIV_Pos))
#define SYSCTRL_MCO_DIV8                                ((uint8_t)(0x03U << SYSCTRL_MCO_DIV_Pos))
#define SYSCTRL_MCO_DIV16                               ((uint8_t)(0x04U << SYSCTRL_MCO_DIV_Pos))
#define SYSCTRL_MCO_DIV32                               ((uint8_t)(0x05U << SYSCTRL_MCO_DIV_Pos))
#define SYSCTRL_MCO_DIV64                               ((uint8_t)(0x06U << SYSCTRL_MCO_DIV_Pos))
#define SYSCTRL_MCO_DIV128                              ((uint8_t)(0x07U << SYSCTRL_MCO_DIV_Pos))
#define SYSCTRL_MCO_DIV256                              ((uint8_t)(0x08U << SYSCTRL_MCO_DIV_Pos))
#define SYSCTRL_MCO_DIV512                              ((uint8_t)(0x09U << SYSCTRL_MCO_DIV_Pos))
#define SYSCTRL_MCO_DIV1024                             ((uint8_t)(0x0AU << SYSCTRL_MCO_DIV_Pos))
#define SYSCTRL_MCO_DIV2048                             ((uint8_t)(0x0BU << SYSCTRL_MCO_DIV_Pos))
#define SYSCTRL_MCO_DIV4096                             ((uint8_t)(0x0CU << SYSCTRL_MCO_DIV_Pos))
#define SYSCTRL_MCO_DIV8192                             ((uint8_t)(0x0DU << SYSCTRL_MCO_DIV_Pos))
#define IS_SYSCTRL_MCO_DIV(INSTANCE)                   ((INSTANCE) <= 0x0DU)

#define SYSCTRL_MCO_SRC_NONE                           (0x00U)
#define SYSCTRL_MCO_SRC_HCLK                           (0x01U)
#define SYSCTRL_MCO_SRC_PCLK                           (0x02U)
#define SYSCTRL_MCO_SRC_HSI                            (0x03U)
#define SYSCTRL_MCO_SRC_LSI                            (0x04U)
#define SYSCTRL_MCO_SRC_HSE                            (0x05U)
#define SYSCTRL_MCO_SRC_LSE                            (0x06U)
#define SYSCTRL_MCO_SRC_PLL                            (0x07U)
#define SYSCTRL_MCO_SRC_RC8K                           (0x09U)

#define IS_SYSCTRL_MCO_SRC(INSTANCE)                   (((INSTANCE) == SYSCTRL_MCO_SRC_NONE)       || \
                                                       ((INSTANCE) == SYSCTRL_MCO_SRC_HCLK)        || \
                                                       ((INSTANCE) == SYSCTRL_MCO_SRC_PCLK)        || \
                                                       ((INSTANCE) == SYSCTRL_MCO_SRC_HSI)         || \
                                                       ((INSTANCE) == SYSCTRL_MCO_SRC_LSI)         || \
                                                       ((INSTANCE) == SYSCTRL_MCO_SRC_HSE)         || \
                                                       ((INSTANCE) == SYSCTRL_MCO_SRC_LSE)         || \
                                                       ((INSTANCE) == SYSCTRL_MCO_SRC_PLL)         || \
                                                       ((INSTANCE) == SYSCTRL_MCO_SRC_RC8K))

// #define RCC_HSE_FLT_CLOSE                       (0x00000000U)
// #define RCC_HSE_FLT_OPEN                        (0x01000000U)
//==================================================================================
extern void SYSCTRL_HCLKPRS_Config(uint32_t HCLKPRS);
extern void SYSCTRL_PCLKPRS_Config(uint32_t PCLKPRS);
extern void SYSCTRL_SYSCLKSRC_Config(uint32_t SYSCLKSRC);
extern void SYSCTRL_CCS_Config(uint32_t CCS, FunctionalState NewState);
extern void SYSCTRL_LSELOCK_Config(uint32_t LSELOCK);
extern void SYSCTRL_WAKEUPCLK_Config(uint32_t WAKEUPCLK);
extern void SYSCTRL_LOCKUP_Config(uint32_t LOCKUP);
extern void SYSCTRL_SWDIO_Config(uint32_t SWDIO);
extern void SYSCTRL_BrakeConfig(uint32_t BrakeType, FunctionalState NewState);
extern int SYSCTRL_DeInit(void);
extern void SYSCTRL_SystemCoreClockUpdate(uint32_t NewFreq);
extern int SYSCTRL_HSI_Enable(uint32_t HSIDiv);
extern void SYSCTRL_HSI_Disable(void);
extern int SYSCTRL_LSI_Enable(void);
extern void SYSCTRL_LSI_Disable(void);
extern void SYSCTRL_HSEWaitClockSet(uint32_t WaitClock);
extern void SYSCTRL_HSEEnablePinPolSet(uint32_t Polarity);
int SYSCTRL_HSE_Enable(uint32_t Mode, uint32_t FreqIn, uint8_t Driver, uint32_t Flt);
void SYSCTRL_HSE_Disable(void);
extern int SYSCTRL_LSE_Enable(uint32_t Mode, uint8_t Driver);
extern void SYSCTRL_LSE_Disable(void);
extern int SYSCTRL_PLL_Enable(PLL_InitTypeDef *PLL_InitStruct);
void SYSCTRL_PLL_Disable(void);
extern int SYSCTRL_SysClk_Switch(uint32_t NewClk);
extern void SYSCTRL_ITConfig(uint32_t SYSCTRL_IT, FunctionalState NewState);
extern ITStatus SYSCTRL_GetITStatus(uint32_t SYSCTRL_IT);
extern void SYSCTRL_ClearITPendingBit(uint32_t SYSCTRL_IT);
extern FlagStatus SYSCTRL_GetStableFlag(uint32_t SYSCTRL_STABLEFLAG);
extern uint32_t SYSCTRL_GetAllStableFlag(void);
extern void SYSCTRL_AHBPeriphClk_Enable(uint32_t Periph, FunctionalState NewState);
extern void SYSCTRL_APBPeriphClk_Enable1(uint32_t Periph, FunctionalState NewState);
extern void SYSCTRL_APBPeriphClk_Enable2(uint32_t Periph, FunctionalState NewState);
extern void SYSCTRL_AHBPeriphReset(uint32_t Periph, FunctionalState NewState);
extern void SYSCTRL_APBPeriphReset1(uint32_t Periph, FunctionalState NewState);
extern void SYSCTRL_APBPeriphReset2(uint32_t Periph, FunctionalState NewState);
extern FlagStatus SYSCTRL_GetRstFlag(uint32_t SYSCTRL_RSTFLAG);
extern uint32_t SYSCTRL_GetAllRstFlag(void);
extern void SYSCTRL_ClearRstFlag(uint32_t SYSCTRL_RSTFLAG);

extern void SYSCTRL_PCLK_OUT(void);
extern void SYSCTRL_MCO_OUT(uint8_t Source, uint8_t Div);

extern uint32_t SYSCTRL_GetHClkFreq(void);
extern uint32_t SYSCTRL_GetPClkFreq(void);

void SYSCTRL_GotoSleep(void);
void SYSCTRL_GotoDeepSleep(void);
void SYSCTRL_ConfigSleepOnExit(FunctionalState state);

#ifdef __cplusplus
}
#endif

#endif /* __CW32L012_SYSCTRL_H */
