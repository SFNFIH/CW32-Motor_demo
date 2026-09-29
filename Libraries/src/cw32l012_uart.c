/**
 * @file cw32l012_uart.c
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

/* Includes ------------------------------------------------------------------*/
#include "cw32l012_uart.h"

/* Private_TypesDefinitions --------------------------------------------------*/

/* Private_Defines -----------------------------------------------------------*/

/* Private_Variables ---------------------------------------------------------*/
volatile boolean_t txComplete = TRUE;
volatile boolean_t rxComplete = TRUE;
volatile uint8_t *rxDataPtr;
volatile uint8_t *txDataPtr;
volatile uint32_t rxDataLen;
volatile uint32_t txDataLen;
/* Private_FunctionPrototypes ------------------------------------------------*/

/* Private_Functions ---------------------------------------------------------*/

/* Public_Functions ----------------------------------------------------------*/

/**
 * @brief 通过UARTx发送一个数据(8bit/9bit)
 *
 * @param UARTx :UARTx外设
 *        参数可以是:
 *           CW_UART1、CW_UART2、CW_UART3
 * @param Data :待发送的数据
 */
void UART_SendData(UART_TypeDef* UARTx, uint16_t Data)
{
    assert_param(IS_UART_ALL_PERIPH(UARTx));    

    UARTx->TDR = Data;
}

/**
 * @brief 发送字符串
 *
 * @param UARTx :UARTx外设
 *        参数可以是:
 *           CW_UART1、CW_UART2、CW_UART3
 * @param String :待发送的字符串
 */
void UART_SendString(UART_TypeDef* UARTx, char* String)
{
    assert_param(IS_UART_ALL_PERIPH(UARTx));
    
    while (*String != '\0')
    {
        UARTx->TDR = *String;        
        while (!UARTx->ISR_f.TXE);
        String++;
    }
    while (UARTx->ISR_f.TXBUSY);
}

/**
 * @brief 通过UARTx接收一个数据(8bit/9bit)
 *
 * @param UARTx :UARTx外设
 *        参数可以是:
 *           CW_UART1、CW_UART2、CW_UART3
 * @return uint16_t :接收的数据
 */
uint16_t UART_ReceiveData(UART_TypeDef* UARTx)
{
    assert_param(IS_UART_ALL_PERIPH(UARTx));

    return UARTx->RDR;
}

/**
 * @brief 发送一组数据
 *
 * @param UARTx :UARTx外设
 *        参数可以是: CW_UART1、CW_UART2、CW_UART3
 * @param String :发送的字符串
 * @param length :发送字符串的长度
 */
UART_StatusTypeDef UART_TransmitPolling(UART_TypeDef* UARTx, uint8_t *Data, uint32_t length)
{
    UART_StatusTypeDef status = UART_STATUS_OK;
    // 检查传入的UART外设参数是否合法
    assert_param(IS_UART_ALL_PERIPH(UARTx));

    // 检查传入的数据指针是否为空
    if (Data == NULL)
    {
        return UART_STATUS_ERROR; 
    }

    if (UARTx->ISR_f.TXBUSY)
    {
        return UART_STATUS_BUSY;
    }
    
    UARTx->CR1_f.TXEN = 1;
    while (length > 0)
    {
        UARTx->TDR = *Data;
        while (!UARTx->ISR_f.TXE);
        Data++;
        length--;
    }
    while (UARTx->ISR_f.TXBUSY);
    UARTx->CR1_f.TXEN = 0;
    return status;
}

UART_StatusTypeDef UART_ReceivePolling(UART_TypeDef* UARTx, uint8_t *Data, uint32_t length)
{
    UART_StatusTypeDef status = UART_STATUS_OK;
    // 检查传入的UART外设参数是否合法
    assert_param(IS_UART_ALL_PERIPH(UARTx));

    // 检查传入的数据指针是否为空
    if (Data == NULL)
    {
        return UART_STATUS_ERROR;
    }
    
    UARTx->ICR_f.RC = 0;
    UARTx->CR1_f.RXEN = 1;
    while (length > 0)
    {
        while (!UARTx->ISR_f.RC);        
        *Data = UARTx->RDR;
        Data++;
        length--;
        UARTx->ICR_f.RC = 0;
    }
    UARTx->CR1_f.RXEN = 0;
    return status;
}

/**
 * @brief 通过UARTx以中断方式发送数据
 * 
 * @param UARTx : UARTx外设，参数可以是 CW_UART1、CW_UART2、CW_UART3 等
 * @param Data : 待发送数据的指针
 * @param length : 待发送数据的个数
 * @note  中断服务函数中需要完成数据的再次发送和标志位的清除
 */
UART_StatusTypeDef UART_TransmitInterrupt(UART_TypeDef* UARTx, uint8_t *Data, uint32_t length)
{
    UART_StatusTypeDef status = UART_STATUS_OK;
    // 检查传入的UART外设参数是否合法
    assert_param(IS_UART_ALL_PERIPH(UARTx));
    
    // 检查传入的数据指针是否为空
    if (Data == NULL)
    {
        return UART_STATUS_ERROR;
    }

    if (UARTx->ISR_f.TXBUSY)
    {
        return UART_STATUS_BUSY;
    }

    if (txComplete == FALSE)
    {
        return UART_STATUS_BUSY;
    }
    txDataPtr = Data;
    txDataLen = length;
    txComplete = FALSE;
    UARTx->ICR_f.TC = 1;    
    UARTx->IER_f.TC = 1;
    UARTx->CR1_f.TXEN = 1;
    UARTx->TDR = *txDataPtr;
    txDataLen--;
    return status;
}

/**
 * @brief 通过UARTx以中断方式接收数据
 *
 * @param UARTx : UARTx外设，参数可以是 CW_UART1、CW_UART2、CW_UART3 等
 * @param Data : 待接收数据的指针
 * @param length : 待接收数据的个数
 * @note  中断服务函数中需要完成数据的接收和标志位的清除
 */
