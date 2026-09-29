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
#ifndef __CW32L012_OPA_H__
#define __CW32L012_OPA_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/
#include "system_cw32l012.h"

/*****************************************************************************/
/* Global pre-processor symbols/macros ('#define')                           */
/*****************************************************************************/
/** @defgroup OPA枚举类型定义
  * @{
  */

/**
  * @brief OPA偏置电流
  */
typedef enum
{
    OPA_BIAS_1UA_36US                   = (0x00UL << OPAx_CR_BIAS_Pos),        /*!< OPA 偏置电流为 1uA*/
    OPA_BIAS_2UA_20US                   = (0x01UL << OPAx_CR_BIAS_Pos),        /*!< OPA 偏置电流为 2uA*/
    OPA_BIAS_3UA_11US                   = (0x02UL << OPAx_CR_BIAS_Pos),        /*!< OPA 偏置电流为 3uA*/
    OPA_BIAS_4UA_10US                   = (0x03UL << OPAx_CR_BIAS_Pos),        /*!< OPA 偏置电流为 4uA*/
    OPA_BIAS_5UA_8US                    = (0x04UL << OPAx_CR_BIAS_Pos),        /*!< OPA 偏置电流为 5uA*/
    OPA_BIAS_6UA_7US                    = (0x05UL << OPAx_CR_BIAS_Pos),        /*!< OPA 偏置电流为 6uA*/
    OPA_BIAS_7UA_6US                    = (0x06UL << OPAx_CR_BIAS_Pos),        /*!< OPA 偏置电流为 7uA*/
    OPA_BIAS_8UA_4US                    = (0x07UL << OPAx_CR_BIAS_Pos)         /*!< OPA 偏置电流为 8uA*/
} OPA_BiasTypeDef;

/**
  * @brief OPA PGA同向放大倍数
  */
typedef enum
{
    OPA_PGA_GAIN2                       = (0x00UL << OPAx_CR_AMP_Pos),        /*!< OPA PGA同向放大2倍 */
    OPA_PGA_GAIN4                       = (0x01UL << OPAx_CR_AMP_Pos),        /*!< OPA PGA同向放大4倍 */
    OPA_PGA_GAIN8                       = (0x02UL << OPAx_CR_AMP_Pos),        /*!< OPA PGA同向放大8倍 */
    OPA_PGA_GAIN16                      = (0x03UL << OPAx_CR_AMP_Pos),        /*!< OPA PGA同向放大16倍 */
    OPA_PGA_GAIN32                      = (0x04UL << OPAx_CR_AMP_Pos)         /*!< OPA PGA同向放大32倍 */
} OPA_PgaGainTypeDef;

/**
  * @brief OPA工作模式
  */
typedef enum
{
    OPA_WORKMODE_STANDALONE             = (0x00UL << OPAx_CR_MODE_Pos),        /*!< 独立运放模式，与外接电路联合工作 */
    OPA_WORKMODE_PGA                    = (0x02UL << OPAx_CR_MODE_Pos),        /*!< PGA模式，放大系数由GAIN控制 */
    OPA_WORKMODE_FOLLOWER               = (0x03UL << OPAx_CR_MODE_Pos)         /*!< 内置跟随模式，输出电压等于输入电压 */
} OPA_WorkModeTypeDef;

/**
  * @brief OPA校正时钟源
  */
