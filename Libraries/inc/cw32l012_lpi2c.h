/**
 * @file cw32l012_lpi2c.h
 * @author WHXY
 * @brief
 * @version 0.1
 * @date 2022-7-12
 *
 * @copyright Copyright (c) 2021
 *
 */

/*******************************************************************************
 *
 * 代码许可和免责信息
 * 武汉力源半导体有限公司授予您使用所有编程代码示例的非专属的版权许可，您可以由此
 * 生成根据您的特定需要而定制的相似功能。根据不能被排除的任何法定保证，武汉力源半
 * 导体有限公司及其程序开发商和供应商对程序或技术支持（如果有）不提供任何明示或暗
 * 含的保证或条件，包括但不限于暗含的有关适销性、适用于某种特定用途和非侵权的保证
 * 或条件。
 * 无论何种情形，武汉力源半导体有限公司及其程序开发商或供应商均不对下列各项负责，
 * 即使被告知其发生的可能性时，也是如此：数据的丢失或损坏；直接的、特别的、附带的
 * 或间接的损害，或任何后果性经济损害；或利润、业务、收入、商誉或预期可节省金额的
 * 损失。
 * 某些司法辖区不允许对直接的、附带的或后果性的损害有任何的排除或限制，因此某些或
 * 全部上述排除或限制可能并不适用于您。
 *
 *******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CW32L012X_I2C_H
#define __CW32L012X_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "cw32l012.h"


/* Private_TypesDefinitions --------------------------------------------------*/
typedef int32_t      status_t;

/******************************************************************************/
/* Global enum type definitions                                               */
/******************************************************************************/
/** @defgroup I2C枚举类型定义
 * @{
 */

/**
 * @brief I2C 主从模式
 */
typedef enum
{
    I2C_ROLE_MASTER                         = 0,                                   /*!< 主机模式 */
    I2C_ROLE_SLAVE                                                                 /*!< 从机模式 */
} I2C_RoleTypeDef;

/**
* @brief I2C 从机地址模式
  */
typedef enum
{
    I2C_ADDRESS_7BIT                        = 0,                                   /*!< 7bit slaveAddr mode */
    I2C_ADDRESS_10BIT                                                              /*!< 10bit slaveAddr mode */
} I2C_AddressTypeDef;

/**
  * @brief I2C 从机寄存器地址模式
  */
typedef enum
{
    I2C_REG_ADDR_8BIT                       = 0,                                   /*!< 8bit slave Register Address */
    I2C_REG_ADDR_16BIT,                                                            /*!< 16bit slave Register Address */
    I2C_REG_ADDR_24BIT,                                                            /*!< 24bit slave Register Address */
    I2C_REG_ADDR_32BIT                                                             /*!< 32bit slave Register Address */
} I2C_SlaveRegAddrTypeDef;

/**
  * @brief I2C 读/写模式
  */
typedef enum
{
    I2C_MODE_WRITE                          = 0,                                   /*!< Master Write mode */
    I2C_MODE_READ                                                                  /*!< Master Read mode */
} I2C_ModeTypeDef;

/**
  * @brief I2C 时钟来源
  */
typedef enum
{
    I2C_CLK_SRC_PCLK                        = (0x0UL << I2Cx_MCR0_CLKSRC_Pos),      /*!< PCLK */
    I2C_CLK_SRC_LSE                         = (0x2UL << I2Cx_MCR0_CLKSRC_Pos),      /*!< LSE */
    I2C_CLK_SRC_LSI                         = (0x3UL << I2Cx_MCR0_CLKSRC_Pos),      /*!< LSI */
} I2C_MasterClkSrcTypeDef;

/**
  * @brief I2C 主机时钟分频系数
  */
typedef enum
{
    I2C_TCLK_Prescaler_Div1                 = 0,
    I2C_TCLK_Prescaler_Div2,
    I2C_TCLK_Prescaler_Div4,
    I2C_TCLK_Prescaler_Div8,
    I2C_TCLK_Prescaler_Div16,
    I2C_TCLK_Prescaler_Div32,
    I2C_TCLK_Prescaler_Div64,
    I2C_TCLK_Prescaler_Div128,
} I2C_TclkPrecaleTypeDef;

/**
  * @brief I2C slave 时钟延展和滤波时钟来源
  */
typedef enum
{
    I2C_CLK_SRC_PCLK_SLV                    = (0x0UL << I2Cx_SCR0_CLKSRC_Pos),     /*!< PCLK */
    I2C_CLK_SRC_LSE_SLV                     = (0x2UL << I2Cx_SCR0_CLKSRC_Pos),     /*!< LSE */
    I2C_CLK_SRC_LSI_SLV                     = (0x3UL << I2Cx_SCR0_CLKSRC_Pos),     /*!< LSI */
} I2C_SlaveClkSrcTypeDef;

/**
  * @brief I2C SDA 输入信号来源
  */
typedef enum
{
    I2C_SDA_INSRC_GPIO                      = (0x0UL << I2Cx_INSEL_SDAINSEL_Pos),  /*!< I2Cx_SDA管脚 */
    I2C_SDA_INSRC_VC1_OUT                   = (0x1UL << I2Cx_INSEL_SDAINSEL_Pos),  /*!< VC1_OUT */
    I2C_SDA_INSRC_VC2_OUT                   = (0x2UL << I2Cx_INSEL_SDAINSEL_Pos),  /*!< VC2_OUT */
    I2C_SDA_INSRC_VC3_OUT                   = (0x3UL << I2Cx_INSEL_SDAINSEL_Pos),  /*!< VC3_OUT */
    I2C_SDA_INSRC_VC4_OUT                   = (0x4UL << I2Cx_INSEL_SDAINSEL_Pos),  /*!< VC4_OUT */
} I2C_SdaInSrcTypeDef;

/**
  * @brief I2C SCL 输入信号来源
  */
typedef enum
{
    I2C_SCL_INSRC_GPIO                      = (0x0UL << I2Cx_INSEL_SCLINSEL_Pos),  /*!< I2Cx_SCL管脚 */
    I2C_SCL_INSRC_VC1_OUT                   = (0x1UL << I2Cx_INSEL_SCLINSEL_Pos),  /*!< VC1_OUT */
    I2C_SCL_INSRC_VC2_OUT                   = (0x2UL << I2Cx_INSEL_SCLINSEL_Pos),  /*!< VC2_OUT */
    I2C_SCL_INSRC_VC3_OUT                   = (0x3UL << I2Cx_INSEL_SCLINSEL_Pos),  /*!< VC3_OUT */
    I2C_SCL_INSRC_VC4_OUT                   = (0x4UL << I2Cx_INSEL_SCLINSEL_Pos),  /*!< VC4_OUT */
} I2C_SclInSrcTypeDef;

/**
  * @brief I2C 主机数据匹配方式
  */
typedef enum
{
    I2C_ByteMatch_Prohibit                  = (0x0UL),                             /*!< 禁止数据匹配 */
    I2C_ByteMatch_1st_M0_Or_M1              = (0x2UL),                             /*!< 第一个字节匹配 MATCH0 或 MATCH1 */
    I2C_ByteMatch_Any_M0_Or_M1              = (0x3UL),                             /*!< 任意字节匹配 MATCH0 或 MATCH1 */
    I2C_ByteMatch_1stM0_And_2ndM1           = (0x4UL),                             /*!< 第一个字节匹配 MATCH0 且第二个字节匹配 MATCH1 */
    I2C_ByteMatch_AnyM0_And_NextM1          = (0x5UL),                             /*!< 任意字节匹配 MATCH0 且下一字节匹配 MATCH1 */
    I2C_ByteMatch_1stAndM1_MAT_M0AndM1      = (0x6UL),                             /*!< 第一个字节与 MATCH1 相与后的值匹配 MATCH0 与MATCH1 相与后的值 */
    I2C_ByteMatch_AnyAndM1_MAT_M0AndM1      = (0x7UL),                             /*!< 任 意 字 节 与 MATCH1 相 与 后 的 值 匹 配 MATCH0 与MATCH1 相与后的值 */
} I2C_DataMatchTypeDef;

/**
  * @brief I2C 从机地址比较方式
  */
typedef enum
{
    I2C_Addr0_7bit                          = 0x0UL,                               /*!< 比较 ADDR0(7bit) */
    I2C_Addr0_10bit,                                                               /*!< 比较 ADDR0(10bit) */
    I2C_Addr0_Or_Addr1_7bit,                                                       /*!< 比较 ADDR0(7bit) / 比较 ADDR1(7bit) */
    I2C_Addr0_Or_Addr1_10bit,                                                      /*!< 比较 ADDR0(10bit) / 比较 ADDR1(10bit) */
    I2C_Addr0_7bit_Or_Addr1_10bit,                                                 /*!< 比较 ADDR0(7bit) / 比较 ADDR1(10bit) */
    I2C_Addr0_10bit_Or_Addr1_7bit,                                                 /*!< 比较 ADDR0(10bit) / 比较 ADDR1(7bit) */
    I2C_Addr0_To_Addr1_7bit,                                                       /*!< 比较地址空间 ADDR0 ~ ADDR1，7bit 模式 */
    I2C_Addr0_To_Addr1_10bit,                                                      /*!< 比较地址空间 ADDR0 ~ ADDR1，10bit 模式 */
} I2C_SlaveAddrCfgTypeDef;

