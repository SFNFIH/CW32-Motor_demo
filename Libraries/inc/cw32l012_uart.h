/*******************************************************************************
 * @file cw32l012_uart.h
 * @brief CW32L012 UART驱动头文件
 * @version 1.0
 * @date 
 * @copyright Copyright (c) 2025
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
 ********************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CW32L012_UART_H
#define __CW32L012_UART_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "system_cw32l012.h" 
#include "cw32l012_dma.h"

/******************************************************************************
 * type definitions ('typedef')
 ******************************************************************************/

/**
 * @brief  UART Init Structure definition
 */

typedef struct
{
    uint32_t UART_BaudRate;            /*!< 波特率
                                           计算公式:
                                            - UART_Over = 00， BaudRate = UCLK / (16 * BRRI + BRRF)
                                            - UART_Over = 01， BaudRate = UCLK / (8 * BRRI)
                                            - UART_Over = 10， BaudRate = UCLK / (4 * BRRI)
                                            - UART_Over = 11， BaudRate = (256 * UCLK) / BRRI */

    uint16_t UART_Over;                /*!< 采样方式
                                           参数取值 @ref UART_Over */

    uint16_t UART_Source;              /*!< 传输时钟源UCLK
                                           参数取值 @ref UART_Source */

    uint32_t UART_UclkFreq;            /*!< 传输时钟UCLK频率 Hz */

    uint16_t UART_StartBit;            /*!< 起始位判定方式
                                           参数取值 @ref UART_Start_Bit */

    uint16_t UART_StopBits;            /*!< 停止位长度
                                           参数取值 @ref UART_Stop_Bits */

    uint16_t UART_Parity;              /*!< 校验方式
                                           参数取值 @ref UART_Parity
                                           @note 当设置为无校验时，数据字长为8位；
                                                 当设置为有校验时，数据字长自动设置为9位 */

    uint16_t UART_Mode;                /*!< 发送/接收使能
                                           参数取值 @ref UART_Mode */

    uint16_t UART_HardwareFlowControl; /*!< 硬件流控
                                           参数取值 @ref UART_Hardware_Flow_Control */
} UART_InitTypeDef;

/**
  * @brief  UART Clock Init Structure definition
  */

typedef struct
{
    uint16_t UART_Clock;   /*!< UART同步模式使能/失能
                               参数取值 @ref UART_Clock */

    uint16_t UART_Source;  /*!< 传输时钟源UCLK
                               参数取值 @ref UART_Source */
} UART_ClockInitTypeDef;

typedef struct
{
  uint32_t UART_ClockSource;   /*!< 时钟源选择 */
  uint32_t UART_ClockFreq;     /*!< 时钟频率 Hz */
  uint32_t UART_BaudRate;      /*!< 波特率 */
  uint32_t UART_TransmitOrder; /*!< 发送顺序 (0: LSB first, 1: MSB first)
                                    参数取值 @ref UART_Transmit_Order */
} UART_SyncModeInitTypeDef;

/**
  * @brief  UART 状态枚举定义
  */
typedef enum
{
    UART_STATUS_OK        = 0x00,  /*!< UART 操作成功 */
    UART_STATUS_ERROR     = 0x01,  /*!< UART 操作出错 */
    UART_STATUS_BUSY      = 0x02,  /*!< UART 忙碌 */
    UART_STATUS_TIMEOUT   = 0x03,  /*!< UART 操作超时 */
    UART_STATUS_PE        = 0x04,  /*!< UART 奇偶校验错误 */
    UART_STATUS_FE        = 0x05,  /*!< UART 帧错误 */
    UART_STATUS_NE        = 0x06,  /*!< UART 噪声错误 */
    UART_STATUS_ORE       = 0x07   /*!< UART 过载错误 */
} UART_StatusTypeDef;

// TIMCR位域定义
typedef enum {
    UART_TIMER_STOP_IMMEDIATELY = 0x0,       // 立即停止定时器
    UART_TIMER_WAIT_FOR_TIMEOUT = 0x2,       // 等待超时检测模式
    UART_TIMER_RECEIVE_IDLE = 0x3,           // 接收空闲检测模式
    UART_TIMER_AUTO_BAUD_RATE_1 = 0x4,       // 自动波特率检测模式1
    UART_TIMER_AUTO_BAUD_RATE_2 = 0x5,       // 自动波特率检测模式2
    UART_TIMER_GENERAL_PURPOSE = 0x7         // 通用定时模式
} UART_TimerModeTypeDef;