typedef enum
{
    OPA_AZCLOCK_PCLK_DIV1               = (0x00UL << OPAx_CAL_CLKDIV_Pos),    /*!< Auto Zero Clk = PCLK/1   */
    OPA_AZCLOCK_PCLK_DIV2               = (0x01UL << OPAx_CAL_CLKDIV_Pos),    /*!< Auto Zero Clk = PCLK/2   */
    OPA_AZCLOCK_PCLK_DIV4               = (0x02UL << OPAx_CAL_CLKDIV_Pos),    /*!< Auto Zero Clk = PCLK/4   */
    OPA_AZCLOCK_PCLK_DIV8               = (0x03UL << OPAx_CAL_CLKDIV_Pos),    /*!< Auto Zero Clk = PCLK/8   */
    OPA_AZCLOCK_PCLK_DIV16              = (0x04UL << OPAx_CAL_CLKDIV_Pos),    /*!< Auto Zero Clk = PCLK/16  */
    OPA_AZCLOCK_PCLK_DIV32              = (0x05UL << OPAx_CAL_CLKDIV_Pos),    /*!< Auto Zero Clk = PCLK/32  */
    OPA_AZCLOCK_PCLK_DIV64              = (0x06UL << OPAx_CAL_CLKDIV_Pos),    /*!< Auto Zero Clk = PCLK/64  */
    OPA_AZCLOCK_PCLK_DIV128             = (0x07UL << OPAx_CAL_CLKDIV_Pos)     /*!< Auto Zero Clk = PCLK/128 */
} OPA_AutoZeroClockTypeDef;

/**
  * @brief OPA触发校正时长
  */
typedef enum
{
    OPA_AZPERIOD_AZCLK16                = (0x00UL << OPAx_CAL_CALPERIOD_Pos),      /*!< OPA 触发校正过程持续 16    个tAZCLK */
    OPA_AZPERIOD_AZCLK32                = (0x01UL << OPAx_CAL_CALPERIOD_Pos),      /*!< OPA 触发校正过程持续 32    个tAZCLK */
    OPA_AZPERIOD_AZCLK64                = (0x02UL << OPAx_CAL_CALPERIOD_Pos),      /*!< OPA 触发校正过程持续 64    个tAZCLK */
    OPA_AZPERIOD_AZCLK128               = (0x03UL << OPAx_CAL_CALPERIOD_Pos),      /*!< OPA 触发校正过程持续 128   个tAZCLK */
    OPA_AZPERIOD_AZCLK256               = (0x04UL << OPAx_CAL_CALPERIOD_Pos),      /*!< OPA 触发校正过程持续 256   个tAZCLK */
    OPA_AZPERIOD_AZCLK512               = (0x05UL << OPAx_CAL_CALPERIOD_Pos),      /*!< OPA 触发校正过程持续 512   个tAZCLK */
    OPA_AZPERIOD_AZCLK1024              = (0x06UL << OPAx_CAL_CALPERIOD_Pos),      /*!< OPA 触发校正过程持续 1024  个tAZCLK */
    OPA_AZPERIOD_AZCLK2048              = (0x07UL << OPAx_CAL_CALPERIOD_Pos),      /*!< OPA 触发校正过程持续 2048  个tAZCLK */
    OPA_AZPERIOD_AZCLK4096              = (0x08UL << OPAx_CAL_CALPERIOD_Pos),      /*!< OPA 触发校正过程持续 4096  个tAZCLK */
    OPA_AZPERIOD_AZCLK8192              = (0x09UL << OPAx_CAL_CALPERIOD_Pos),      /*!< OPA 触发校正过程持续 8192  个tAZCLK */
    OPA_AZPERIOD_AZCLK16384             = (0x0AUL << OPAx_CAL_CALPERIOD_Pos),      /*!< OPA 触发校正过程持续 16384 个tAZCLK */
    OPA_AZPERIOD_AZCLK32768             = (0x0BUL << OPAx_CAL_CALPERIOD_Pos),      /*!< OPA 触发校正过程持续 32768 个tAZCLK */
    OPA_AZPERIOD_AZCLK65536             = (0x0CUL << OPAx_CAL_CALPERIOD_Pos),      /*!< OPA 触发校正过程持续 65536 个tAZCLK */
    OPA_AZPERIOD_AZCLK131072            = (0x0DUL << OPAx_CAL_CALPERIOD_Pos),      /*!< OPA 触发校正过程持续 131072个tAZCLK */
    OPA_AZPERIOD_AZCLK262144            = (0x0EUL << OPAx_CAL_CALPERIOD_Pos),      /*!< OPA 触发校正过程持续 262144个tAZCLK */
    OPA_AZPERIOD_AZCLK524288            = (0x0FUL << OPAx_CAL_CALPERIOD_Pos)       /*!< OPA 触发校正过程持续 524288个tAZCLK */
} OPA_AutoZeroPeriodTypeDef;