/**
  * @brief I2C 主机待执行的指令
  */
enum I2C_MASTER_CMD
{
    I2C_Cmd_Tx_Data                         = (0x0UL << I2Cx_MTDR_CMD_Pos),        /*!< 向总线发送一个字节数据 MTDR[7:0] */
    I2C_Cmd_Rx_DataToMRDR                   = (0x1UL << I2Cx_MTDR_CMD_Pos),        /*!< 从总线接收（MTDR[7:0]+1）个字节数据，存入MRDR寄存器 */
    I2C_Cmd_Tx_STOP                         = (0x2UL << I2Cx_MTDR_CMD_Pos),        /*!< 在总线上产生 STOP 信号 */
    I2C_Cmd_Rx_Data_BypassMRDR              = (0x3UL << I2Cx_MTDR_CMD_Pos),        /*!< 从总线接收（MTDR[7:0]+1）个字节数据， 不存入MRDR 寄存器 */
    I2C_Cmd_Tx_RepSTART_Data_Rx_ACK         = (0x4UL << I2Cx_MTDR_CMD_Pos),        /*!< 在总线上生成（Repeated）START 信号，并向总线发送一个字节数据 MTDR[7:0]，期待从总线收到 ACK */
    I2C_Cmd_Tx_RepSTART_Data_Rx_NACK        = (0x5UL << I2Cx_MTDR_CMD_Pos),        /*!< 在总线上生成（Repeated）START 信号，并向总线发送一个字节数据 MTDR[7:0]，期待从总线收到 NACK */
    I2C_Cmd_Tx_RepSTART_Data_Rx_ACK_HS      = (0x6UL << I2Cx_MTDR_CMD_Pos),        /*!< 在高速模式下生成（Repeated）START 信号，并向总线发送一个字节数据 MTDR[7:0]，期待从总线收到 ACK */
    I2C_Cmd_Tx_RepSTART_Data_Rx_NACK_HS     = (0x7UL << I2Cx_MTDR_CMD_Pos),        /*!< 在高速模式下生成（Repeated）START 信号，并向总线发送一个字节数据 MTDR[7:0]，期待从总线收到 NACK */
} ;

/**
  * @brief I2C 总线状态字
  */
enum I2C_STATUS
{
    I2C_Status_Success = 0,
    I2C_Status_Busy,                                                               /*!< The master is already performing a transfer. */
    I2C_Status_Idle,                                                               /*!< The slave driver is idle. */
    I2C_Status_Nak,                                                                /*!< The slave device sent a NAK in response to a byte. */
    I2C_Status_FifoError,                                                          /*!< FIFO under run or overrun. */
    I2C_Status_BitError,                                                           /*!< Transferred bit was not seen on the bus. */
    I2C_Status_ArbitrationLost,                                                    /*!< Arbitration lost error. */
    I2C_Status_PinLowTimeout,                                                      /*!< SCL or SDA were held low longer than the timeout. */
    I2C_Status_NoTransferInProgress,                                               /*!< Attempt to abort a transfer when one is not in progress. */
    I2C_Status_DmaRequestFail,                                                     /*!< DMA request failed. */
    I2C_Status_Timeout,                                                            /*!< Timeout poling status flags. */
    I2C_Status_Error,
};

/* Private_Defines -----------------------------------------------------------*/
//I2C从设备地址，按照7/10 BIT设置
#define I2C_SLAVEADDRESS_EEPROM             (0xA0 >> 1)                            //读EEPROM时设置为(0xA0>>1)

#ifndef I2C_WAIT_TIMEOUT
#define I2C_WAIT_TIMEOUT                    0x100U                                 //Best to adjust this value according to MCU's HCLK frequency.
#endif

/*! @brief Retry times for waiting flag. */
#ifndef I2C_RETRY_TIMES
#define I2C_RETRY_TIMES                     0U                                     /* Define to zero means keep waiting until the flag is assert/deassert. */
#endif

#define MAX_SLAVE_ADDR_7B                   (0x7F)
#define MAX_SLAVE_ADDR_10B                  (0x3FF)
#define I2C_SLAVE_ADDR10B_MSB_MASK          (0x300)
#define I2C_SLAVE_ADDR10B_LSB_MASK          (0x0FF)
#define I2C_SLAVE_ADDR10B_MSB               (0xF0)                                 //b11110XXY
#define MAX_DATA_LEN_RX                     (1024UL)
#define MAX_DATA_LEN_EEPROM                 (256UL)
#define MAX_SIZE_SUBADDRESS                 (4UL)

#define IS_I2C_ALL_PERIPH(PERIPH)           (((PERIPH) == CW_I2C1) || ((PERIPH) == CW_I2C2))

#define I2C_CMD_TX_DATA                     (0U)
#define I2C_CMD_RX_DATA                     (1UL << I2Cx_MTDR_CMD_Pos)
#define I2C_CMD_STOP                        (2UL << I2Cx_MTDR_CMD_Pos)
#define I2C_CMD_START                       (4UL << I2Cx_MTDR_CMD_Pos)

/**
  * @defgroup I2C_Interrupts & Flags of I2C Master
  */
#define ENABLE_IT_MASK                      (0xFFFF80FC)                           // 定义清除I2C_MISR特定位的掩码
#define CLEAR_IT_MASK                       (0xFFFF80FF)                           // 定义清除I2C_MICR特定位的掩码
#define POLLING_IT_MASK                     (0xFCFF80FC)                           // 定义查询I2C_MISR特定位的掩码
// 中断标志位定义
#define I2C_IT_BUSBUSY                      (I2Cx_MISR_BUSBUSY_Msk)
#define I2C_IT_MSTBUSY                      (I2Cx_MISR_MSTBUSY_Msk)
#define I2C_IT_MATCH                        (I2Cx_MISR_MATCH_Msk)
#define I2C_IT_PINLOW                       (I2Cx_MISR_PINLOW_Msk)
#define I2C_IT_FIFO                         (I2Cx_MISR_FIFO_Msk)
#define I2C_IT_ARBI                         (I2Cx_MISR_ARBI_Msk)
#define I2C_IT_NACK                         (I2Cx_MISR_NACK_Msk)
#define I2C_IT_STOP                         (I2Cx_MISR_STOP_Msk)
#define I2C_IT_PACKET                       (I2Cx_MISR_PACKET_Msk)
#define I2C_IT_FRAME                        (I2Cx_MISR_FRAME_Msk)
#define I2C_IT_RXNE                         (I2Cx_MISR_RXNE_Msk)
#define I2C_IT_TXE                          (I2Cx_MISR_TXE_Msk)

// 检查中断标志位
#define IS_I2C_IT(IT)                       ((((IT) & POLLING_IT_MASK) == 0UL) && ((IT) != 0UL))
// 检查清除中断标志位
#define IS_I2C_CLEAR_IT(IT)                 ((((IT) & CLEAR_IT_MASK) == 0UL) && ((IT) != 0UL))
// 检查使能中断标志位
#define IS_I2C_ENABLE_IT(IT)                ((((IT) & ENABLE_IT_MASK) == 0UL) && ((IT) != 0UL))
// 检查标志位
#define IS_I2C_FLAG(FLAG)                   ((((FLAG) & POLLING_IT_MASK) == 0UL) && ((FLAG) != 0UL))
// 检查清除标志位
#define IS_I2C_CLEAR_FLAG(FLAG)             ((((FLAG) & CLEAR_IT_MASK) == 0UL) && ((FLAG) != 0UL))

/**
  * @defgroup I2C_Interrupts & Flags of I2C Slave
  */
#define ENABLE_IT_MASK_SLV                  (0xFFFF00F0)                           // 定义清除I2C_SISR特定位的掩码
#define CLEAR_IT_MASK_SLV                   (0xFFFFF0FF)                           // 定义清除I2C_SICR特定位的掩码
#define POLLING_IT_MASK_SLV                 (0xFCFF00F0)                           // 定义查询I2C_SISR特定位的掩码
// 中断标志位定义
#define I2C_IT_BUSBUSY_SLV                  (I2Cx_SISR_BUSBUSY_Msk)
#define I2C_IT_SLVBUSY_SLV                  (I2Cx_SISR_SLVBUSY_Msk)
#define I2C_IT_ALERT_SLV                    (I2Cx_SISR_ALERT_Msk)
#define I2C_IT_GC_SLV                       (I2Cx_SISR_GC_Msk)
#define I2C_IT_AM1_SLV                      (I2Cx_SISR_AM1_Msk)
#define I2C_IT_AM0_SLV                      (I2Cx_SISR_AM0_Msk)
#define I2C_IT_FIFO_SLV                     (I2Cx_SISR_FIFO_Msk)
#define I2C_IT_BIT_SLV                      (I2Cx_SISR_BIT_Msk)
#define I2C_IT_STOP_SLV                     (I2Cx_SISR_STOP_Msk)
#define I2C_IT_RESTART_SLV                  (I2Cx_SISR_RESTART_Msk)
#define I2C_IT_TACK_SLV                     (I2Cx_SISR_TACK_Msk)
#define I2C_IT_ADDR_SLV                     (I2Cx_SISR_ADDR_Msk)
#define I2C_IT_RXNE_SLV                     (I2Cx_SISR_RXNE_Msk)
#define I2C_IT_TXE_SLV                      (I2Cx_SISR_TXE_Msk)