/**
 * @brief  UART 自动波特率检测模式
 * 
 */
typedef enum {
    UART_AUTO_BAUD_RATE_MODE_1 = 0x0,  // 自动波特率检测模式1
    UART_AUTO_BAUD_RATE_MODE_2 = 0x1   // 自动波特率检测模式2 
} UART_AutoBaudRateModeTypeDef;

/**
 * @brief  UART LIN 模式间隔段检测长度
 */
typedef enum {
    UART_LIN_BREAK_DETECT_LENGTH_10B = 0x0,  // 10位
    UART_LIN_BREAK_DETECT_LENGTH_11B = 0x1   // 11位
} UART_LINBreakDetectLengthTypeDef;


/**
 * @brief  UART LIN 模式间隔段发送长度
 */
typedef enum {
    UART_LIN_BREAK_SEND_LENGTH_13B = 0x040D,   // 13位
    UART_LIN_BREAK_SEND_LENGTH_14B = 0x040E,   // 14位
    UART_LIN_BREAK_SEND_LENGTH_15B = 0x040F,   // 15位
    UART_LIN_BREAK_SEND_LENGTH_16B = 0x0410,   // 16位
    UART_LIN_BREAK_SEND_LENGTH_17B = 0x0411    // 17位
} UART_LINBreakSendLengthTypeDef;

/******************************************************************************
 * pre-processor symbols/macros ('#define')
 ******************************************************************************/

/** @defgroup UART_Over
  * @{
  */

#define UART_Over_16                        (0u << UARTx_CR1_OVER_Pos)
#define UART_Over_8                         (1u << UARTx_CR1_OVER_Pos)
#define UART_Over_4                         (2u << UARTx_CR1_OVER_Pos)
#define UART_Over_sp                        (3u << UARTx_CR1_OVER_Pos)
#define IS_UART_Over(Over)                  (((Over) == UART_Over_16) || \
                                             ((Over) == UART_Over_8)  || \
                                             ((Over) == UART_Over_4)  || \
                                             ((Over) == UART_Over_sp))

/** @defgroup UART_Source
  * @{
  */

#define UART_Source_PCLK                    (1u << UARTx_CR1_SOURCE_Pos)
#define UART_Source_LSE                     (2u << UARTx_CR1_SOURCE_Pos)
#define UART_Source_LSI                     (3u << UARTx_CR1_SOURCE_Pos)
#define IS_UART_Source(Source)              (((Source) == UART_Source_PCLK) || \
                                             ((Source) == UART_Source_LSE)  || \
                                             ((Source) == UART_Source_LSI))

/** @defgroup UART_Start_Bit
  * @{
  */

#define UART_StartBit_FE                    (0u) //RXD下降沿
#define UART_StartBit_LL                    (1u << UARTx_CR1_START_Pos) //RXD低电平
#define IS_UART_StartBit(StartBit)          (((StartBit) == UART_StartBit_FE) || \
                                             ((StartBit) == UART_StartBit_LL))

/** @defgroup UART_Stop_Bits
  * @{
  */

#define UART_StopBits_1                     (0u)
#define UART_StopBits_2                     (1u << UARTx_CR1_STOP_Pos)
#define UART_StopBits_1_5                   (2u << UARTx_CR1_STOP_Pos)
#define IS_UART_STOPBITS(STOPBITS)          (((STOPBITS) == UART_StopBits_1) || \
                                             ((STOPBITS) == UART_StopBits_2) || \
                                             ((STOPBITS) == UART_StopBits_1_5))

/** @defgroup UART_Parity
  * @{
  */

#define UART_Parity_No                      (0u)
#define UART_Parity_Even                    (2u << UARTx_CR1_PARITY_Pos)
#define UART_Parity_Odd                     (3u << UARTx_CR1_PARITY_Pos)
#define IS_UART_PARITY(PARITY)              (((PARITY) == UART_Parity_No)   || \
                                             ((PARITY) == UART_Parity_Even) || \
                                             ((PARITY) == UART_Parity_Odd))

/** @defgroup UART_Transmit_Order
 * 
 * @brief  发送顺序
 */ 
#define UART_Transmit_Order_MSB             (1u << UARTx_CR1_MSBF_Pos)
#define UART_Transmit_Order_LSB             (0u)

/** @defgroup UART_Mode
  * @{
  */