UART_StatusTypeDef UART_ReceiveInterrupt(UART_TypeDef* UARTx, uint8_t *Data, uint32_t length)
{
    UART_StatusTypeDef status = UART_STATUS_OK;
    // 检查传入的UART外设参数是否合法
    assert_param(IS_UART_ALL_PERIPH(UARTx));

    // 检查传入的数据指针是否为空
    if (Data == NULL)
    {
        return UART_STATUS_ERROR; 
    } 

    if (rxComplete == FALSE)
    {
        return UART_STATUS_BUSY;
    }
    if (length  == 0)
    {
        return UART_STATUS_ERROR;
    }
    rxDataPtr = Data;
    rxDataLen = length;
    rxComplete = FALSE;
    UARTx->ICR_f.RC = 1;
    UARTx->IER_f.RC = 1;
    UARTx->CR1_f.RXEN = 1;
    return status;
} 

/**
 * @brief 通过UARTx以DMA方式发送数据
 *
 * @param UARTx : UARTx外设，参数可以是 CW_UART1、CW_UART2、CW_UART3 等
 * @param DMACHx : DMA通道，参数可以是 DMA_CHANNEL_1、DMA_CHANNEL_2、DMA_CHANNEL_3、DMA_CHANNEL_4 等
 * @param Data : 待发送数据的指针
 * @param length : 待发送数据的个数
 */
UART_StatusTypeDef UART_TransmitDMA(UART_TypeDef* UARTx, DMA_ChannelTypeDef DMACHx, uint8_t *Data, uint32_t length)
{
    UART_StatusTypeDef status = UART_STATUS_OK;
    // 检查传入的UART外设参数是否合法
    assert_param(IS_UART_ALL_PERIPH(UARTx));

    // 检查传入的数据指针是否为空
    if (Data == NULL)
    {
        return UART_STATUS_ERROR;
    }

    if (UARTx->ISR_f.TXBUSY)         // 检查UART是否忙
    {
        return UART_STATUS_BUSY;
    }

    CW_SYSCTRL->AHBEN |= 0x5a5a0000 | SYSCTRL_AHBEN_DMA_Msk;    // 使能DMA时钟
    CW_SYSCTRL->AHBRST_f.DMA = 0;                             // 复位DMA
    CW_SYSCTRL->AHBRST_f.DMA = 1;                             // 使能DMA

    *DMA_CNTy(DMACHx) = length | 1 << DMA_CNT1_REPEAT_Pos ;          // 设置DMA传输的数据长度
    *DMA_SRCADDRy(DMACHx) = (uint32_t)Data;                    // 设置DMA源地址
    *DMA_DSTADDRy(DMACHx) = (uint32_t)&UARTx->TDR;             // 设置DMA目的地址

    switch ((uint32_t)UARTx)
    {
        case (uint32_t)CW_UART1:
            *DMA_TRIGy(DMACHx) = DMA_TRIGGER_SRC_UART1_TX << DMACHx_TRIG_HARDSRC_Pos | DMA_TRIGGER_HARDWARE << DMACHx_TRIG_TYPE_Pos;             // 设置DMA触发源
            break;
        case (uint32_t)CW_UART2:
            *DMA_TRIGy(DMACHx) = DMA_TRIGGER_SRC_UART2_TX << DMACHx_TRIG_HARDSRC_Pos | DMA_TRIGGER_HARDWARE << DMACHx_TRIG_TYPE_Pos;             // 设置DMA触发源
            break;
        case (uint32_t)CW_UART3:
            *DMA_TRIGy(DMACHx) = DMA_TRIGGER_SRC_UART3_TX << DMACHx_TRIG_HARDSRC_Pos | DMA_TRIGGER_HARDWARE << DMACHx_TRIG_TYPE_Pos;             // 设置DMA触发源
            break;
    }
    *DMA_CSRy(DMACHx) = DISABLE << DMACHx_CSR_DSTLOAD_Pos | DISABLE << DMACHx_CSR_SRCLOAD_Pos |\
                        DISABLE << DMACHx_CSR_RESTART_Pos | DMA_DATA_SIZE_8BITS << DMACHx_CSR_SIZE_Pos |\
                        DMA_ADDRESS_FIXED << DMACHx_CSR_DSTINC_Pos | DMA_ADDRESS_INCREMENT << DMACHx_CSR_SRCINC_Pos |\
                        DMA_MODE_BLOCK << DMACHx_CSR_TRANS_Pos | ENABLE << DMACHx_CSR_EN_Pos; // 配置DMA通道

    UARTx->CR1_f.TXEN = 1;            // 使能UART发送
    UARTx->CR2_f.DMATX = 1;         // 使能UART DMA发送
    return status;
}

/**
 * @brief 通过UARTx以DMA方式接收数据
 *
 * @param UARTx : UARTx外设，参数可以是 CW_UART1、CW_UART2、CW_UART3 等
 * @param DMACHx : DMA通道，参数可以是 DMA_CHANNEL_1、DMA_CHANNEL_2、DMA_CHANNEL_3、DMA_CHANNEL_4 等
 * @param Data : 待接收数据的指针
 * @param length : 待接收数据的个数
 */