// 检查中断标志位
#define IS_I2C_IT_SLV(IT)                   ((((IT) & POLLING_IT_MASK_SLV) == 0UL) && ((IT) != 0UL))
// 检查清除中断标志
#define IS_I2C_CLEAR_IT_SLV(IT)             ((((IT) & CLEAR_IT_MASK_SLV) == 0UL) && ((IT) != 0UL))
// 检查使能中断标志
#define IS_I2C_ENABLE_IT_SLV(IT)            ((((IT) & ENABLE_IT_MASK_SLV) == 0UL) && ((IT) != 0UL))
// 检查标志
#define IS_I2C_FLAG_SLV(FLAG)               ((((FLAG) & POLLING_IT_MASK_SLV) == 0UL) && ((FLAG) != 0UL))
// 检查清除标志
#define IS_I2C_CLEAR_FLAG_SLV(FLAG)         ((((FLAG) & CLEAR_IT_MASK_SLV) == 0UL) && ((FLAG) != 0UL))

/** Array initializer of LPI2C peripheral base addresses */
#define LPI2C_BASE_ADDRS                    { 0u, I2C1_BASE, I2C2_BASE,}
/** Array initializer of LPI2C peripheral base pointers */
#define LPI2C_BASE_PTRS                     { (I2C_TypeDef *)0u, CW_I2C1, CW_I2C2}
/** Interrupt vectors for the LPI2C peripheral type */
#define LPI2C_IRQS                          { Reset_IRQn, I2C1_IRQn, I2C2_IRQn}
#define FSL_FEATURE_I2C_FIFO_SIZEn(x)       (1)

//I2C Master
#define I2C_MCR1_RDMO(x)                    (((uint32_t)(((uint32_t)(x)) << I2Cx_MCR1_RDMO_Pos)) & I2Cx_MCR1_RDMO_Msk)

#define I2C_MCR2_MATCFG(x)                  (((uint32_t)(((uint32_t)(x)) << I2Cx_MCR2_MATCFG_Pos)) & I2Cx_MCR2_MATCFG_Msk)
#define I2C_MCR2_PINCFG(x)                  (((uint32_t)(((uint32_t)(x)) << I2Cx_MCR2_PINCFG_Pos)) & I2Cx_MCR2_PINCFG_Msk)
#define I2C_MCR2_IGNACK(x)                  (((uint32_t)(((uint32_t)(x)) << I2Cx_MCR2_IGNACK_Pos)) & I2Cx_MCR2_IGNACK_Msk)
#define I2C_MCR2_PRESCALE(x)                (((uint32_t)(((uint32_t)(x)) << I2Cx_MCR2_PRESCALE_Pos)) & I2Cx_MCR2_PRESCALE_Msk)

#define I2C_MCR3_BUSIDLE(x)                 (((uint32_t)(((uint32_t)(x)) << I2Cx_MCR3_BUSIDLE_Pos)) & I2Cx_MCR3_BUSIDLE_Msk)
#define I2C_MCR3_FILTSCL(x)                 (((uint32_t)(((uint32_t)(x)) << I2Cx_MCR3_FLTSCL_Pos)) & I2Cx_MCR3_FLTSCL_Msk)
#define I2C_MCR3_FILTSDA(x)                 (((uint32_t)(((uint32_t)(x)) << I2Cx_MCR3_FLTSDA_Pos)) & I2Cx_MCR3_FLTSDA_Msk)

#define I2C_MCR4_PINLOW(x)                  (((uint32_t)(((uint32_t)(x)) << I2Cx_MCR4_PINLOW_Pos)) & I2Cx_MCR4_PINLOW_Msk)

#define I2C_MCCR0_CLKLO(x)                  (((uint32_t)(((uint32_t)(x)) << I2Cx_MCCR_CLKLO_Pos)) & I2Cx_MCCR_CLKLO_Msk)
#define I2C_MCCR0_CLKHI(x)                  (((uint32_t)(((uint32_t)(x)) << I2Cx_MCCR_CLKHI_Pos)) & I2Cx_MCCR_CLKHI_Msk)
#define I2C_MCCR0_SETHOLD(x)                (((uint32_t)(((uint32_t)(x)) << I2Cx_MCCR_SETHOLD_Pos)) & I2Cx_MCCR_SETHOLD_Msk)
#define I2C_MCCR0_DATAVD(x)                 (((uint32_t)(((uint32_t)(x)) << I2Cx_MCCR_DATAVD_Pos)) & I2Cx_MCCR_DATAVD_Msk)

#define I2C_MTDR_CMD(x)                     (((uint32_t)(((uint32_t)(x)) << I2Cx_MTDR_CMD_Pos)) & I2Cx_MTDR_CMD_Msk)
#define I2C_MTDR_DATA(x)                    (((uint32_t)(((uint32_t)(x)) << I2Cx_MTDR_DATA_Pos)) & I2Cx_MTDR_DATA_Msk)

#define I2C_MCR_MEN(x)                      (((uint32_t)(((uint32_t)(x)) << I2Cx_MCR0_MEN_Pos)) & I2Cx_MCR0_MEN_Msk)

#define I2C_MDER_TDDE(x)                    (((uint32_t)(((uint32_t)(x)) << I2Cx_MDER_TXE_Pos)) & I2Cx_MDER_TXE_Msk)
#define I2C_MDER_RDDE(x)                    (((uint32_t)(((uint32_t)(x)) << I2Cx_MDER_RXNE_Pos)) & I2Cx_MDER_RXNE_Msk)

#define I2C_MFCR_TXWATER(x)                 (((uint32_t)(((uint32_t)(x)) << I2Cx_MFIFOCR_TXWATER_Pos)) & I2Cx_MFIFOCR_TXWATER_Msk)
#define I2C_MFCR_RXWATER(x)                 (((uint32_t)(((uint32_t)(x)) << I2Cx_MFIFOCR_RXWATER_Pos)) & I2Cx_MFIFOCR_RXWATER_Msk)

#define I2C_MDMR_MATCH0(x)                  (((uint32_t)(((uint32_t)(x)) << I2Cx_MMATCH_MATCH0_Pos)) & I2Cx_MMATCH_MATCH0_Msk)
#define I2C_MDMR_MATCH1(x)                  (((uint32_t)(((uint32_t)(x)) << I2Cx_MMATCH_MATCH1_Pos)) & I2Cx_MMATCH_MATCH1_Msk)
/*! @} */

//I2C Slave
#define I2C_SCR1_ADRSTALL(x)                (((uint32_t)(((uint32_t)(x)) << I2Cx_SCR1_ADRSTALL_Pos)) & I2Cx_SCR1_ADRSTALL_Msk)
#define I2C_SCR1_RXSTALL(x)                 (((uint32_t)(((uint32_t)(x)) << I2Cx_SCR1_RXSTALL_Pos)) & I2Cx_SCR1_RXSTALL_Msk)
#define I2C_SCR1_TXSTALL(x)                 (((uint32_t)(((uint32_t)(x)) << I2Cx_SCR1_TXSTALL_Pos)) & I2Cx_SCR1_TXSTALL_Msk)
#define I2C_SCR1_ACKSTALL(x)                (((uint32_t)(((uint32_t)(x)) << I2Cx_SCR1_ACKSTALL_Pos)) & I2Cx_SCR1_ACKSTALL_Msk)
#define I2C_SCR1_GCEN(x)                    (((uint32_t)(((uint32_t)(x)) << I2Cx_SCR1_GCEN_Pos)) & I2Cx_SCR1_GCEN_Msk)
#define I2C_SCR1_SAEN(x)                    (((uint32_t)(((uint32_t)(x)) << I2Cx_SCR1_ALERTEN_Pos)) & I2Cx_SCR1_ALERTEN_Msk)
#define I2C_SCR1_TXCFG(x)                   (((uint32_t)(((uint32_t)(x)) << I2Cx_SCR1_TXCFG_Pos)) & I2Cx_SCR1_TXCFG_Msk)
#define I2C_SCR1_RXCFG(x)                   (((uint32_t)(((uint32_t)(x)) << I2Cx_SCR1_RXCFG_Pos)) & I2Cx_SCR1_RXCFG_Msk)
#define I2C_SCR1_IGNACK(x)                  (((uint32_t)(((uint32_t)(x)) << I2Cx_SCR1_IGNACK_Pos)) & I2Cx_SCR1_IGNACK_Msk)
#define I2C_SCR1_HSMEN(x)                   (((uint32_t)(((uint32_t)(x)) << I2Cx_SCR1_HSMEN_Pos)) & I2Cx_SCR1_HSMEN_Msk)
#define I2C_SCR1_ADDRCFG(x)                 (((uint32_t)(((uint32_t)(x)) << I2Cx_SCR1_ADDRCFG_Pos)) & I2Cx_SCR1_ADDRCFG_Msk)