/** @defgroup OPA结构体类型定义
  * @{
  */

/**
  * @brief  OPA基本功能初始化结构体
  */
typedef struct
{
    uint32_t                            InputP;                       /*!< OPA正端输入信号，可将部分参数或在一起使用，该参数可为下列之一:
                                                                           @arg OPA_INPUT_NONE    正端输入信号不连接到IO
                                                                           @arg OPA_INPUT_INP1    正端输入信号1连接到IO： OPA1_INP1 = PA03, OPA2_INP1 = PA04                               
                                                                           @arg OPA_INPUT_INP2    正端输入信号2连接到IO： OPA1_INP2 = PA06, OPA2_INP2 = PA06
                                                                           @arg OPA_INPUT_INP3    正端输入信号3连接到IO： OPA1_INP3 = PB01, OPA2_INP3 = PB00 
                                                                           @arg OPA_INPUT_DAC_OUT 正端输入信号连接到DACOUT: OPA1_INP4 = DAC1_OUTICT, OPA2_INP4 = DAC2_OUTICT */
    uint32_t                            InputN;                       /*!< OPA负端输入信号，可将部分参数或在一起使用，该参数可为下列之一:
                                                                           @arg OPA_INPUT_NONE    负端输入信号不连接到IO
                                                                           @arg OPA_INPUT_INN1    负端输入信号1连接到IO： OPA1_INN1 = PA04, OPA2_INN1 = PA05
                                                                           @arg OPA_INPUT_INN2    负端输入信号2连接到IO： OPA1_INN2 = PA07, OPA2_INN2 = PA07 */
    OPA_BiasTypeDef                     Bias;                         /*!< 偏置电流配置 */
    OPA_PgaGainTypeDef                  PgaGain;                      /*!< PGA同相放大倍数 */
    OPA_WorkModeTypeDef                 WorkMode;                     /*!< OPA工作模式，独立运放模式/内置PGA模式/内置跟随器模式 */
} OPA_InitTypeDef;
//============================================================
/** @addtogroup OPA1输入输出管脚
  * @{
  */
#define AFx_OPA1INP1_PA03()             (CW_GPIOA->ANALOG_f.PIN3 = 1U)
#define AFx_OPA1INP2_PA06()             (CW_GPIOA->ANALOG_f.PIN6 = 1U)
#define AFx_OPA1INP3_PB01()             (CW_GPIOB->ANALOG_f.PIN1 = 1U)
#define AFx_OPA1INN1_PA04()             (CW_GPIOA->ANALOG_f.PIN4 = 1U)
#define AFx_OPA1INN2_PA07()             (CW_GPIOA->ANALOG_f.PIN7 = 1U)
#define AFx_OPA1OUT_PB00()              (CW_GPIOB->ANALOG_f.PIN0 = 1U)
/** @} */

/** @addtogroup OPA2输入输出管脚
  * @{
  */
#define AFx_OPA2INP1_PA04()             (CW_GPIOA->ANALOG_f.PIN4 = 1U)
#define AFx_OPA2INP2_PA06()             (CW_GPIOA->ANALOG_f.PIN6 = 1U)
#define AFx_OPA2INP3_PB00()             (CW_GPIOB->ANALOG_f.PIN0 = 1U)
#define AFx_OPA2INN1_PA05()             (CW_GPIOA->ANALOG_f.PIN5 = 1U)
#define AFx_OPA2INN2_PA07()             (CW_GPIOA->ANALOG_f.PIN7 = 1U)
#define AFx_OPA2OUT_PB01()              (CW_GPIOB->ANALOG_f.PIN1 = 1U)
/** @} */

/** @addtogroup OPA端口输入信号使能控制
  * @{
  */