#define UART_Mode_Tx                        ((uint16_t)0x0001)
#define UART_Mode_Rx                        ((uint16_t)0x0002)
#define IS_UART_MODE(MODE)                  ((((MODE) & (uint16_t)0xFFFC) == 0x00) && ((MODE) != (uint16_t)0x00))

/** @defgroup UART_Hardware_Flow_Control
  * @{
  */

#define UART_HardwareFlowControl_None       ((uint16_t)0x0000)
#define UART_HardwareFlowControl_RTS        ((uint16_t)0x0008)
#define UART_HardwareFlowControl_CTS        ((uint16_t)0x0004)
#define UART_HardwareFlowControl_RTS_CTS    ((uint16_t)0x000C)
#define IS_UART_HARDWARE_FLOW_CONTROL(CONTROL)\
                                             (((CONTROL) == UART_HardwareFlowControl_None) || \
                                             ((CONTROL) == UART_HardwareFlowControl_RTS)  || \
                                             ((CONTROL) == UART_HardwareFlowControl_CTS)  || \
                                             ((CONTROL) == UART_HardwareFlowControl_RTS_CTS))

/** @defgroup UART_Clock
  * @{
  */

#define UART_Clock_Disable                  ((uint16_t)0x0000)
#define UART_Clock_Enable                   ((uint16_t)0x0040)
#define IS_UART_CLOCK(CLOCK)                (((CLOCK) == UART_Clock_Disable) || \
                                             ((CLOCK) == UART_Clock_Enable))

/** @defgroup UART_Inversion_Pins
  * @{
  */

#define UART_InvPin_Tx                      ((uint16_t)0x0020)
#define UART_InvPin_Rx                      ((uint16_t)0x0010)
#define IS_UART_INVERSTION_PIN(INVPIN)      ((((INVPIN) & (uint16_t)0xFFCF) == 0x00) && ((INVPIN) != (uint16_t)0x00))

/** @defgroup UART_DMA_Requests
  * @{
  */

#define UART_DMAReq_Tx                      ((uint16_t)0x0080)
#define UART_DMAReq_Rx                      ((uint16_t)0x0040)
#define IS_UART_DMAREQ(DMAREQ)              ((((DMAREQ) & (uint16_t)0xFF3F) == 0x00) && ((DMAREQ) != (uint16_t)0x00))


/** @defgroup UART_Timer_Mode 
  * @{
  */
  
#define UART_TimerMode_Stop                (0U)
#define UART_TimerMode_TimeOut             (2U << UARTx_CR2_TIMCR_Pos)
#define UART_TimerMode_Idle                (3U << UARTx_CR2_TIMCR_Pos)
#define UART_TimerMode_AutoBaudRate1       (5U << UARTx_CR2_TIMCR_Pos)
#define UART_TimerMode_AutoBaudRate2       (6U << UARTx_CR2_TIMCR_Pos)
#define UART_TimerMode_GeneralTimer        (7U << UARTx_CR2_TIMCR_Pos)
#define IS_UART_TIMER_MODE(MODE)           (((MODE) == UART_TimerMode_Stop)           || \
                                            ((MODE) == UART_TimerMode_TimeOut)        ||\
                                            ((MODE) == UART_TimerMode_Idle)           || \
                                            ((MODE) == UART_TimerMode_AutoBaudRate1)  ||\
                                            ((MODE) == UART_TimerMode_AutoBaudRate2)  || \
                                            ((MODE) == UART_TimerMode_GeneralTimer))

/** @defgroup UART_Interrupt
  * @{
  */
#define UART_IT_RXMATCH                     ((uint16_t)0x1000)
#define UART_IT_ORE                         ((uint16_t)0x0800)
#define UART_IT_NE                          ((uint16_t)0x0400)
#define UART_IT_PE                          ((uint16_t)0x0200)
#define UART_IT_FE                          ((uint16_t)0x0100)
#define UART_IT_CTS                         ((uint16_t)0x0080)
#define UART_IT_TIMOV                       ((uint16_t)0x0040)
#define UART_IT_BAUD                        ((uint16_t)0x0020)
#define UART_IT_RXBRK                       ((uint16_t)0x0010)
#define UART_IT_RXIDLE                      ((uint16_t)0x0008)
#define UART_IT_RC                          ((uint16_t)0x0004)
#define UART_IT_TC                          ((uint16_t)0x0002)
#define UART_IT_TXE                         ((uint16_t)0x0001)
#define UART_IT_ALL                         (0x1FFF)
#define IS_UART_IT(IT) ((((IT) & (uint16_t)0xE000) == 0x00) && ((IT) != (uint16_t)0x00))
#define IS_UART_CLEAR_IT(IT) ((((IT) & (uint16_t)0xE000) == 0x00) && ((IT) != (uint16_t)0x00))