#define I2C_SCR3_CLKHOLD(x)                 (((uint32_t)(((uint32_t)(x)) << I2Cx_SCR3_CLKHOLD_Pos)) & I2Cx_SCR3_CLKHOLD_Msk)
#define I2C_SCR3_DATAVD(x)                  (((uint32_t)(((uint32_t)(x)) << I2Cx_SCR3_DATAVD_Pos)) & I2Cx_SCR3_DATAVD_Msk)
#define I2C_SCR3_FILTSCL(x)                 (((uint32_t)(((uint32_t)(x)) << I2Cx_SCR3_FLTSCL_Pos)) & I2Cx_SCR3_FLTSCL_Msk)
#define I2C_SCR3_FILTSDA(x)                 (((uint32_t)(((uint32_t)(x)) << I2Cx_SCR3_FLTSDA_Pos)) & I2Cx_SCR3_FLTSDA_Msk)

#define I2C_SCR0_SEN(x)                     (((uint32_t)(((uint32_t)(x)) << I2Cx_SCR0_SEN_Pos)) & I2Cx_SCR0_SEN_Msk)

#define I2C_SDER_AVDE(x)                    (((uint32_t)(((uint32_t)(x)) << I2Cx_SDER_ADDR_Pos)) & I2Cx_SDER_ADDR_Msk)
#define I2C_SDER_RDDE(x)                    (((uint32_t)(((uint32_t)(x)) << I2Cx_SDER_RXNE_Pos)) & I2Cx_SDER_RXNE_Msk)
#define I2C_SDER_TDDE(x)                    (((uint32_t)(((uint32_t)(x)) << I2Cx_SDER_TXE_Pos)) & I2Cx_SDER_TXE_Msk)

#define I2C_STAR_TXNACK(x)                  (((uint32_t)(((uint32_t)(x)) << I2Cx_STAR_TXNACK_Pos)) & I2Cx_STAR_TXNACK_Msk)
/*! @} */

/*!
 * @addtogroup lpi2c
 * @{
 */

/* ! @brief LPI2C master fifo commands. */
enum
{
    kTxDataCmd                              = I2C_MTDR_CMD(0x0U),                  /*!< Transmit DATA[7:0] */
    kRxDataCmd                              = I2C_MTDR_CMD(0X1U),                  /*!< Receive (DATA[7:0] + 1) bytes */
    kStopCmd                                = I2C_MTDR_CMD(0x2U),                  /*!< Generate STOP condition */
    kStartCmd                               = I2C_MTDR_CMD(0x4U),                  /*!< Generate(repeated) START and transmit address in DATA[[7:0] */
};

/*!
 * @brief Default watermark values.
 *
 * The default watermarks are set to zero.
 */
enum
{
    kDefaultTxWatermark                     = 0,
    kDefaultRxWatermark                     = 0,
};

/*! @brief States for the state machine used by transactional APIs. */
enum
{
    kIdleState                              = 0,
    kSendCommandState,
    kIssueReadCommandState,
    kTransferDataState,
    kStopState,
    kWaitForCompletionState,
};


/*! \public
 * @brief Generic status return codes.
 */
enum
{
    kStatus_Success                         = 0,                                   /*!< Generic status for Success. */
    kStatus_Fail                            = 1,                                   /*!< Generic status for Fail. */
    kStatus_ReadOnly                        = 2,                                   /*!< Generic status for read only failure. */
    kStatus_OutOfRange                      = 3,                                   /*!< Generic status for out of range access. */
    kStatus_InvalidArgument                 = 4,                                   /*!< Generic status for invalid argument check. */
    kStatus_Timeout                         = 5,                                   /*!< Generic status for timeout. */
    kStatus_NoTransferInProgress            = 6,                                   /*!< Generic status for no transfer in progress. */
    kStatus_Busy                            = 7,                                   /*!< Generic status for module is busy. */
    kStatus_NoData                          = 8,                                   /*!< Generic status for no data is found for the operation. */
};

/*! @brief LPI2C status return codes. */
enum
{
    kStatus_LPI2C_Busy                      = 100,                                 /*!< The master is already performing a transfer. */
    kStatus_LPI2C_Idle                      = 101,                                 /*!< The slave driver is idle. */
    kStatus_LPI2C_Nak                       = 102,                                 /*!< The slave device sent a NAK in response to a byte. */
    kStatus_LPI2C_FifoError                 = 103,                                 /*!< FIFO under run or overrun. */
    kStatus_LPI2C_BitError                  = 104,                                 /*!< Transferred bit was not seen on the bus. */
    kStatus_LPI2C_ArbitrationLost           = 105,                                 /*!< Arbitration lost error. */
    kStatus_LPI2C_PinLowTimeout             = 106,                                 /*!< SCL or SDA were held low longer than the timeout. */
    kStatus_LPI2C_NoTransferInProgress      = 107,                                 /*!< Attempt to abort a transfer when one is not in progress. */
    kStatus_LPI2C_DmaRequestFail            = 108,                                 /*!< DMA request failed. */
    kStatus_LPI2C_Timeout                   = 109,                                 /*!< Timeout polling status flags. */
};

/*! @} */

/*!
 * @addtogroup lpi2c_master_driver
 * @{
 */

/*!
 * @brief LPI2C master peripheral flags.
 *
 * @note These enums are meant to be OR'd together to form a bit mask.
 */
enum _lpi2c_master_flags
{
    kLPI2C_MasterTxReadyFlag                = I2Cx_MISR_TXE_Msk,                   /*!< Transmit data flag */
    kLPI2C_MasterRxReadyFlag                = I2Cx_MISR_RXNE_Msk,                  /*!< Receive data flag */
    kLPI2C_MasterEndOfPacketFlag            = I2Cx_MISR_PACKET_Msk,                /*!< End Packet flag */
    kLPI2C_MasterStopDetectFlag             = I2Cx_MISR_STOP_Msk,                  /*!< Stop detect flag */
    kLPI2C_MasterNackDetectFlag             = I2Cx_MISR_NACK_Msk,                  /*!< NACK detect flag */
    kLPI2C_MasterArbitrationLostFlag        = I2Cx_MISR_ARBI_Msk,                  /*!< Arbitration lost flag */
    kLPI2C_MasterFifoErrFlag                = I2Cx_MISR_FIFO_Msk,                  /*!< FIFO error flag */
    kLPI2C_MasterPinLowTimeoutFlag          = I2Cx_MISR_PINLOW_Msk,                /*!< Pin low timeout flag */
    kLPI2C_MasterDataMatchFlag              = I2Cx_MISR_MATCH_Msk,                 /*!< Data match flag */
    kLPI2C_MasterBusyFlag                   = I2Cx_MISR_MSTBUSY_Msk,               /*!< Master busy flag */
    kLPI2C_MasterBusBusyFlag                = I2Cx_MISR_BUSBUSY_Msk,               /*!< Bus busy flag */

    /*! All flags which are cleared by the driver upon starting a transfer. */
    kLPI2C_MasterClearFlags                 = I2Cx_MISR_MATCH_Msk   | I2Cx_MISR_PINLOW_Msk  |
                                              I2Cx_MISR_FIFO_Msk    | I2Cx_MISR_ARBI_Msk    |
                                              I2Cx_MISR_NACK_Msk    | I2Cx_MISR_STOP_Msk    |
                                              I2Cx_MISR_PACKET_Msk,

    /*! IRQ sources enabled by the non-blocking transactional API. */
    kLPI2C_MasterIrqFlags                   = I2Cx_MIER_MATCH_Msk  | I2Cx_MIER_PINLOW_Msk |
                                              I2Cx_MIER_FIFO_Msk   | I2Cx_MIER_ARBI_Msk   |
                                              I2Cx_MIER_NACK_Msk   | I2Cx_MIER_STOP_Msk   |
                                              I2Cx_MIER_PACKET_Msk | I2Cx_MIER_RXNE_Msk   |
                                              I2Cx_MIER_TXE_Msk,
    /*! Errors to check for. */
    kLPI2C_MasterErrorFlags                 = kLPI2C_MasterNackDetectFlag | kLPI2C_MasterArbitrationLostFlag |
                                              kLPI2C_MasterFifoErrFlag    | kLPI2C_MasterPinLowTimeoutFlag,
};

/*! @brief LPI2C pin configuration. */
typedef enum _lpi2c_master_pin_config
{
    kLPI2C_2PinOpenDrain                    = 0x0U,                                /*!< LPI2C Configured for 2-pin open drain mode */
    kLPI2C_2PinOutputOnly                   = 0x1U,                                /*!< LPI2C Configured for 2-pin output only mode (ultra-fast mode) */
    kLPI2C_2PinPushPull                     = 0x2U,                                /*!< LPI2C Configured for 2-pin push-pull mode */
    kLPI2C_4PinPushPull                     = 0x3U,                                /*!< LPI2C Configured for 4-pin push-pull mode */
    kLPI2C_2PinOpenDrainWithSeparateSlave   = 0x4U,                                /*!< LPI2C Configured for 2-pin open drain mode with separate LPI2C slave */
    kLPI2C_2PinOutputOnlyWithSeparateSlave  = 0x5U,                                /*!< LPI2C Configured for 2-pin output only mode(ultra-fast mode) with separate LPI2C slave */
    kLPI2C_2PinPushPullWithSeparateSlave    = 0x6U,                                /*!< LPI2C Configured for 2-pin push-pull mode with separate LPI2C slave */
    kLPI2C_4PinPushPullWithInvertedOutput   = 0x7U,                                /*!< LPI2C Configured for 4-pin push-pull mode(inverted outputs) */
} lpi2c_master_pin_config_t;