UART_StatusTypeDef UART_ReceiveDMA(UART_TypeDef* UARTx, DMA_ChannelTypeDef DMACHx, uint8_t *Data, uint32_t length)
{
    UART_StatusTypeDef status = UART_STATUS_OK;
    // 检查传入的UART外设参数是否合法
    assert_param(IS_UART_ALL_PERIPH(UARTx));

    // 检查传入的数据指针是否为空
    if (Data == NULL)
    {
        return UART_STATUS_ERROR;
    }    

    CW_SYSCTRL->AHBEN |= 0x5a5a0000 | SYSCTRL_AHBEN_DMA_Msk;    // 使能DMA时钟
    CW_SYSCTRL->AHBRST_f.DMA = 0;                             // 复位DMA
    CW_SYSCTRL->AHBRST_f.DMA = 1;                             // 使能DMA
    *DMA_CNTy(DMACHx) = length | 1 << DMA_CNT1_REPEAT_Pos ;          // 设置DMA传输的数据长度
    *DMA_SRCADDRy(DMACHx) = (uint32_t)&UARTx->RDR;                    // 设置DMA源地址
    *DMA_DSTADDRy(DMACHx) = (uint32_t)Data;             // 设置DMA目的地址

    switch ((uint32_t)UARTx)
    {
        case (uint32_t)CW_UART1:
            *DMA_TRIGy(DMACHx) = DMA_TRIGGER_SRC_UART1_RX << DMACHx_TRIG_HARDSRC_Pos | DMA_TRIGGER_HARDWARE << DMACHx_TRIG_TYPE_Pos;             // 设置DMA触发源
            break;
        case (uint32_t)CW_UART2:
            *DMA_TRIGy(DMACHx) = DMA_TRIGGER_SRC_UART2_RX << DMACHx_TRIG_HARDSRC_Pos | DMA_TRIGGER_HARDWARE << DMACHx_TRIG_TYPE_Pos;             // 设置DMA触发源
            break;
        case (uint32_t)CW_UART3:
            *DMA_TRIGy(DMACHx) = DMA_TRIGGER_SRC_UART3_RX << DMACHx_TRIG_HARDSRC_Pos | DMA_TRIGGER_HARDWARE << DMACHx_TRIG_TYPE_Pos;             // 设置DMA触发源
            break;             
    }
    *DMA_CSRy(DMACHx) = DISABLE << DMACHx_CSR_DSTLOAD_Pos | DISABLE << DMACHx_CSR_SRCLOAD_Pos |\
                        DISABLE << DMACHx_CSR_RESTART_Pos | DMA_DATA_SIZE_8BITS << DMACHx_CSR_SIZE_Pos |\
                        DMA_ADDRESS_INCREMENT << DMACHx_CSR_DSTINC_Pos | DMA_ADDRESS_FIXED << DMACHx_CSR_SRCINC_Pos |\
                        DMA_MODE_BLOCK << DMACHx_CSR_TRANS_Pos | ENABLE << DMACHx_CSR_EN_Pos; // 配置DMA通道

    UARTx->CR1_f.RXEN = 1;            // 使能UART接收
    UARTx->CR2_f.DMARX = 1;         // 使能UART DMA接收
    return status;
}


/**
 * @brief 在同步模式下通过UARTx发送数据(8bit)
 * @param UARTx :UARTx外设
 * @param Data :待发送的数据的指针
 * @param length :待发送的数据的个数
 */
void UART_SyncMode_SendDataPolling(UART_TypeDef* UARTx, uint8_t *Data, uint32_t length)
{
    uint8_t *ptr = Data;
    assert_param(IS_UART_ALL_PERIPH(UARTx));

    UARTx->ICR = 0;

    UARTx->CR1_f.TXEN = 1;

    while (length)
    {        
        UARTx->TDR_f.TDR = *ptr;
        ptr++;
        length--;
        while (UARTx->ISR_f.TXE == 0);
    }
    while (UARTx->ISR_f.TXBUSY == 1);
    UARTx->CR1_f.TXEN = 0;
}

/**
 * @brief 在同步模式下通过UARTx接收数据(8bit)
 * @param UARTx :UARTx外设
 * @param Data :待接收的数据缓存的指针
 * @param length :待接收的数据的个数
 * @return uint8_t :接收的数据
 */
void UART_SyncMode_ReceiveDataPolling(UART_TypeDef* UARTx, uint8_t *Data, uint32_t length)
{
    uint8_t *ptr = Data;
    assert_param(IS_UART_ALL_PERIPH(UARTx));

    UARTx->ICR_f.RC = 0;

    UARTx->CR1_f.RXEN = 1;

    while (length)
    {
        while (UARTx->ISR_f.RC == 0);
        *ptr = UARTx->RDR;
        ptr++;
        length--;
        UARTx->ICR_f.RC = 0;
    }
    UARTx->CR1_f.RXEN = 0;
}


/**
 * @brief 根据UART_InitStruct中指定的参数初始化UARTx外设
 *
 * @param UARTx :UARTx外设
 *        参数可以是:
 *           CW_UART1、CW_UART2、CW_UART3
 * @param UART_InitStruct :指向UART_InitTypeDef类型结构体的指针
 */