/** @defgroup UART_Flags
  * @{
  */

#define UART_FLAG_CTSLV                     ((uint16_t)0x8000)
#define UART_FLAG_TXBUSY                    ((uint16_t)0x4000)
#define UART_FLAG_SLVMATCH                  ((uint16_t)0x2000)
#define UART_FLAG_RXMATCH                   ((uint16_t)0x1000)
#define UART_FLAG_ORE                       ((uint16_t)0x0800)
#define UART_FLAG_NE                        ((uint16_t)0x0400)
#define UART_FLAG_PE                        ((uint16_t)0x0200)
#define UART_FLAG_FE                        ((uint16_t)0x0100)
#define UART_FLAG_CTS                       ((uint16_t)0x0080)
#define UART_FLAG_TIMOV                     ((uint16_t)0x0040)
#define UART_FLAG_BAUD                      ((uint16_t)0x0020)
#define UART_FLAG_RXBRK                     ((uint16_t)0x0010)
#define UART_FLAG_RXIDLE                    ((uint16_t)0x0008)
#define UART_FLAG_RC                        ((uint16_t)0x0004)
#define UART_FLAG_TC                        ((uint16_t)0x0002)
#define UART_FLAG_TXE                       ((uint16_t)0x0001)
#define IS_UART_FLAG(FLAG)                  ((((FLAG) & (uint16_t)0xF000) == 0x00) && ((FLAG) != (uint16_t)0x00))
#define IS_UART_CLEAR_FLAG(FLAG)            ((((FLAG) & (uint16_t)0xF1A1) == 0x00) && ((FLAG) != (uint16_t)0x00))

#define IS_UART_BAUDRATE(BAUDRATE)          (((BAUDRATE) > 0) && ((BAUDRATE) <= 0x00B71B00))
#define IS_UART_UCLKFREQ(UCLKFREQ)          (((UCLKFREQ) > 0) && ((UCLKFREQ) <= 0x05B8D800))
#define IS_UART_DATA_16BIT(DATA)            ((DATA) <= 0x1FF)
#define IS_UART_DATA_8BIT(DATA)             ((DATA) <= 0xFF)
#define IS_UART_ADDRESS(ADDRESS)            ((ADDRESS) <= 0xFF)
#define IS_UART_ADDRESSMSK(ADDRESSMSK)      ((ADDRESSMSK) <= 0xFF)
#define IS_UART_AUTORELOAD(AUTORELOAD)      ((AUTORELOAD) <= 0xFFFFFF)
#define IS_UART_ALL_PERIPH(PERIPH)          (((PERIPH) == CW_UART1) || \
                                             ((PERIPH) == CW_UART2) || \
                                             ((PERIPH) == CW_UART3))
                                            

/******************************************************************************
* Global variable definitions (declared in header file with 'extern')
******************************************************************************/
extern volatile boolean_t txComplete;
extern volatile boolean_t rxComplete;
extern volatile uint8_t *rxDataPtr;
extern volatile uint8_t *txDataPtr;
extern volatile uint32_t rxDataLen;
extern volatile uint32_t txDataLen;
extern volatile uint32_t rxDataCount;
extern volatile uint32_t txDataCount;
/******************************************************************************
* Global function prototypes
******************************************************************************/

/* 数据收发 */
void UART_SendData(UART_TypeDef* UARTx, uint16_t Data);
uint16_t UART_ReceiveData(UART_TypeDef* UARTx);
void UART_SendString(UART_TypeDef* UARTx, char* String);
UART_StatusTypeDef UART_TransmitPolling(UART_TypeDef* UARTx, uint8_t *Data, uint32_t length);
UART_StatusTypeDef UART_ReceivePolling(UART_TypeDef* UARTx, uint8_t *Data, uint32_t length);
UART_StatusTypeDef UART_TransmitInterrupt(UART_TypeDef* UARTx, uint8_t *Data, uint32_t length);
UART_StatusTypeDef UART_ReceiveInterrupt(UART_TypeDef* UARTx, uint8_t *Data, uint32_t length);
UART_StatusTypeDef UART_TransmitDMA(UART_TypeDef* UARTx, DMA_ChannelTypeDef DMACHx, uint8_t *Data, uint32_t length);
UART_StatusTypeDef UART_ReceiveDMA(UART_TypeDef* UARTx, DMA_ChannelTypeDef DMACHx, uint8_t *Data, uint32_t length);