/*! @brief LPI2C master host request selection. */
typedef enum _lpi2c_host_request_source
{
    kLPI2C_HostRequestExternalPin           = 0x0U,                                /*!< Select the LPI2C_HREQ pin as the host request input */
    kLPI2C_HostRequestInputTrigger          = 0x1U,                                /*!< Select the input trigger as the host request input */
} lpi2c_host_request_source_t;

/*! @brief LPI2C master host request pin polarity configuration. */
typedef enum _lpi2c_host_request_polarity
{
    kLPI2C_HostRequestPinActiveLow          = 0x0U,                                /*!< Configure the LPI2C_HREQ pin active low */
    kLPI2C_HostRequestPinActiveHigh         = 0x1U,                                /*!< Configure the LPI2C_HREQ pin active high */
} lpi2c_host_request_polarity_t;

/**
 * \brief I2C主机初始化结构体
 */
typedef struct
{
    uint32_t                                I2C_BaudRate_Hz;
    uint32_t                                busIdleTimeout_ns;                     /*!< Bus idle timeout in nanoseconds. Set to 0 to disable. */
    uint32_t                                pinLowTimeout_ns;                      /*!< Pin low timeout in nanoseconds. Set to 0 to disable. */
    I2C_DataMatchTypeDef                    I2C_DataMatchMode;
    I2C_MasterClkSrcTypeDef                 I2C_ClockSouce;
    I2C_SdaInSrcTypeDef                     I2C_SDA_Source;
    I2C_SclInSrcTypeDef                     I2C_SCL_Source;
    boolean_t                               I2C_InDebugState;
    boolean_t                               I2C_InDeepSleepState;
    boolean_t                               I2C_RxDataMatch;
    boolean_t                               I2C_TxFIFO_Autoload;
    boolean_t                               ignoreAck;                             /*!< Whether to ignore ACK/NACK. */
    uint8_t                                 sdaGlitchFilterWidth_ns;               /*!< Width in nanoseconds of glitch filter on SDA pin. Set to 0 to disable. */
    uint8_t                                 sclGlitchFilterWidth_ns;               /*!< Width in nanoseconds of glitch filter on SCL pin. Set to 0 to disable. */
    uint8_t                                 I2C_PinsConfig;
    struct
    {
        boolean_t                           enable;                                /*!< Enable host request. */
        lpi2c_host_request_source_t         source;                                /*!< Host request source. */
        lpi2c_host_request_polarity_t       polarity;                              /*!< Host request pin polarity. */
    } hostRequest;
} I2C_MasterConfigTypeDef;

/*! @brief LPI2C master data match configuration structure. */
typedef struct _lpi2c_match_config
{
    I2C_DataMatchTypeDef matchMode;                                                /*!< Data match configuration setting. */
    boolean_t rxDataMatchOnly;                                                     /*!< When set to TRUE, received data is ignored until a successful match. */
    uint32_t match0;                                                               /*!< Match value 0. */
    uint32_t match1;                                                               /*!< Match value 1. */
} lpi2c_data_match_config_t;

/* Forward declaration of the transfer descriptor and handle typedefs. */
typedef struct _lpi2c_master_transfer lpi2c_master_transfer_t;
typedef struct _lpi2c_master_handle lpi2c_master_handle_t;

/*!
 * @brief Master completion callback function pointer type.
 *
 * This callback is used only for the non-blocking master transfer API. Specify the callback you wish to use
 * in the call to LPI2C_MasterTransferCreateHandle().
 *
 * @param I2Cx The LPI2C peripheral base address.
 * @param completionStatus Either kStatus_Success or an error code describing how the transfer completed.
 * @param userData Arbitrary pointer-sized value passed from the application.
 */
typedef void (*lpi2c_master_transfer_callback_t)(I2C_TypeDef *I2Cx,
        lpi2c_master_handle_t *handle,
        status_t completionStatus,
        void *userData);

/*!
 * @brief Transfer option flags.
 *
 * @note These enumerations are intended to be OR'd together to form a bit mask of options for
 * the #_lpi2c_master_transfer::flags field.
 */
enum _lpi2c_master_transfer_flags
{
    kLPI2C_TransferDefaultFlag              = 0x00U,                               /*!< Transfer starts with a start signal, stops with a stop signal. */
    kLPI2C_TransferNoStartFlag              = 0x01U,                               /*!< Don't send a start condition, address, and sub address */
    kLPI2C_TransferRepeatedStartFlag        = 0x02U,                               /*!< Send a repeated start condition */
    kLPI2C_TransferNoStopFlag               = 0x04U,                               /*!< Don't send a stop condition. */
};

/*!
 * @brief Non-blocking transfer descriptor structure.
 *
 * This structure is used to pass transaction parameters to the LPI2C_MasterTransferNonBlocking() API.
 */
struct _lpi2c_master_transfer
{
    uint32_t flags;                                                                /*!< Bit mask of options for the transfer. See enumeration #_lpi2c_master_transfer_flags for
                                                                                       available options. Set to 0 or #kLPI2C_TransferDefaultFlag for normal transfers. */
    uint16_t slaveAddress;                                                         /*!< The 7-bit slave address. */
    I2C_ModeTypeDef direction;                                                     /*!< Either #I2C_MODE_READ or #I2C_MODE_WRITE. */
    uint32_t subaddress;                                                           /*!< Sub address. Transferred MSB first. */
    size_t subaddressSize;                                                         /*!< Length of sub address to send in bytes. Maximum size is 4 bytes. */
    void *data;                                                                    /*!< Pointer to data to transfer. */
    size_t dataSize;                                                               /*!< Number of bytes to transfer. */
};

/*!
 * @brief Driver handle for master non-blocking APIs.
 * @note The contents of this structure are private and subject to change.
 */
struct _lpi2c_master_handle
{
    uint8_t state;                                                                 /*!< Transfer state machine current state. */
    uint16_t remainingBytes;                                                       /*!< Remaining byte count in current state. */
    uint8_t *buf;                                                                  /*!< Buffer pointer for current state. */
    uint16_t commandBuffer[6];                                                     /*!< LPI2C command sequence. When all 6 command words are used:
                                                                                       Start&addr&write[1 word] + subaddr[4 words] + restart&addr&read[1 word] */
    lpi2c_master_transfer_t transfer;                                              /*!< Copy of the current transfer info. */
    lpi2c_master_transfer_callback_t completionCallback;                           /*!< Callback function pointer. */
    void *userData;                                                                /*!< Application data passed to callback. */
};

/*! @brief Typedef for master interrupt handler, used internally for LPI2C master interrupt APIs.
 */
typedef void (*lpi2c_master_isr_t)(I2C_TypeDef *I2Cx, void *handle);

/*! @} */

/*!
 * @addtogroup lpi2c_slave_driver
 * @{
 */

/*!
 * @brief LPI2C slave peripheral flags.
 *
 * @note These enumerations are meant to be OR'd together to form a bit mask.
 */
enum _lpi2c_slave_flags
{
    kLPI2C_SlaveTxReadyFlag                 = I2Cx_SISR_TXE_Msk,                   /*!< Transmit data flag */
    kLPI2C_SlaveRxReadyFlag                 = I2Cx_SISR_RXNE_Msk,                  /*!< Receive data flag */
    kLPI2C_SlaveAddressValidFlag            = I2Cx_SISR_ADDR_Msk,                  /*!< Address valid flag */
    kLPI2C_SlaveTransmitAckFlag             = I2Cx_SISR_TACK_Msk,                  /*!< Transmit ACK flag */
    kLPI2C_SlaveRepeatedStartDetectFlag     = I2Cx_SISR_RESTART_Msk,               /*!< Repeated start detect flag */
    kLPI2C_SlaveStopDetectFlag              = I2Cx_SISR_STOP_Msk,                  /*!< Stop detect flag */
    kLPI2C_SlaveBitErrFlag                  = I2Cx_SISR_BIT_Msk,                   /*!< Bit error flag */
    kLPI2C_SlaveFifoErrFlag                 = I2Cx_SISR_FIFO_Msk,                  /*!< FIFO error flag */
    kLPI2C_SlaveAddressMatch0Flag           = I2Cx_SISR_AM0_Msk,                   /*!< Address match 0 flag */
    kLPI2C_SlaveAddressMatch1Flag           = I2Cx_SISR_AM1_Msk,                   /*!< Address match 1 flag */
    kLPI2C_SlaveGeneralCallFlag             = I2Cx_SISR_GC_Msk,                    /*!< General call flag */
    kLPI2C_SlaveBusyFlag                    = I2Cx_SISR_SLVBUSY_Msk,               /*!< Master busy flag */
    kLPI2C_SlaveBusBusyFlag                 = I2Cx_SISR_BUSBUSY_Msk,               /*!< Bus busy flag */
    /*! All flags which are cleared by the driver upon starting a transfer. */
    kLPI2C_SlaveClearFlags                  = I2Cx_SISR_RESTART_Msk | I2Cx_SISR_STOP_Msk |
                                              I2Cx_SISR_BIT_Msk     | I2Cx_SISR_FIFO_Msk,
    /*! IRQ sources enabled by the non-blocking transactional API. */
    kLPI2C_SlaveIrqFlags                    = I2Cx_SIER_ALERT_Msk  | I2Cx_SIER_GC_Msk      |
                                              I2Cx_SIER_AM1_Msk    | I2Cx_SIER_AM0_Msk     |
                                              I2Cx_SIER_FIFO_Msk   | I2Cx_SIER_BIT_Msk     |
                                              I2Cx_SIER_STOP_Msk   | I2Cx_SIER_RESTART_Msk |
                                              I2Cx_SIER_TACK_Msk   | I2Cx_SIER_ADDR_Msk    |
                                              I2Cx_SIER_RXNE_Msk   | I2Cx_SIER_TXE_Msk,