void UART_Init(UART_TypeDef* UARTx, UART_InitTypeDef* UART_InitStruct)
{
    uint16_t integerdivider = 0x00;
    uint16_t fractionaldivider = 0x00;
    float32_t temp = 0;

    assert_param(IS_UART_ALL_PERIPH(UARTx));
    assert_param(IS_UART_BAUDRATE(UART_InitStruct->UART_BaudRate));
    assert_param(IS_UART_Over(UART_InitStruct->UART_Over));
    assert_param(IS_UART_Source(UART_InitStruct->UART_Source));
    assert_param(IS_UART_UCLKFREQ(UART_InitStruct->UART_UclkFreq));
    assert_param(IS_UART_StartBit(UART_InitStruct->UART_StartBit));
    assert_param(IS_UART_STOPBITS(UART_InitStruct->UART_StopBits));
    assert_param(IS_UART_PARITY(UART_InitStruct->UART_Parity));
    assert_param(IS_UART_MODE(UART_InitStruct->UART_Mode));
    assert_param(IS_UART_HARDWARE_FLOW_CONTROL(UART_InitStruct->UART_HardwareFlowControl));

    switch ((uint32_t)UARTx)
    {
    case (uint32_t)CW_UART1:
        CW_SYSCTRL->APBEN1 = (CW_SYSCTRL->APBEN1 & 0x0000FFFF) | 0x5a5a0000 | SYSCTRL_APBEN1_UART1_Msk;
        break;
    case (uint32_t)CW_UART2:
        CW_SYSCTRL->APBEN1 = (CW_SYSCTRL->APBEN1 & 0x0000FFFF) | 0x5a5a0000 | SYSCTRL_APBEN1_UART2_Msk;
        break;
    case (uint32_t)CW_UART3:
        CW_SYSCTRL->APBEN1 = (CW_SYSCTRL->APBEN1 & 0x0000FFFF) | 0x5a5a0000 | SYSCTRL_APBEN1_UART3_Msk;
        break;
    default:
        break;
    }

    /* UARTx_CR1 */
    REGBITS_MODIFY(UARTx->CR1, UARTx_CR1_SOURCE_Msk | UARTx_CR1_OVER_Msk | UARTx_CR1_START_Msk | \
                   UARTx_CR1_STOP_Msk | UARTx_CR1_PARITY_Msk | \
                   UARTx_CR1_RXEN_Msk | UARTx_CR1_TXEN_Msk    \
                   , \
                   UART_InitStruct->UART_Source | \
                   (uint32_t)UART_InitStruct->UART_Over | \
                   UART_InitStruct->UART_StartBit       | \
                   UART_InitStruct->UART_StopBits       | \
                   UART_InitStruct->UART_Parity         | \
                   UART_InitStruct->UART_Mode);

    /* UARTx_CR2 */
    REGBITS_MODIFY(UARTx->CR2,  UARTx_CR2_RTSEN_Msk | \
                   UARTx_CR2_CTSEN_Msk  \
                   , \
                   UART_InitStruct->UART_HardwareFlowControl);
    /* 校验位 */
    if (UART_InitStruct->UART_Parity != UART_Parity_No)
    {
        UARTx->CR1_f.CHLEN = 1;
    }
    else
    {
        UARTx->CR1_f.CHLEN = 0;
    }

    /* 波特率设置 */
    if (UART_Over_16 == UART_InitStruct->UART_Over)
    {
        /* 16倍采样 */
        temp = (float32_t)(UART_InitStruct->UART_UclkFreq) / (16 * (UART_InitStruct->UART_BaudRate));
        integerdivider = (uint16_t)temp;
        fractionaldivider = (uint16_t)((temp - integerdivider) * 16 + 0.5);
        UARTx->BRRI = (uint16_t)integerdivider;
        UARTx->BRRF = (uint16_t)fractionaldivider;
    }
    else if (UART_Over_8 == UART_InitStruct->UART_Over)
    {
        /* 8倍采样 */
        integerdivider = (uint16_t)((float32_t)(UART_InitStruct->UART_UclkFreq) / (8 * (UART_InitStruct->UART_BaudRate)) + 0.5);
        UARTx->BRRI = (uint16_t)integerdivider;
    }
    else if (UART_Over_4 == UART_InitStruct->UART_Over)
    {
        /* 4倍采样 */
        integerdivider = (uint16_t)((float32_t)(UART_InitStruct->UART_UclkFreq) / (4 * (UART_InitStruct->UART_BaudRate)) + 0.5);
        UARTx->BRRI = (uint16_t)integerdivider;
    }
    else
    {
        /* 专用采样 */
        /* 专用采样仅适合传输时钟源为LSE或者LSI时，进行2400bps、4800bps或9600bps波特率下的UART通信 */
        integerdivider = (uint16_t)((float32_t)(UART_InitStruct->UART_UclkFreq) * 256 / (UART_InitStruct->UART_BaudRate) + 0.5);
        UARTx->BRRI = (uint16_t)integerdivider;
    }
}

/**
 * @brief 默认值赋值UART_InitStruct结构体成员
 *
 * @param UART_InitStruct :指向UART_InitTypeDef类型结构体的指针
 */
void UART_StructInit(UART_InitTypeDef* UART_InitStruct)
{
    /* UART_InitStruct members default value */
    UART_InitStruct->UART_BaudRate = 9600;
    UART_InitStruct->UART_Over = UART_Over_16;
    UART_InitStruct->UART_Source = UART_Source_PCLK;
    UART_InitStruct->UART_StartBit = UART_StartBit_FE;
    UART_InitStruct->UART_StopBits = UART_StopBits_1;
    UART_InitStruct->UART_Parity = UART_Parity_No ;
    UART_InitStruct->UART_Mode = UART_Mode_Rx | UART_Mode_Tx;
    UART_InitStruct->UART_HardwareFlowControl = UART_HardwareFlowControl_None;
}

/**
 * @brief UARTx同步模式初始化，CPOL=1、CPHA=1，只能做主机
 *
 * @param UARTx :UARTx外设
 *        参数可以是:
 *           CW_UART1、CW_UART2、CW_UART3
 * @param UART_ClockInitStruct :指向UART_ClockInitTypeDef类型结构体的指针
 */
void UART_ClockInit(UART_TypeDef* UARTx, UART_ClockInitTypeDef* UART_ClockInitStruct)
{
    assert_param(IS_UART_ALL_PERIPH(UARTx));
    assert_param(IS_UART_CLOCK(UART_ClockInitStruct->UART_Clock));
    assert_param(IS_UART_Source(UART_ClockInitStruct->UART_Source));

    REGBITS_MODIFY(UARTx->CR1, UARTx_CR1_SYNC_Msk | UARTx_CR1_SOURCE_Msk, \
                   (uint32_t)UART_ClockInitStruct->UART_Clock | \
                   (uint32_t)UART_ClockInitStruct->UART_Source);
}

/**
 * @brief 默认值赋值UART_ClockInitStruct结构体成员
 *
 * @param UART_ClockInitStruct :指向UART_ClockInitTypeDef类型结构体的指针
 */
void UART_ClockStructInit(UART_ClockInitTypeDef* UART_ClockInitStruct)
{
    /* UART_ClockInitStruct members default value */
    UART_ClockInitStruct->UART_Clock = UART_Clock_Disable;
    UART_ClockInitStruct->UART_Source = UART_Source_PCLK;
}

/**
 * @brief UARTx同步模式初始化，CPOL=1、CPHA=1，只能做主机
 *
 * @param UARTx :UARTx外设
 *        参数可以是:
 *           CW_UART1、CW_UART2、CW_UART3
 * @param UART_SyncModeInitStruct :指向UART_SyncModeInitTypeDef类型结构体的指针
 */