#define OPA_INPUT_NONE                   0
#define OPA_INPUT_INP1                   OPAx_CR_INP1EN_Msk           /*!< 正端输入信号1连接到IO使能控制 */
                                                                      /*!< - OPA1_INPUT_INP1 = PA03 */
                                                                      /*!< - OPA2_INPUT_INP1 = PA04 */
#define OPA_INPUT_INP2                   OPAx_CR_INP2EN_Msk           /*!< 正端输入信号2连接到IO使能控制 */
                                                                      /*!< - OPA1_INPUT_INP2 = PA06 */
                                                                      /*!< - OPA2_INPUT_INP2 = PA06 */
#define OPA_INPUT_INP3                   OPAx_CR_INP3EN_Msk           /*!< 正端输入信号3连接到IO使能控制 */
                                                                      /*!< - OPA1_INPUT_INP3 = PB01 */
                                                                      /*!< - OPA2_INPUT_INP3 = PB00 */
#define OPA_INPUT_DAC_OUT                OPAx_CR_INP4EN_Msk           /*!< 正端输入信号4连接到内部DAC使能控制 */
                                                                      /*!< - OPA1_INPUT_INP4 = DAC1_OUTICT */
                                                                      /*!< - OPA2_INPUT_INP4 = DAC2_OUTICT */
                                                                      
#define OPA_INPUT_INN1                   OPAx_CR_INN1EN_Msk           /*!< 负端输入信号1连接到IO使能控制 */
                                                                      /*!< - OPA1_INN1 = PA04 */
                                                                      /*!< - OPA2_INN1 = PA05 */
#define OPA_INPUT_INN2                   OPAx_CR_INN2EN_Msk           /*!< 负端输入信号2连接到IO使能控制 */
                                                                      /*!< - OPA1_INN2 = PA07 */
                                                                      /*!< - OPA2_INN2 = PA07 */
/** @} */

#define OPA_CR0_RESET_VALUE             0x0000E000
#define OPA_INP_MASK                    (OPA_INPUT_INP1 | OPA_INPUT_INP2 | OPA_INPUT_INP3 | OPA_INPUT_DAC_OUT)
#define OPA_INN_MASK                    (OPA_INPUT_INN1 | OPA_INPUT_INN2)

/** @addtogroup OPA软件触发校正的触发源
  */