    /*! Errors to check for. */
    kLPI2C_SlaveErrorFlags                  = I2Cx_SISR_FIFO_Msk   | I2Cx_SISR_BIT_Msk,
};

/**
 * \brief I2C从机初始化结构体
 */
typedef struct
{
    uint32_t                                sdaGlitchFilterWidth_ns;               /*!< Width in nanoseconds of the digital filter on the SDA signal. Set to 0 to disable. */
    uint32_t                                sclGlitchFilterWidth_ns;               /*!< Width in nanoseconds of the digital filter on the SCL signal. Set to 0 to disable. */
    uint32_t                                dataValidDelay_ns;                     /*!< Width in nanoseconds of the data valid delay. */
    uint32_t                                clockHoldTime_ns;                      /*!< Width in nanoseconds of the clock hold time. */
    I2C_SlaveClkSrcTypeDef                  I2C_ClockSouce;
    uint16_t                                I2C_Addr0;
    uint16_t                                I2C_Addr1;
    I2C_SlaveAddrCfgTypeDef                 addressMatchMode;
    I2C_SlaveRegAddrTypeDef                 I2C_RegAddr;
    boolean_t                               filterEnable;                          /*!< Enable digital glitch filter. */
    boolean_t                               enableGeneralCall;                     /*!< Enable general call address matching. */
    struct
    {
        boolean_t                           enableAck;                             /*!< Enables SCL clock stretching during slave-transmit address byte(s)
                                                                                      and slave-receiver address and data byte(s) to allow software to
                                                                                      write the Transmit ACK Register before the ACK or NACK is transmitted.
                                                                                      Clock stretching occurs when transmitting the 9th bit. When
                                                                                      enableAckSCLStall is enabled, there is no need to set either
                                                                                      enableRxDataSCLStall or enableAddressSCLStall. */
        boolean_t                           enableTx;                              /*!< Enables SCL clock stretching when the transmit data flag is set
                                                                                      during a slave-transmit transfer. */
        boolean_t                           enableRx;                              /*!< Enables SCL clock stretching when receive data flag is set during
                                                                                      a slave-receive transfer. */
        boolean_t                           enableAddress;                         /*!< Enables SCL clock stretching when the address valid flag is asserted. */
    } sclStall;

    boolean_t                               ignoreAck;                             /*!< Continue transfers after a NACK is detected. */
    boolean_t                               enableReceivedAddressRead;             /*!< Enable reading the address received address as the first byte of data. */
} I2C_SlaveConfigTypeDef;

/*!
 * @brief Set of events sent to the callback for non blocking slave transfers.
 *
 * @note These enumerations are meant to be OR'd together to form a bit mask of events.
 */
typedef enum _lpi2c_slave_transfer_event
{
    kLPI2C_SlaveAddressMatchEvent           = 0x01U,                               /*!< Received the slave address after a start or repeated start. */
    kLPI2C_SlaveTransmitEvent               = 0x02U,                               /*!< Callback is requested to provide data to transmit
                                                                                       (slave-transmitter role). */
    kLPI2C_SlaveReceiveEvent                = 0x04U,                               /*!< Callback is requested to provide a buffer in which to place received
                                                                                       data (slave-receiver role). */
    kLPI2C_SlaveTransmitAckEvent            = 0x08U,                               /*!< Callback needs to either transmit an ACK or NACK. */
    kLPI2C_SlaveRepeatedStartEvent          = 0x10U,                               /*!< A repeated start was detected. */
    kLPI2C_SlaveCompletionEvent             = 0x20U,                               /*!< A stop was detected, completing the transfer. */

    /*! Bit mask of all available events. */
    kLPI2C_SlaveAllEvents                   = kLPI2C_SlaveAddressMatchEvent | kLPI2C_SlaveTransmitEvent | kLPI2C_SlaveReceiveEvent |
                                              kLPI2C_SlaveTransmitAckEvent | kLPI2C_SlaveRepeatedStartEvent | kLPI2C_SlaveCompletionEvent,
} lpi2c_slave_transfer_event_t;

/*! @brief LPI2C slave transfer structure */
typedef struct _lpi2c_slave_transfer
{
    lpi2c_slave_transfer_event_t event;                                            /*!< Reason the callback is being invoked. */
    uint8_t receivedAddress;                                                       /*!< Matching address send by master. */
    uint8_t *data;                                                                 /*!< Transfer buffer */
    size_t dataSize;                                                               /*!< Transfer size */
    status_t completionStatus;                                                     /*!< Success or error code describing how the transfer completed. Only applies for
                                                                                       #kLPI2C_SlaveCompletionEvent. */
    size_t transferredCount;                                                       /*!< Number of bytes actually transferred since start or last repeated start. */
} lpi2c_slave_transfer_t;

/* Forward declaration. */
typedef struct _lpi2c_slave_handle lpi2c_slave_handle_t;

/*!
 * @brief Slave event callback function pointer type.
 *
 * @param I2Cx Base address for the LPI2C instance on which the event occurred.
 * @param transfer Pointer to transfer descriptor containing values passed to and/or from the callback.
 * @param userData Arbitrary pointer-sized value passed from the application.
 */
typedef void (*lpi2c_slave_transfer_callback_t)(I2C_TypeDef *I2Cx, lpi2c_slave_transfer_t *transfer, void *userData);

/*!
 * @brief LPI2C slave handle structure.
 */
struct _lpi2c_slave_handle
{
    lpi2c_slave_transfer_t transfer;                                               /*!< LPI2C slave transfer copy. */
    boolean_t isBusy;                                                              /*!< Whether transfer is busy. */
    boolean_t wasTransmit;                                                         /*!< Whether the last transfer was a transmit. */
    uint32_t eventMask;                                                            /*!< Mask of enabled events. */
    uint32_t transferredCount;                                                     /*!< Count of bytes transferred. */
    lpi2c_slave_transfer_callback_t callback;                                      /*!< Callback function called at transfer event. */
    void *userData;                                                                /*!< Callback parameter passed to callback. */
};

/*! @brief Typedef for slave interrupt handler. */
typedef void (*lpi2c_slave_isr_t)(I2C_TypeDef *I2Cx, lpi2c_slave_handle_t *handle);
/*! @} */

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* Private_Variables ---------------------------------------------------------*/

/* Private_FunctionPrototypes ------------------------------------------------*/

/* Private_Functions ---------------------------------------------------------*/

/* Public_Functions ----------------------------------------------------------*/

// I2C Master使能或禁能
void I2C_MasterCmd(I2C_TypeDef *I2Cx, FunctionalState NewState);
// I2C Slave使能或禁能
void I2C_SlaveCmd(I2C_TypeDef *I2Cx, FunctionalState NewState);
// I2C Master初始化
ErrorStatus I2C_MasterInit(I2C_TypeDef *I2Cx, I2C_MasterConfigTypeDef *I2C_MasterConfig, uint32_t sourceClock_Hz);
// I2C Slave初始化
ErrorStatus I2C_SlaveInit(I2C_TypeDef *I2Cx, I2C_SlaveConfigTypeDef *I2C_SlaveConfig, uint32_t sourceClock_Hz);
// I2C 去初始化
void I2C_DeInit(I2C_TypeDef *I2Cx);
void I2C_MasterDeinit(I2C_TypeDef *I2Cx);
void I2C_SlaveDeinit(I2C_TypeDef *I2Cx);
// I2C Master设置波特率
void I2C_MasterSetBaudRate(I2C_TypeDef *I2Cx, uint32_t sourceClock_Hz, uint32_t baudRate_Hz);
// 发送START/ReSTART信号
uint8_t I2C_MasterSTART_Addr7b(I2C_TypeDef *I2Cx, uint8_t slaveAddr, I2C_ModeTypeDef dataMode);
uint8_t I2C_MasterReSTART_Addr7b(I2C_TypeDef *I2Cx, uint8_t slaveAddr, I2C_ModeTypeDef dataMode);
uint8_t I2C_MasterSTART_Addr10b(I2C_TypeDef *I2Cx, uint16_t slaveAddr, I2C_ModeTypeDef dataMode);
uint8_t I2C_MasterReSTART_Addr10b(I2C_TypeDef *I2Cx, uint16_t slaveAddr, I2C_ModeTypeDef dataMode);
// 发送STOP信号
uint8_t I2C_MasterSTOP(I2C_TypeDef *I2Cx);
// I2C 主机接收
uint8_t I2C_MasterReceive(I2C_TypeDef *I2Cx, uint8_t *rxBuff, uint16_t rxSize);
// I2C 主机发送
uint8_t I2C_MasterSend(I2C_TypeDef *I2Cx, uint8_t *txBuff, uint16_t txSize);
// I2C 从机接收
uint8_t I2C_SlaveReceive(I2C_TypeDef *I2Cx, uint8_t *rxBuff, uint16_t *rxSize);
// I2C 从机发送
uint8_t I2C_SlaveSend(I2C_TypeDef *I2Cx, uint8_t *txBuff, uint16_t txSize);