void UART_SyncMode_Init(UART_TypeDef* UARTx, UART_SyncModeInitTypeDef *UART_SyncModeInitStruct)
{
    uint32_t BRRI;

    if (UART_SyncModeInitStruct->UART_BaudRate == 0)
    {
        return;
    }
    BRRI = UART_SyncModeInitStruct->UART_ClockFreq / (UART_SyncModeInitStruct->UART_BaudRate << 1);
    if (BRRI > 0xFF)
    {
        BRRI = 0xFF;    // 低8位有效
    }
    // 打开UART时钟
    switch ((uint32_t)UARTx)
    {
    case (uint32_t)CW_UART1:
        CW_SYSCTRL->APBEN1 = (CW_SYSCTRL->APBEN1 & 0x0000FFFF) | 0x5a5a0000 | SYSCTRL_APBEN1_UART1_Msk;
        break;
    case (uint32_t)CW_UART2:
        CW_SYSCTRL->APBEN1 = (CW_SYSCTRL->APBEN1 & 0x0000FFFF) | 0x5a5a0000 | SYSCTRL_APBEN1_UART2_Msk;
        break;
    case (uint32_t)CW_UART3:
        CW_SYSCTRL->APBEN1 = (CW_SYSCTRL->APBEN1 & 0x0000FFFF) | 0x5a5a0000 | SYSCTRL_APBEN1_UART3_Msk;
        break;
    default:
        break;
    }

    //
    UARTx->CR1 = UARTx_CR1_SYNC_Msk | \
                 UART_SyncModeInitStruct->UART_ClockSource | \
                 UART_SyncModeInitStruct->UART_TransmitOrder;
    UARTx->CR2_f.ADDREN = 0;
    UARTx->BRRI = BRRI;
}


/**
 * @brief UART1去初始化
 *
 */
void UART1_DeInit(void)
{
    CW_SYSCTRL->APBRST1_f.UART1 = 0;
    CW_SYSCTRL->APBRST1_f.UART1 = 1;
}

/**
 * @brief UART2去初始化
 *
 */
void UART2_DeInit(void)
{
    CW_SYSCTRL->APBRST1_f.UART2 = 0;
    CW_SYSCTRL->APBRST1_f.UART2 = 1;
}

/**
 * @brief UART3去初始化
 *
 */
void UART3_DeInit(void)
{
    CW_SYSCTRL->APBRST1_f.UART3 = 0;
    CW_SYSCTRL->APBRST1_f.UART3 = 1;
}


/**
 * @brief 使能/失能UARTx中断
 *
 * @param UARTx :UARTx外设
 *        参数可以是:
 *           CW_UART1、CW_UART2、CW_UART3
 * @param UART_IT :中断源
 *   参数可以是:
 *     @arg UART_IT_RXMATCH:  接收匹配中断
 *     @arg UART_IT_ORE:     溢出错误中断
 *     @arg UART_IT_NE:      噪声错误中断
 *     @arg UART_IT_PE:    奇偶校验错误中断
 *     @arg UART_IT_FE:    帧结构错误中断
 *     @arg UART_IT_CTS:   CTS信号变化中断
 *     @arg UART_IT_TIMOV: 定时器溢出中断
 *     @arg UART_IT_BAUD:  自动波特率检测完成中断
 *     @arg UART_IT_RXBRK: 间隔段接收完成中断
 *     @arg UART_IT_RXIDLE: 接收空闲中断
 *     @arg UART_IT_RC:    接收完成中断
 *     @arg UART_IT_TC:    发送完成中断
 *     @arg UART_IT_TXE:   发送缓冲空中断
 * @param NewState :ENABLE or DISABLE
 */
void UART_ITConfig(UART_TypeDef* UARTx, uint16_t UART_IT, FunctionalState NewState)
{
    assert_param(IS_UART_ALL_PERIPH(UARTx));
    assert_param(IS_UART_IT(UART_IT));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    NewState ? (UARTx->IER |= UART_IT) : (UARTx->IER &= (uint16_t)~UART_IT);

    // if (NewState != DISABLE)
    // {
    //   UARTx->IER |= UART_IT;
    // }
    // else
    // {
    //   UARTx->IER &= (uint16_t)~UART_IT;
    // }
}

/**
 * @brief 获取UARTx中断标志位
 *
 * @param UARTx :UARTx外设
 *        参数可以是:
 *           CW_UART1、CW_UART2、CW_UART3
 * @param UART_IT :中断源
 *   参数可以是：
 *     @arg UART_IT_RXMATCH:  接收匹配中断
 *     @arg UART_IT_ORE:     溢出错误中断
 *     @arg UART_IT_NE:      噪声错误中断
 *     @arg UART_IT_PE:    奇偶校验错误中断
 *     @arg UART_IT_FE:    帧结构错误中断
 *     @arg UART_IT_CTS:   CTS信号变化中断
 *     @arg UART_IT_TIMOV: 定时器溢出中断
 *     @arg UART_IT_BAUD:  自动波特率检测完成中断
 *     @arg UART_IT_RXBRK: 间隔段接收完成中断
 *     @arg UART_IT_RXIDLE: 接收空闲中断
 *     @arg UART_IT_RC:    接收完成中断
 *     @arg UART_IT_TC:    发送完成中断
 *     @arg UART_IT_TXE:   发送缓冲空中断
 * @return ITStatus :UART_IT当前状态 (SET or RESET)
 */
ITStatus UART_GetITStatus(UART_TypeDef* UARTx, uint16_t UART_IT)
{
    assert_param(IS_UART_ALL_PERIPH(UARTx));
    assert_param(IS_UART_IT(UART_IT));

    return (REGBITS_GET(UARTx->ISR, UART_IT) ? SET : RESET);
}

/**
 * @brief 清除UARTx中断标志位
 *
 * @param UARTx :UARTx外设
 *        参数可以是:
 *           CW_UART1、CW_UART2、CW_UART3
 * @param UART_IT :中断源
 *   参数可以是：
 *     @arg UART_IT_RXMATCH:  接收匹配中断
 *     @arg UART_IT_ORE:     溢出错误中断
 *     @arg UART_IT_NE:      噪声错误中断
 *     @arg UART_IT_PE:    奇偶校验错误中断
 *     @arg UART_IT_FE:    帧结构错误中断
 *     @arg UART_IT_CTS:   CTS信号变化中断
 *     @arg UART_IT_TIMOV: 定时器溢出中断
 *     @arg UART_IT_BAUD:  自动波特率检测完成中断
 *     @arg UART_IT_RXBRK: 间隔段接收完成中断
 *     @arg UART_IT_RXIDLE: 接收空闲中断
 *     @arg UART_IT_RC:    接收完成中断
 *     @arg UART_IT_TC:    发送完成中断
 *     @arg UART_IT_TXE:   发送缓冲空中断
 * @note
 *    -TXE中断标志位只能通过写UARTx_TDR寄存器来清除
 *    -RC中断标志位只能软件清除，不能通过读UARTx_RDR寄存器来清除
 */