#define OPA_AZTRIGSRC_NONE              (0x00000000UL)                /*!< 无校正触发源 */
#define OPA_AZTRIGSRC_SOFT              OPAx_CAL_SOFTTRIG_Msk         /*!< 使用软件触发校正 */
#define OPA_AZTRIGSRC_ADC1START         OPAx_CAL_ADC1START1_Msk        /*!< 使用ADC1START信号触发校正 */
#define OPA_AZTRIGSRC_ADC2START         OPAx_CAL_ADC2START1_Msk        /*!< 使用ADC2START信号触发校正 */
#define OPA_AZTRIGSRC_ATIMTRGO          OPAx_CAL_ATIMTRGO_Msk         /*!< 使用ATIMTRGO信号触发校正 */
#define OPA_AZTRIGSRC_ATIMOC4REFC       OPAx_CAL_ATIMOC4REFC_Msk      /*!< 使用ATIMOC4REFC信号触发校正 */
#define OPA_AZTRIGSRC_ATIMOC5REFC       OPAx_CAL_ATIMOC5REFC_Msk      /*!< 使用ATIMOC5REFC信号触发校正 */
#define OPA_AZTRIGSRC_ATIMOC6REFC       OPAx_CAL_ATIMOC6REFC_Msk      /*!< 使用ATIMOC6REFC信号触发校正 */
#define OPA_AZTRIGSRC_GTIM1TRGO         OPAx_CAL_GTIM1TRGO_Msk        /*!< 使用GTIM1TRGO信号触发校正 */
#define OPA_AZTRIGSRC_GTIM1OC1REFC      OPAx_CAL_GTIM1OC1REFC_Msk     /*!< 使用GTIM1OC1REFC信号触发校正 */
#define OPA_AZTRIGSRC_GTIM1OC2REFC      OPAx_CAL_GTIM1OC2REFC_Msk     /*!< 使用GTIM1OC2REFC信号触发校正 */
#define OPA_AZTRIGSRC_GTIM2TRGO         OPAx_CAL_GTIM2TRGO_Msk        /*!< 使用GTIM2TRGO信号触发校正 */
#define OPA_AZTRIGSRC_GTIM2OC1REFC      OPAx_CAL_GTIM2OC1REFC_Msk     /*!< 使用GTIM2OC1REFC信号触发校正 */
#define OPA_AZTRIGSRC_GTIM2OC2REFC      OPAx_CAL_GTIM2OC2REFC_Msk     /*!< 使用GTIM2OC2REFC信号触发校正 */
#define OPA_AZTRIGSRC_GTIM3TRGO         OPAx_CAL_GTIM3TRGO_Msk        /*!< 使用GTIM3TRGO信号触发校正 */
#define OPA_AZTRIGSRC_GTIM3OC1REFC      OPAx_CAL_GTIM3OC1REFC_Msk     /*!< 使用GTIM3OC1REFC信号触发校正 */
#define OPA_AZTRIGSRC_GTIM3OC2REFC      OPAx_CAL_GTIM3OC2REFC_Msk     /*!< 使用GTIM3OC2REFC信号触发校正 */
#define OPA_AZTRIGSRC_GTIM4TRGO         OPAx_CAL_GTIM4TRGO_Msk        /*!< 使用GTIM4TRGO信号触发校正 */
#define OPA_AZTRIGSRC_GTIM4OC1REFC      OPAx_CAL_GTIM4OC1REFC_Msk     /*!< 使用GTIM4OC1REFC信号触发校正 */
#define OPA_AZTRIGSRC_GTIM4OC2REFC      OPAx_CAL_GTIM4OC2REFC_Msk     /*!< 使用GTIM4OC2REFC信号触发校正 */
#define OPA_AZTRIGSRC_ALLHARD           0xFFFFC000;                   /*!< 使用所有硬件触发源校正 */

//============================================================

/******************************************************************************/
/* Global macro function                                                      */
/******************************************************************************/
/** @defgroup OPA宏函数定义
  * @{
  */

/**
  * @brief  等待OPA校正完成
  * @param  __INSTANCE__ OPA实体
  * @retval None
  */
#define OPA_WAIT_BUSY(__INSTANCE__)  while (((__INSTANCE__)->CAL & OPAx_CAL_AZRUN_Msk) != 0)

/**
  * @brief  创建OPA软件触发校正事件
  * @param  __INSTANCE__ OPA实体
  * @retval None
  */
#define OPA_AUTOZEROTRIG_SOFTEVENT(__INSTANCE__)                 \
do {                                                                 \
    REGBITS_SET((__INSTANCE__)->CAL, OPAx_CAL_SOFTTRIG_Msk);         \
    OPA_WAIT_BUSY(__INSTANCE__);                                 \
} while (0)
/** @} */


/******************************************************************************/
/* Global function prototypes ('extern', definition in C source)              */
/******************************************************************************/
/** @defgroup OPA驱动函数原型
  * @{
  */

void OPA_Init(OPA_TypeDef *OPAx, OPA_InitTypeDef *OPA_InitStruct);
void OPA_DeInit(OPA_TypeDef *OPAx);
void OPA_Start(OPA_TypeDef *OPAx);
void OPA_Stop(OPA_TypeDef *OPAx);

void OPA_AutoZeroSoft_Start(OPA_TypeDef *OPAx);
void OPA_AutoZeroSoft_Stop(OPA_TypeDef *OPAx);

void OPA_AutoZeroTrig_Config(OPA_TypeDef *OPAx, 
                                              OPA_AutoZeroClockTypeDef AutoZeroClk,
                                              OPA_AutoZeroPeriodTypeDef AutoZeroPeriod,
                                              uint32_t AutoZeroTrigSrc);
void OPA_AutoZeroTrig_SoftEvent(OPA_TypeDef *OPAx);


#ifdef __cplusplus
}
#endif

#endif