void I2C_MasterSoftReset(I2C_TypeDef *I2Cx);
void I2C_SlaveSoftReset(I2C_TypeDef *I2Cx);
void I2C_MasterTxFIFOReset(I2C_TypeDef *I2Cx);
void I2C_MasterRxFIFOReset(I2C_TypeDef *I2Cx);
void I2C_SlaveTxFIFOReset(I2C_TypeDef *I2Cx);
void I2C_SlaveRxFIFOReset(I2C_TypeDef *I2Cx);
void I2C_MasterBusFilter(I2C_TypeDef *I2Cx, uint8_t SDAFilter, uint8_t SCLFilter);
void I2C_SlaveBusFilter(I2C_TypeDef *I2Cx, uint8_t SDAFilter, uint8_t SCLFilter);

void I2C_MasterClearFlag(I2C_TypeDef *I2Cx, uint32_t I2C_FLAG);
uint8_t I2C_MasterPollingFlag(I2C_TypeDef *I2Cx, uint32_t I2C_FLAG);
uint8_t I2C_MasterPollingFlagReverse(I2C_TypeDef *I2Cx, uint32_t I2C_FLAG);
static uint8_t I2C_MasterPollingFlagCommon(I2C_TypeDef *I2Cx, uint32_t I2C_FLAG, boolean_t reverse);
uint32_t I2C_MasterGetFlag(I2C_TypeDef *I2Cx, uint32_t I2C_FLAG);

void I2C_SlaveClearFlag(I2C_TypeDef *I2Cx, uint32_t I2C_FLAG);
uint8_t I2C_SlavePollingFlag(I2C_TypeDef *I2Cx, uint32_t I2C_FLAG);
uint8_t I2C_SlavePollingFlagReverse(I2C_TypeDef *I2Cx, uint32_t I2C_FLAG);
static uint8_t I2C_SlavePollingFlagCommon(I2C_TypeDef *I2Cx, uint32_t I2C_FLAG, boolean_t reverse);
uint32_t I2C_SlaveGetFlag(I2C_TypeDef *I2Cx, uint32_t I2C_FLAG);

void I2C_MasterInitDefault(I2C_MasterConfigTypeDef *masterConfig);
void I2C_SlaveInitDefault(I2C_SlaveConfigTypeDef *slaveConfig);

// 主写EEPROM函数
uint8_t I2C_MasterWriteEepromData(I2C_TypeDef *I2Cx, uint32_t regAddr, I2C_SlaveRegAddrTypeDef regAddrType, uint8_t *pu8Data, uint16_t u16Len);
// 主读EEPROM函数
uint8_t I2C_MasterReadEepromData(I2C_TypeDef *I2Cx, uint32_t regAddr, I2C_SlaveRegAddrTypeDef regAddrType, uint8_t *pu8Data, uint16_t u16Len);

// 主机发送X字节数据
uint8_t I2C_MasterSendDataToSlave(I2C_TypeDef *I2Cx, uint16_t slaveAddr, uint8_t *pu8Data, uint16_t u16Len);
// 主机读取X字节数据
uint8_t I2C_MasterReceiveDataFromSlave(I2C_TypeDef *I2Cx, uint16_t slaveAddr, uint8_t *pu8RegAddr, I2C_SlaveRegAddrTypeDef regAddrType, uint8_t *pu8Data, uint16_t u16Len);

void I2C_SlaveTxNAck(I2C_TypeDef *I2Cx, FunctionalState stateNAck);

static uint32_t I2C_GetCyclesForWidth(uint32_t sourceClock_Hz, uint32_t width_ns, uint32_t minCycles, uint32_t maxCycles, uint32_t prescaler);

uint32_t LPI2C_GetInstance(I2C_TypeDef *I2Cx);

/*!
 * @addtogroup lpi2c_master_driver
 * @{
 */

void LPI2C_MasterConfigureDataMatch(I2C_TypeDef *I2Cx, const lpi2c_data_match_config_t *matchConfig);

status_t LPI2C_MasterCheckAndClearError(I2C_TypeDef *I2Cx, uint32_t status);

status_t LPI2C_CheckForBusyBus(I2C_TypeDef *I2Cx);

/*@}*/

/*! @name Status */
/*@{*/

/**
 * @brief 获取LPI2C主模式状态标志
 *
 * 该函数用于获取指定LPI2C接口的主模式状态标志。
 *
 * @param I2Cx LPI2C接口指针，指向一个I2C_TypeDef结构体
 *
 * @return 返回LPI2C主模式状态标志，通过读取MSR寄存器获取
 */
static inline uint32_t LPI2C_MasterGetStatusFlags(I2C_TypeDef *I2Cx)
{
    return I2Cx->MISR;
}

/*@}*/

/*! @name Interrupts */
/*@{*/

/**
 * @brief 使能指定的I2C主模式中断
 *
 * 该函数用于使能I2C主模式下指定的中断。
 *
 * @param I2Cx 指向I2C外设的指针
 * @param interruptMask 要使能的中断掩码，可以是多个中断的组合
 */
static inline void LPI2C_MasterEnableInterrupts(I2C_TypeDef *I2Cx, uint32_t interruptMask)
{
    I2Cx->MIER |= interruptMask;
}

/**
 * @brief 禁用LPI2C主机中断
 *
 * 禁用指定LPI2C主机接口上的指定中断。
 *
 * @param I2Cx LPI2C主机接口指针，指向I2C_TypeDef类型的结构体。
 * @param interruptMask 要禁用的中断掩码，可以通过按位或运算组合多个中断标志。
 */
static inline void LPI2C_MasterDisableInterrupts(I2C_TypeDef *I2Cx, uint32_t interruptMask)
{
    I2Cx->MIER &= ~interruptMask;
}

/**
 * @brief 获取LPI2C主模式已启用的中断
 *
 * 获取指定LPI2C接口在主模式下已启用的中断。
 *
 * @param I2Cx LPI2C接口指针
 * @return 已启用的中断标志，具体值参考LPI2C模块的MIER寄存器
 */
static inline uint32_t LPI2C_MasterGetEnabledInterrupts(I2C_TypeDef *I2Cx)
{
    return I2Cx->MIER;
}

/*@}*/

/*! @name DMA control */
/*@{*/

/**
 * @brief 使能或禁用LPI2C主机模式下的DMA传输
 *
 * 使能或禁用指定LPI2C接口在主机模式下的DMA传输功能。
 *
 * @param I2Cx 指向LPI2C接口的指针
 * @param enableTx 是否使能TX DMA传输，TRUE表示使能，FALSE表示禁用
 * @param enableRx 是否使能RX DMA传输，TRUE表示使能，FALSE表示禁用
 */
static inline void LPI2C_MasterEnableDMA(I2C_TypeDef *I2Cx, boolean_t enableTx, boolean_t enableRx)
{
    I2Cx->MDER = I2C_MDER_TDDE(enableTx) | I2C_MDER_RDDE(enableRx);
}

/**
 * @brief 获取LPI2C主模式发送FIFO的数据
 *
 * 获取指定LPI2C接口的发送FIFO（MTDR）的数据。
 *
 * @param I2Cx LPI2C接口指针
 * @return 返回一个uint32_t类型的值，表示LPI2C主模式发送FIFO的数据
 */
static inline uint32_t LPI2C_MasterGetTxFifoAddress(I2C_TypeDef *I2Cx)
{
    return (uint32_t)&I2Cx->MTDR;
}

/**
 * @brief 获取LPI2C主设备接收FIFO的数据
 *
 * 获取LPI2C主设备接收FIFO的数据。
 *
 * @param I2Cx LPI2C模块指针
 *
 * @return 返回接收FIFO的数据
 */
static inline uint32_t LPI2C_MasterGetRxFifoAddress(I2C_TypeDef *I2Cx)
{
    return (uint32_t)&I2Cx->MRDR;
}

/*@}*/

/*! @name FIFO control */
/*@{*/

/**
 * @brief 设置LPI2C主模式传输和接收的水位标记
 *
 * 该函数用于设置LPI2C主模式下的传输和接收水位标记。
 *
 * @param I2Cx 指向I2C类型定义的指针，表示要配置的LPI2C实例
 * @param txWords 要设置的传输水位标记，单位为字（word）
 * @param rxWords 要设置的接收水位标记，单位为字（word）
 */
static inline void LPI2C_MasterSetWatermarks(I2C_TypeDef *I2Cx, size_t txWords, size_t rxWords)
{
    I2Cx->MFIFOCR = I2C_MFCR_TXWATER(txWords) | I2C_MFCR_RXWATER(rxWords);
}

/**
 * @brief I2C主机获取FIFO中TX和RX的计数值
 *
 * 该函数用于获取I2C主模式FIFO中TX和RX的计数值。
 *
 * @param I2Cx I2C模块指针
 * @param rxCount 指向存储RX计数值的变量的指针
 * @param txCount 指向存储TX计数值的变量的指针
 *
 * @return 无
 */