void UART_ClearITPendingBit(UART_TypeDef* UARTx, uint16_t UART_IT)
{
    assert_param(IS_UART_ALL_PERIPH(UARTx));
    assert_param(IS_UART_CLEAR_IT(UART_IT));

    REGBITS_CLR(UARTx->ICR, UART_IT);
}

/**
 * @brief 获取UARTx标志位
 *
 * @param UARTx :UARTx外设
 *        参数可以是:
 *           CW_UART1、CW_UART2、CW_UART3
 * @param  UART_FLAG :标志
 *   参数可以是：
 *     @arg UART_FLAG_RXBRK:   间隔段接收完成中断
 *     @arg UART_FLAG_BAUD:    自动波特率检测完成中断
 *     @arg UART_FLAG_TIMOV:   定时器溢出中断
 *     @arg UART_FLAG_TXBUSY:  发送忙标志
 *     @arg UART_FLAG_CTSLV:   CTS信号电平状态标志
 *     @arg UART_FLAG_CTS:     CTS信号变化标志
 *     @arg UART_FLAG_RXMATCH:   从机地址匹配标志
 *     @arg UART_FLAG_PE:      奇偶校验错误标志
 *     @arg UART_FLAG_FE:      帧结构错误标志
 *     @arg UART_FLAG_RC:      接收完成标志
 *     @arg UART_FLAG_TC:      发送完成标志
 *     @arg UART_FLAG_TXE:     发送缓冲器空标志
 * @return FlagStatus :UART_FLAG当前状态 (SET or RESET)
 */
FlagStatus UART_GetFlagStatus(UART_TypeDef* UARTx, uint16_t UART_FLAG)
{
    assert_param(IS_UART_ALL_PERIPH(UARTx));
    assert_param(IS_UART_FLAG(UART_FLAG));

    return (REGBITS_GET(UARTx->ISR, UART_FLAG) ? SET : RESET);
}

/**
 * @brief 清除UARTx标志位
 *
 * @param UARTx :UARTx外设
 *        参数可以是:
 *           CW_UART1、CW_UART2、CW_UART3
 * @param  UART_FLAG :标志
 *   参数可以是：
 *     @arg UART_FLAG_RXBRK:   间隔段接收完成中断
 *     @arg UART_FLAG_BAUD:    自动波特率检测完成中断
 *     @arg UART_FLAG_TIMOV:   定时器溢出中断
 *     @arg UART_FLAG_TXBUSY:  发送忙标志
 *     @arg UART_FLAG_CTSLV:   CTS信号电平状态标志
 *     @arg UART_FLAG_CTS:     CTS信号变化标志
 *     @arg UART_FLAG_RXMATCH:   从机地址匹配标志
 *     @arg UART_FLAG_PE:      奇偶校验错误标志
 *     @arg UART_FLAG_FE:      帧结构错误标志
 *     @arg UART_FLAG_RC:      接收完成标志
 *     @arg UART_FLAG_TC:      发送完成标志
 *     @arg UART_FLAG_TXE:     发送缓冲器空标志
 * @note
 *    -TXBUSY、CTSLV、SLVMATCH标志位由硬件自动设置和清除
 *    -TXE标志位只能通过写UARTx_TDR寄存器来清除
 *    -RC标志位只能软件清除，不能通过读UARTx_RDR寄存器来清除
 */
void UART_ClearFlag(UART_TypeDef* UARTx, uint16_t UART_FLAG)
{
    assert_param(IS_UART_ALL_PERIPH(UARTx));
    assert_param(IS_UART_CLEAR_FLAG(UART_FLAG));

    REGBITS_CLR(UARTx->ICR, UART_FLAG);
}

/**
 * @brief 使能/失能TXD输出、RXD输入
 *
 * @param UARTx :UARTx外设
 *        参数可以是:
 *           CW_UART1、CW_UART2、CW_UART3
 * @param UART_DirectionMode :传输方向
 *   参数可以是：
 *     @arg UART_Mode_Tx: TXD输出
 *     @arg UART_Mode_Rx: RXD输入
 * @param NewState :ENABLE or DISABLE
 */
void UART_DirectionModeCmd(UART_TypeDef* UARTx, uint16_t UART_DirectionMode, FunctionalState NewState)
{
    assert_param(IS_UART_ALL_PERIPH(UARTx));
    assert_param(IS_UART_MODE(UART_DirectionMode));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    NewState ? (UARTx->CR1 |= UART_DirectionMode) : (UARTx->CR1 &= (uint16_t)~UART_DirectionMode);

    // if (NewState != DISABLE)
    // {
    //   UARTx->CR1 |= UART_DirectionMode;
    // }
    // else
    // {
    //   UARTx->CR1 &= (uint16_t)~UART_DirectionMode;
    // }
}

/**
 * @brief 使能/失能TXD输出信号、RXD输入信号反相
 *
 * @param UARTx :UARTx外设
 *        参数可以是:
 *           CW_UART1、CW_UART2、CW_UART3
 * @param UART_InvPin :反相引脚
 *   参数可以是：
 *     @arg UART_InvPin_Tx: TXD引脚输出信号反相
 *     @arg UART_InvPin_Rx: RXD引脚输入信号反相
 * @param NewState :ENABLE or DISABLE
 */
void UART_InvPinCmd(UART_TypeDef* UARTx, uint16_t UART_InvPin, FunctionalState NewState)
{
    assert_param(IS_UART_ALL_PERIPH(UARTx));
    assert_param(IS_UART_INVERSTION_PIN(UART_InvPin));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    NewState ? (UARTx->CR2 |= UART_InvPin) : (UARTx->CR2 &= (uint16_t)~UART_InvPin);

    // if (NewState != DISABLE)
    // {
    //   UARTx->CR2 |= UART_InvPin;
    // }
    // else
    // {
    //   UARTx->CR2 &= (uint16_t)~UART_InvPin;
    // }
}