void UART_SyncMode_SendDataPolling(UART_TypeDef* UARTx, uint8_t *Data, uint32_t length);
void UART_SyncMode_ReceiveDataPolling(UART_TypeDef* UARTx, uint8_t *Data, uint32_t length);

/* 初始化及配置*/
void UART_Init(UART_TypeDef* UARTx, UART_InitTypeDef* UART_InitStruct);
void UART_StructInit(UART_InitTypeDef* UART_InitStruct);
void UART_SyncMode_Init(UART_TypeDef* UARTx, UART_SyncModeInitTypeDef *UART_SyncModeInitStruct);
void UART_ClockInit(UART_TypeDef* UARTx, UART_ClockInitTypeDef* UART_ClockInitStruct);
void UART_ClockStructInit(UART_ClockInitTypeDef* UART_ClockInitStruct);
void UART1_DeInit(void);
void UART2_DeInit(void);
void UART3_DeInit(void);

/* 中断及标志 */
void UART_ITConfig(UART_TypeDef* UARTx, uint16_t UART_IT, FunctionalState NewState);
ITStatus UART_GetITStatus(UART_TypeDef* UARTx, uint16_t UART_IT);
void UART_ClearITPendingBit(UART_TypeDef* UARTx, uint16_t UART_IT);
FlagStatus UART_GetFlagStatus(UART_TypeDef* UARTx, uint16_t UART_FLAG);
void UART_ClearFlag(UART_TypeDef* UARTx, uint16_t UART_FLAG);

/* 发送和接收控制 */
void UART_DirectionModeCmd(UART_TypeDef* UARTx, uint16_t UART_DirectionMode, FunctionalState NewState);
void UART_InvPinCmd(UART_TypeDef* UARTx, uint16_t UART_InvPin, FunctionalState NewState);
void UART_PinSwapCmd(UART_TypeDef* UARTx, FunctionalState NewState);

/* 半双工通信 */
void UART_HalfDuplexCmd(UART_TypeDef* UARTx, FunctionalState NewState);

/* DMA */
void UART_DMACmd(UART_TypeDef* UARTx, uint16_t UART_DMAReq, FunctionalState NewState);

/* 多机通信 */
void UART_SetMultiMode(UART_TypeDef* UARTx, uint8_t UART_Address, uint8_t UART_AddressMsK);

/* LIN通信 */
void UART_LINCmd(UART_TypeDef* UARTx, FunctionalState NewState);
void UART_LINSendBreak(UART_TypeDef* UARTx, UART_LINBreakSendLengthTypeDef BreakLength);
void UART_LINBreakDetectLengthConfig(UART_TypeDef* UARTx, UART_LINBreakDetectLengthTypeDef UART_LINBreakDetectLength);

/* 定时器工作模式 */
void UART_TimerModeConfig(UART_TypeDef* UARTx, uint16_t UART_TimerMode);
void UART_SetAutoReload(UART_TypeDef* UARTx, uint32_t AutoReload);
uint32_t UART_GetCounter(UART_TypeDef* UARTx);

/* 自动波特率检测 */
void UART_EnableAutoBaudRateDetection(UART_TypeDef* UARTx,UART_AutoBaudRateModeTypeDef AutoBaudRateMode);
void UART_DisableAutoBaudRateDetection(UART_TypeDef* UARTx);
boolean_t UART_AutoBaudRateDetectionComplete(UART_TypeDef* UARTx);
uint32_t UART_GetDetectedBaudRate(UART_TypeDef* UARTx);

/* 数据匹配功能 */
void  UART_EnableDataMatch(UART_TypeDef* UARTx);
void  UART_DisableDataMatch(UART_TypeDef* UARTx);
void  UART_SetDataMatch(UART_TypeDef* UARTx, uint16_t DataMatch);
boolean_t UART_GetDataMatchFlag(UART_TypeDef* UARTx);


#ifdef __cplusplus
}
#endif

#endif /*__CW32L012_UART_H */
/**
  * @}
  */