static inline void LPI2C_MasterGetFifoCounts(I2C_TypeDef *I2Cx, size_t *rxCount, size_t *txCount)
{
    if (NULL != txCount)
    {
        *txCount = (I2Cx->MFIFOSR & I2Cx_MFIFOSR_TXCNT_Msk) >> I2Cx_MFIFOSR_TXCNT_Pos;
    }

    if (NULL != rxCount)
    {
        *rxCount = (I2Cx->MFIFOSR & I2Cx_MFIFOSR_RXCNT_Msk) >> I2Cx_MFIFOSR_RXCNT_Pos;
    }
}

/*@}*/

/*! @name Bus operations */
/*@{*/

/**
 * @brief 主机获取I2C总线的空闲状态
 *
 * 获取指定I2C总线的空闲状态。如果总线处于忙碌状态，则返回FALSE；否则返回TRUE。
 *
 * @param I2Cx 指向I2C模块的指针
 *
 * @return 如果I2C总线空闲，则返回TRUE；否则返回FALSE
 */
static inline boolean_t LPI2C_MasterGetBusIdleState(I2C_TypeDef *I2Cx)
{
    return ((I2Cx->MISR & I2Cx_MISR_BUSBUSY_Msk) >> I2Cx_MISR_BUSBUSY_Pos) == 1U ? TRUE : FALSE;
}

status_t LPI2C_MasterSend(I2C_TypeDef *I2Cx, void *txBuff, size_t txSize);

status_t LPI2C_MasterReceive(I2C_TypeDef *I2Cx, void *rxBuff, size_t rxSize);

status_t LPI2C_MasterStop(I2C_TypeDef *I2Cx);

status_t LPI2C_MasterTransferBlocking(I2C_TypeDef *I2Cx, lpi2c_master_transfer_t *transfer);

/*@}*/

/*! @name Non-blocking */
/*@{*/
void LPI2C_MasterTransferCreateHandle(I2C_TypeDef *I2Cx,
                                      lpi2c_master_handle_t *handle,
                                      lpi2c_master_transfer_callback_t callback,
                                      void *userData);

status_t LPI2C_MasterTransferNonBlocking(I2C_TypeDef *I2Cx,
        lpi2c_master_handle_t *handle,
        lpi2c_master_transfer_t *transfer);

status_t LPI2C_MasterTransferGetCount(I2C_TypeDef *I2Cx, lpi2c_master_handle_t *handle, size_t *count);

void LPI2C_MasterTransferAbort(I2C_TypeDef *I2Cx, lpi2c_master_handle_t *handle);

/*@}*/

/*! @name IRQ handler */
/*@{*/
void LPI2C_MasterTransferHandleIRQ(I2C_TypeDef *I2Cx, void *lpi2cMasterHandle);

/*@}*/

/*! @} */

/*!
 * @addtogroup lpi2c_slave_driver
 * @{
 */

/**
 * @brief 获取LPI2C从设备状态标志
 *
 * 该函数用于获取指定的LPI2C从设备状态标志。
 *
 * @param I2Cx 指向I2C类型的指针，指向要操作的LPI2C从设备
 *
 * @return 返回从设备的状态标志，类型为uint32_t
 */
static inline uint32_t LPI2C_SlaveGetStatusFlags(I2C_TypeDef *I2Cx)
{
    return I2Cx->SISR;
}

/*@}*/

/*! @name Slave interrupts */
/*@{*/

/**
 * @brief 使能指定I2C从设备的中断
 *
 * 该函数用于使能指定I2C从设备的中断。通过向I2C从设备的SIER寄存器写入指定的中断掩码，
 * 来使能相应的中断。
 *
 * @param I2Cx 指向I2C设备的指针
 * @param interruptMask 需要使能的中断掩码，可以通过按位或操作组合多个中断标志
 */
static inline void LPI2C_SlaveEnableInterrupts(I2C_TypeDef *I2Cx, uint32_t interruptMask)
{
    I2Cx->SIER |= interruptMask;
}

/**
 * @brief 禁用I2C从设备的中断
 *
 * 该函数用于禁用指定I2C从设备的中断。
 *
 * @param I2Cx I2C设备指针
 * @param interruptMask 需要禁用的中断掩码
 */
static inline void LPI2C_SlaveDisableInterrupts(I2C_TypeDef *I2Cx, uint32_t interruptMask)
{
    I2Cx->SIER &= ~interruptMask;
}

/**
 * @brief 获取LPI2C从机使能的中断
 *
 * 获取指定I2C从机使能的中断状态。
 *
 * @param I2Cx 指向I2C模块的指针
 *
 * @return 返回I2Cx->SIER寄存器的值，即当前使能的中断状态
 */
static inline uint32_t LPI2C_SlaveGetEnabledInterrupts(I2C_TypeDef *I2Cx)
{
    return I2Cx->SIER;
}

/*@}*/

/*! @name Slave DMA control */
/*@{*/

/**
 * @brief 使能LPI2C从设备的DMA功能
 *
 * 该函数用于配置LPI2C从设备的DMA相关功能，包括地址有效性、接收和发送使能。
 *
 * @param I2Cx LPI2C设备指针，指向需要配置的LPI2C外设实例。
 * @param enableAddressValid 是否使能地址有效性检查。
 *                           TRUE: 使能地址有效性检查；
 *                           FALSE: 不使能地址有效性检查。
 * @param enableRx 是否使能DMA接收功能。
 *                TRUE: 使能DMA接收功能；
 *                FALSE: 不使能DMA接收功能。
 * @param enableTx 是否使能DMA发送功能。
 *                TRUE: 使能DMA发送功能；
 *                FALSE: 不使能DMA发送功能。
 */
static inline void LPI2C_SlaveEnableDMA(I2C_TypeDef *I2Cx, boolean_t enableAddressValid, boolean_t enableRx, boolean_t enableTx)
{
    I2Cx->SDER = (I2Cx->SDER & ~(I2Cx_SDER_ADDR_Msk | I2Cx_SDER_RXNE_Msk | I2Cx_SDER_TXE_Msk)) |
                 I2C_SDER_AVDE(enableAddressValid) | I2C_SDER_RDDE(enableRx) | I2C_SDER_TDDE(enableTx);
}

/*@}*/

/*! @name Slave bus operations */
/*@{*/

/**
 * @brief 从设备获取LPI2C的总线空闲状态
 *
 * 该函数用于从设备获取指定LPI2C的总线空闲状态。
 *
 * @param I2Cx LPI2C设备实例指针
 *
 * @return 返回一个布尔值，表示总线是否空闲：
 *         - TRUE：总线空闲
 *         - FALSE：总线忙碌
 */
static inline boolean_t LPI2C_SlaveGetBusIdleState(I2C_TypeDef *I2Cx)
{
    return ((I2Cx->SISR & I2Cx_SISR_BUSBUSY_Msk) >> I2Cx_SISR_BUSBUSY_Pos) == 1U ? TRUE : FALSE;
}

/**
 * @brief 从设备发送应答或非应答信号
 *
 * 该函数用于在I2C通信过程中，从设备发送应答（ACK）或非应答（NACK）信号。
 *
 * @param I2Cx I2C设备指针，指向需要操作的I2C设备。
 * @param ackOrNack 应答信号标志，传入TRUE表示发送应答信号，传入FALSE表示发送非应答信号。
 */
static inline void LPI2C_SlaveTransmitAck(I2C_TypeDef *I2Cx, boolean_t ackOrNack)
{
    I2Cx->STAR = I2C_STAR_TXNACK(!ackOrNack);
}

/**
 * @brief LPI2C从设备获取接收到的地址
 *
 * 该函数从指定的LPI2C设备的状态和控制寄存器中获取从设备接收到的地址。
 *
 * @param I2Cx 指向I2C设备类型的指针
 *
 * @return 返回接收到的地址
 */
static inline uint32_t LPI2C_SlaveGetReceivedAddress(I2C_TypeDef *I2Cx)
{
    return I2Cx->SASR & I2Cx_SASR_RADDR_Msk;
}

status_t LPI2C_SlaveSend(I2C_TypeDef *I2Cx, void *txBuff, size_t txSize, size_t *actualTxSize);

status_t LPI2C_SlaveReceive(I2C_TypeDef *I2Cx, void *rxBuff, size_t rxSize, size_t *actualRxSize);

/*@}*/

/*! @name Slave non-blocking */
/*@{*/

void LPI2C_SlaveTransferCreateHandle(I2C_TypeDef *I2Cx,
                                     lpi2c_slave_handle_t *handle,
                                     lpi2c_slave_transfer_callback_t callback,
                                     void *userData);

status_t LPI2C_SlaveTransferNonBlocking(I2C_TypeDef *I2Cx, lpi2c_slave_handle_t *handle, uint32_t eventMask);

status_t LPI2C_SlaveTransferGetCount(I2C_TypeDef *I2Cx, lpi2c_slave_handle_t *handle, size_t *count);

void LPI2C_SlaveTransferAbort(I2C_TypeDef *I2Cx, lpi2c_slave_handle_t *handle);

/*@}*/

/*! @name Slave IRQ handler */
/*@{*/
void LPI2C_SlaveTransferHandleIRQ(I2C_TypeDef *I2Cx, lpi2c_slave_handle_t *handle);

void LPI2C_CommonIRQHandler(I2C_TypeDef *I2Cx, uint32_t instance);

#ifdef __cplusplus
}

#endif

#endif /*__CW32L012_LPI2C_H */