/**
 * @brief 使能/失能UARTx半双工通信
 *
 * @param UARTx :UARTx外设
 *        参数可以是:
 *           CW_UART1、CW_UART2、CW_UART3
 * @param NewState :ENABLE or DISABLE
 */

void UART_HalfDuplexCmd(UART_TypeDef* UARTx, FunctionalState NewState)
{
    assert_param(IS_UART_ALL_PERIPH(UARTx));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    UARTx->CR1_f.SIGNAL = NewState;
}

/**
 * @brief 使能/失能UARTx DMA
 *
 * @param UARTx :UARTx外设
 *        参数可以是:
 *           CW_UART1、CW_UART2、CW_UART3
 * @param UART_DMAReq :DMA请求
 *   参数可以是：
 *     @arg UART_DMAReq_Tx: DMA发送请求
 *     @arg UART_DMAReq_Rx: DMA接收请求
 * @param NewState :ENABLE or DISABLE
 */
void UART_DMACmd(UART_TypeDef* UARTx, uint16_t UART_DMAReq, FunctionalState NewState)
{
    assert_param(IS_UART_ALL_PERIPH(UARTx));
    assert_param(IS_UART_DMAREQ(UART_DMAReq));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    NewState ? (UARTx->CR2 |= UART_DMAReq) : (UARTx->CR2 &= (uint16_t)~UART_DMAReq);
}

/**
 * @brief UARTx多机通信配置
 *
 * @param UARTx :UARTx外设
 *        参数可以是:
 *           CW_UART1、CW_UART2、CW_UART3
 * @param UART_Address :从机地址
 * @param UART_AddressMsK :从机地址掩码
 * @note
 *        主机和从机均需配置为自定义校验
 */
void UART_SetMultiMode(UART_TypeDef* UARTx, uint8_t UART_Address, uint8_t UART_AddressMsK)
{
    assert_param(IS_UART_ALL_PERIPH(UARTx));
    assert_param(IS_UART_ADDRESS(UART_Address));
    assert_param(IS_UART_ADDRESSMSK(UART_AddressMsK));

    REGBITS_SET(UARTx->CR2, UARTx_CR2_ADDREN_Msk);
    REGBITS_MODIFY(UARTx->ADDR, UARTx_ADDR_ADDR_Msk, (uint32_t)UART_Address);
    REGBITS_MODIFY(UARTx->MASK, UARTx_MASK_MASK_Msk, (uint32_t)UART_AddressMsK);
}

/**
 * @brief 使能/失能LIN模式
 *
 * @param UARTx :UARTx外设
 *        参数可以是:
 *           CW_UART1、CW_UART2、
 * @param NewState :ENABLE or DISABLE
 */
void UART_LINCmd(UART_TypeDef* UARTx, FunctionalState NewState)
{
    assert_param(IS_UART_ALL_PERIPH(UARTx));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    UARTx->CR3_f.LIN = NewState;
    UARTx->CR1_f.SIGNAL = 0;
    UARTx->CR1_f.SYNC = 0;
    UARTx->CR1_f.STOP = 0;
    UARTx->CR1_f.PARITY = 0;
}

/**
 * @brief 发送LIN中断
 *
 * @param UARTx :UARTx外设
 *        参数可以是:
 *           CW_UART1、CW_UART2、CW_UART3
 * @param BreakLength :间隔段长度 一般大于13bit
 */ 
void UART_LINSendBreak(UART_TypeDef* UARTx, UART_LINBreakSendLengthTypeDef BreakLength)
{
    assert_param(IS_UART_ALL_PERIPH(UARTx));

    UARTx->TDR = BreakLength;   
}
/**
 * @brief 设置LIN模式接收间隔段的长度阈值
 *
 * @param UARTx :UARTx外设
 *        参数可以是:
 *           CW_UART1、CW_UART2、
 * @param UART_LINBreakDetectLength :间隔段长度阈值
  *   参数可以是：
 *     @arg UART_LIN_BREAK_DETECT_LENGTH_10B: 10bit
 *     @arg UART_LIN_BREAK_DETECT_LENGTH_11B: 11bit
 */
void UART_LINBreakDetectLengthConfig(UART_TypeDef* UARTx, UART_LINBreakDetectLengthTypeDef UART_LINBreakDetectLength)
{
    assert_param(IS_UART_ALL_PERIPH(UARTx));    

    UARTx->CR3_f.BRKL = UART_LINBreakDetectLength;
}

/**
 * @brief 定时器工作模式配置
 *
 * @param UARTx :UARTx外设
 *        参数可以是:
 *           CW_UART1、CW_UART2、
 * @param UART_TimerMode :定时器工作模式
  *   参数可以是：
 *     @arg UART_TimerMode_Stop:          停止定时器
 *     @arg UART_TimerMode_TimeOut:       等待超时检测模式
 *     @arg UART_TimerMode_Idle:          接收空闲检测模式
 *     @arg UART_TimerMode_AutoBaudRate1: 自动波特率侦测模式1
 *     @arg UART_TimerMode_AutoBaudRate2: 自动波特率侦测模式2
 *     @arg UART_TimerMode_GeneralTimer:  通用定时功能
 */
void UART_TimerModeConfig(UART_TypeDef* UARTx, uint16_t UART_TimerMode)
{
    assert_param(IS_UART_ALL_PERIPH(UARTx));
    assert_param(IS_UART_TIMER_MODE(UART_TimerMode));

    REGBITS_MODIFY(UARTx->CR2, UARTx_CR2_TIMCR_Msk, (uint32_t)UART_TimerMode);
}

/**
 * @brief 设置定时器重载值
 *
 * @param UARTx :UARTx外设
 *        参数可以是:
 *           CW_UART1、CW_UART2、
 * @param Autoreload :定时器重载值 范围0x000000~0xFFFFFF
 */
void UART_SetAutoReload(UART_TypeDef* UARTx, uint32_t AutoReload)
{
    assert_param(IS_UART_ALL_PERIPH(UARTx));
    assert_param(IS_UART_AUTORELOAD(AutoReload));

    UARTx->TIMARR = AutoReload;
}

/**
 * @brief 获取定时器当前计数值
 *
 * @param UARTx :UARTx外设
 *        参数可以是:
 *           CW_UART1、CW_UART2、
 * @return uint32_t :定时器当前计数值
 */
uint32_t UART_GetCounter(UART_TypeDef* UARTx)
{
    assert_param(IS_UART_ALL_PERIPH(UARTx));

    return UARTx->TIMCNT;
}


/**
 * @brief 交换UARTx引脚功能
 *
 * @param UARTx :UARTx外设
 *        参数可以是:
 *           CW_UART1、CW_UART2、CW_UART3
 * @param NewState : 使能或禁用引脚交换，ENABLE 表示启用交换，DISABLE 表示恢复默认
 */
void UART_PinSwapCmd(UART_TypeDef* UARTx, FunctionalState NewState)
{
    assert_param(IS_UART_ALL_PERIPH(UARTx));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    // 根据不同的UART外设，配置对应的引脚交换
    UARTx->CR2_f.SWAP = NewState;
}

/**
 * @brief 启用自动波特率检测
 * 
 * @param UARTx :UARTx外设
 *        参数可以是:
 *           CW_UART1、CW_UART2、CW_UART3
 * @param AutoBaudRateMode : 自动波特率检测模式，UART_AutoBaudRateModeTypeDef 类型的枚举值
 * @note
 *    - 确保在调用此函数之前已经配置了 UART 模块的时钟源
 *    - 此函数会启用自动波特率检测功能，并根据指定的模式进行配置
 *    - 自动波特率检测模式包括：
 *        - UART_AUTO_BAUD_RATE_MODE_1: RXD下降沿使定时器启动从0开始计数，上升沿停止定时器；计数时钟为UCLK；检测完成自动停止
 *        - UART_AUTO_BAUD_RATE_MODE_2: RXD下降沿使定时器启动从0开始计数，第4个下升沿停止定时器；计数时钟为UCLK；检测完成自动停止
 */
void UART_EnableAutoBaudRateDetection(UART_TypeDef* UARTx,UART_AutoBaudRateModeTypeDef AutoBaudRateMode)
 {
    assert_param(IS_UART_ALL_PERIPH(UARTx));

    UARTx->ICR_f.BAUD = 0; // 清除自动波特率检测完成标志
    UARTx->TIMARR = 0xFFFFFF; // 设置定时器重载值为最大值，确保定时器不会溢出

    if (AutoBaudRateMode == UART_AUTO_BAUD_RATE_MODE_1) 
    {        
        // 配置定时器工作模式为 UART_AUTO_BAUD_RATE_MODE_1
        UART_TimerModeConfig(UARTx, UART_TimerMode_AutoBaudRate1);
    }
    else if (AutoBaudRateMode == UART_AUTO_BAUD_RATE_MODE_2)
    {
        // 配置定时器工作模式为 UART_AUTO_BAUD_RATE_MODE_2
        UART_TimerModeConfig(UARTx, UART_TimerMode_AutoBaudRate2);
    }
}

/**
 * @brief 禁用自动波特率检测
 *
 * @param UARTx :UARTx外设
 *        参数可以是:
 *           CW_UART1、CW_UART2、CW_UART3
 * @note
 *    - 此函数会停止自动波特率检测功能，并禁用定时器
 */
void UART_DisableAutoBaudRateDetection(UART_TypeDef* UARTx)
{
    assert_param(IS_UART_ALL_PERIPH(UARTx));
    UARTx->ICR_f.BAUD = 0; // 清除自动波特率检测完成标志
    UART_TimerModeConfig(UARTx, UART_TimerMode_Stop); // 停止定时器
}


// 检查自动波特率检测是否完成
boolean_t UART_AutoBaudRateDetectionComplete(UART_TypeDef* UARTx) 
{
    assert_param(IS_UART_ALL_PERIPH(UARTx));
    
    return (boolean_t)UARTx->ISR_f.BAUD; // 检查自动波特率检测完成标志
}

// 获取检测到的波特率
uint32_t UART_GetDetectedBaudRate(UART_TypeDef* UARTx) 
{
    assert_param(IS_UART_ALL_PERIPH(UARTx));

    return UARTx->TIMCNT;
}

/**
 * @brief 使能UARTx数据匹配
 *
 * @param UARTx :UARTx外设
 *        参数可以是:
 *           CW_UART1、CW_UART2、CW_UART3
 * @note
 *    - 此函数会使能UARTx的数据匹配功能
 */ 
void  UART_EnableDataMatch(UART_TypeDef* UARTx)
{
    assert_param(IS_UART_ALL_PERIPH(UARTx));

    UARTx->CR2_f.RXMATCHEN = 1;
}

/**
 * @brief 禁用UARTx数据匹配
 *
 * @param UARTx :UARTx外设
 *        参数可以是:
 *           CW_UART1、CW_UART2、CW_UART3
 * @note
 *    - 此函数会禁用UARTx的数据匹配功能
 */
void  UART_DisableDataMatch(UART_TypeDef* UARTx)
{
    assert_param(IS_UART_ALL_PERIPH(UARTx));

    UARTx->CR2_f.RXMATCHEN = 0;
}

/**
 * @brief 设置UARTx数据匹配值
 *
 * @param UARTx :UARTx外设
 *        参数可以是:
 *           CW_UART1、CW_UART2、CW_UART3
 * @param DataMatch : 数据匹配值，范围为0x000~0x1FF
 * @note DataMach的最高位为奇偶校验位
 */
void  UART_SetDataMatch(UART_TypeDef* UARTx, uint16_t DataMatch)
{
   assert_param(IS_UART_ALL_PERIPH(UARTx));
   
   UARTx->RXMATCH = DataMatch;
}

/**
 * @brief 获取UARTx数据匹配标志
 *
 * @param UARTx :UARTx外设
 *        参数可以是:
 *           CW_UART1、CW_UART2、CW_UART3
 * @return boolean_t : 数据匹配标志，TRUE 表示数据匹配，FALSE 表示数据不匹配
 */
boolean_t UART_GetDataMatchFlag(UART_TypeDef* UARTx)
{
    assert_param(IS_UART_ALL_PERIPH(UARTx));
    return (boolean_t)UARTx->ISR_f.RXMATCH;
}



