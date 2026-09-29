/**
 * @file cw32l012_lpi2c.c
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
#include "cw32l012_lpi2c.h"
#include "cw32l012_sysctrl.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static status_t LPI2C_MasterWaitForTxReady(I2C_TypeDef *I2Cx);

static status_t LPI2C_RunTransferStateMachine(I2C_TypeDef *I2Cx, lpi2c_master_handle_t *handle, boolean_t *isDone);

static void LPI2C_InitTransferStateMachine(lpi2c_master_handle_t *handle);

static status_t LPI2C_SlaveCheckAndClearError(I2C_TypeDef *I2Cx, uint32_t flags);


/*******************************************************************************
 * Variables
 ******************************************************************************/

/*! @brief Array to map LPI2C instance number to base pointer. */
static I2C_TypeDef *const kLpi2cBases[] = LPI2C_BASE_PTRS;

/*! @brief Array to map LPI2C instance number to IRQ number, used internally for LPI2C master interrupt APIs. */
IRQn_Type const kLpi2cIrqs[] = LPI2C_IRQS;

/*! @brief Pointer to master IRQ handler for each instance, used internally for LPI2C master interrupt APIs. */
lpi2c_master_isr_t s_lpi2cMasterIsr;

/*! @brief Pointers to master handles for each instance, used internally for LPI2C master interrupt APIs. */
void *s_lpi2cMasterHandle[ARRAY_SZ(kLpi2cBases)];

/*! @brief Pointer to slave IRQ handler for each instance. */
static lpi2c_slave_isr_t s_lpi2cSlaveIsr;

/*! @brief Pointers to slave handles for each instance. */
static lpi2c_slave_handle_t *s_lpi2cSlaveHandle[ARRAY_SZ(kLpi2cBases)];


/* Private_TypesDefinitions --------------------------------------------------*/

/* Private_Defines -----------------------------------------------------------*/

/* Private_Variables ---------------------------------------------------------*/

/* Private_FunctionPrototypes ------------------------------------------------*/

/* Private_Functions ---------------------------------------------------------*/

/* Public_Functions ----------------------------------------------------------*/

/**
 * @brief 初始化I2C主设备
 *
 * 初始化I2C主设备，包括配置时钟源、调试模式、休眠模式、引脚配置、总线空闲超时、引脚低电平超时等参数。
 *
 * @param I2Cx I2C设备指针
 * @param I2C_MasterConfig I2C主设备配置结构体指针
 * @param sourceClock_Hz 主时钟频率（Hz）
 *
 * @return ErrorStatus 错误状态码
 *         - SUCCESS: 初始化成功
 *         - ERROR: 初始化失败
 */
ErrorStatus I2C_MasterInit(I2C_TypeDef *I2Cx, I2C_MasterConfigTypeDef *I2C_MasterConfig, uint32_t sourceClock_Hz)
{
    ErrorStatus RetVal = SUCCESS;
    uint32_t prescaler;
    uint32_t cycles;
    uint32_t cfgr2;

    assert_param(IS_I2C_ALL_PERIPH(I2Cx));

    if ((uint32_t)I2Cx == (uint32_t)CW_I2C1)
    {
        // 打开I2C1模块的时钟
        __SYSCTRL_I2C1_CLK_ENABLE();
    }
    else if((uint32_t)I2Cx == (uint32_t)CW_I2C2)
    {
        // 打开I2C2模块的时钟
        __SYSCTRL_I2C2_CLK_ENABLE();
    }
    else
    {
        return ERROR;
    }

    /* 配置前先对I2C Master复位 */
    I2C_MasterSoftReset(I2Cx);

    /* 配置总线输入信号选择寄存器 */
    REGBITS_MODIFY(I2Cx->INSEL, I2Cx_INSEL_SDAINSEL_Msk | I2Cx_INSEL_SCLINSEL_Msk,
                   ((uint32_t)(I2C_MasterConfig->I2C_SDA_Source)) | ((uint32_t)(I2C_MasterConfig->I2C_SCL_Source)));

    /* 配置时钟源及调试、休眠模式 */
    REGBITS_MODIFY(I2Cx->MCR0, I2Cx_MCR0_CLKSRC_Msk | I2Cx_MCR0_DBGEN_Msk | I2Cx_MCR0_SLEEPEN_Msk,
                   ((uint32_t)(I2C_MasterConfig->I2C_ClockSouce)) |
                       (((uint32_t)(I2C_MasterConfig->I2C_InDebugState & 0x01)) << I2Cx_MCR0_DBGEN_Pos) |
                       (((uint32_t)(I2C_MasterConfig->I2C_InDeepSleepState & 0x01)) << I2Cx_MCR0_SLEEPEN_Pos));

    /* pin config and ignore ack, MATCFG*/
    REGBITS_MODIFY(I2Cx->MCR2, I2Cx_MCR2_PINCFG_Msk | I2Cx_MCR2_IGNACK_Msk | I2Cx_MCR2_MATCFG_Msk,
                   (((uint32_t)(I2C_MasterConfig->I2C_PinsConfig & 0x07)) << I2Cx_MCR2_PINCFG_Pos) |
                       ((uint32_t)(I2C_MasterConfig->ignoreAck) << I2Cx_MCR2_IGNACK_Pos) |
                       ((I2C_MasterConfig->I2C_DataMatchMode) << I2Cx_MCR2_MATCFG_Pos));

    /* Configure glitch filters. */
    cfgr2 = I2Cx->MCR3;

    if (0U != (I2C_MasterConfig->sdaGlitchFilterWidth_ns))
    {
        /* Calculate SDA filter width. The width is equal to FILTSDA cycles of functional clock.
            And set FILTSDA to 0 disables the fileter, so the min value is 1. */
        cycles = I2C_GetCyclesForWidth(sourceClock_Hz, I2C_MasterConfig->sdaGlitchFilterWidth_ns, 1U,
                                       (I2Cx_MCR3_FLTSDA_Msk >> I2Cx_MCR3_FLTSDA_Pos), 0U);
        cfgr2 &= ~I2Cx_MCR3_FLTSDA_Msk;
        cfgr2 |= I2C_MCR3_FILTSDA(cycles);
    }

    if (0U != I2C_MasterConfig->sclGlitchFilterWidth_ns)
    {
        /* Calculate SDL filter width. The width is equal to FILTSCL cycles of functional clock.
            And set FILTSCL to 0 disables the fileter, so the min value is 1. */
        cycles = I2C_GetCyclesForWidth(sourceClock_Hz, I2C_MasterConfig->sclGlitchFilterWidth_ns, 1U,
                                       (I2Cx_MCR3_FLTSCL_Msk >> I2Cx_MCR3_FLTSCL_Pos), 0U);
        cfgr2 &= ~I2Cx_MCR3_FLTSCL_Msk;
        cfgr2 |= I2C_MCR3_FILTSCL(cycles);
    }

    I2Cx->MCR3 = cfgr2;

    /* Configure baudrate after the SDA/SCL glitch filter setting,
       since the baudrate calculation needs them as parameter. */
    I2C_MasterSetBaudRate(I2Cx, sourceClock_Hz, I2C_MasterConfig->I2C_BaudRate_Hz);

    /* Configure bus idle and pin low timeouts after baudrate setting,
       since the timeout calculation needs prescaler as parameter. */
    prescaler = (I2Cx->MCR2 & I2Cx_MCR2_PRESCALE_Msk) >> I2Cx_MCR2_PRESCALE_Pos;

    if (0U != (I2C_MasterConfig->busIdleTimeout_ns))
    {
        /* Calculate bus idle timeout value. The value is equal to BUSIDLE cycles of functional clock divided by
            prescaler. And set BUSIDLE to 0 disables the fileter, so the min value is 1. */
        cycles = I2C_GetCyclesForWidth(sourceClock_Hz, I2C_MasterConfig->busIdleTimeout_ns, 1U,
                                       (I2Cx_MCR3_BUSIDLE_Msk >> I2Cx_MCR3_BUSIDLE_Pos), prescaler);
        cfgr2 &= ~I2Cx_MCR3_BUSIDLE_Msk;
        cfgr2 |= I2C_MCR3_BUSIDLE(cycles);
    }

    I2Cx->MCR3 = cfgr2;

    if (0U != I2C_MasterConfig->pinLowTimeout_ns)
    {
        /* Calculate bus pin low timeout value. The value is equal to PINLOW cycles of functional clock divided by
            prescaler. And set PINLOW to 0 disables the fileter, so the min value is 1. */
        cycles = I2C_GetCyclesForWidth(sourceClock_Hz, I2C_MasterConfig->pinLowTimeout_ns / 256U, 1U,
                                       (I2Cx_MCR3_BUSIDLE_Msk >> I2Cx_MCR3_BUSIDLE_Pos), prescaler);
        I2Cx->MCR4 = (I2Cx->MCR4 & ~I2Cx_MCR4_PINLOW_Msk) | I2C_MCR4_PINLOW(cycles);
    }

    return RetVal;
}

/**
 * @brief 初始化I2C从设备
 *
 * 该函数用于初始化I2C从设备，包括设置从设备地址、时钟延展、滤波时钟源等。
 *
 * @param I2Cx 指向I2C外设的指针
 * @param I2C_SlaveConfig 指向I2C从设备配置结构体的指针
 * @param sourceClock_Hz I2C外设的时钟源频率，单位为Hz
 *
 * @return ErrorStatus 返回初始化状态
 *         - SUCCESS: 初始化成功
 *         - ERROR: 初始化失败
 */
ErrorStatus I2C_SlaveInit(I2C_TypeDef *I2Cx, I2C_SlaveConfigTypeDef *I2C_SlaveConfig, uint32_t sourceClock_Hz)
{
    ErrorStatus RetVal = SUCCESS;
    uint32_t tmpReg;
    uint32_t tmpCycle;

    // 检查I2C外设是否有效
    assert_param(IS_I2C_ALL_PERIPH(I2Cx));

    // 空指针检查
    if (I2Cx == NULL || I2C_SlaveConfig == NULL)
    {
        return ERROR;
    }

    if ((uint32_t)I2Cx == (uint32_t)CW_I2C1)
    {
        // 打开I2C1模块的时钟
        __SYSCTRL_I2C1_CLK_ENABLE();
    }
    else if ((uint32_t)I2Cx == (uint32_t)CW_I2C2)
    {
        // 打开I2C2模块的时钟
        __SYSCTRL_I2C2_CLK_ENABLE();
    }
    else
    {
        return ERROR;
    }

    /* 配置前先对I2C Slave复位 */
    I2C_SlaveSoftReset(I2Cx);

    /* 配置Slave 从机地址匹配方式 */
    REGBITS_MODIFY(I2Cx->SAMR, I2Cx_SAMR_ADDR1_Msk | I2Cx_SAMR_ADDR0_Msk,
                   ((((uint32_t)(I2C_SlaveConfig->I2C_Addr1 & MAX_SLAVE_ADDR_10B)) << I2Cx_SAMR_ADDR1_Pos) |
                    (((uint32_t)(I2C_SlaveConfig->I2C_Addr0 & MAX_SLAVE_ADDR_10B)) << I2Cx_SAMR_ADDR0_Pos)));

    /* 配置Slave SCR1 */
    I2Cx->SCR1 =
        I2C_SCR1_ADDRCFG(I2C_SlaveConfig->addressMatchMode) | I2C_SCR1_IGNACK(I2C_SlaveConfig->ignoreAck) |
        I2C_SCR1_RXCFG(I2C_SlaveConfig->enableReceivedAddressRead) | I2C_SCR1_GCEN(I2C_SlaveConfig->enableGeneralCall) |
        I2C_SCR1_ACKSTALL(I2C_SlaveConfig->sclStall.enableAck) | I2C_SCR1_TXSTALL(I2C_SlaveConfig->sclStall.enableTx) |
        I2C_SCR1_RXSTALL(I2C_SlaveConfig->sclStall.enableRx) |
        I2C_SCR1_ADRSTALL(I2C_SlaveConfig->sclStall.enableAddress);

    /* Calculate SDA filter width. The width is equal to FILTSDA+3 cycles of functional clock.
        And set FILTSDA to 0 disables the fileter, so the min value is 4. */
    tmpReg = I2C_SCR3_FILTSDA(
        I2C_GetCyclesForWidth(sourceClock_Hz, I2C_SlaveConfig->sdaGlitchFilterWidth_ns, 4U,
                              (I2Cx_SCR3_FLTSDA_Msk >> I2Cx_SCR3_FLTSDA_Pos) + 3U, 0U) - 3U);

    /* Calculate SCL filter width. The width is equal to FILTSCL+3 cycles of functional clock.
        And set FILTSCL to 0 disables the fileter, so the min value is 4. */
    tmpCycle = I2C_GetCyclesForWidth(sourceClock_Hz, I2C_SlaveConfig->sclGlitchFilterWidth_ns, 4U,
                                     (I2Cx_SCR3_FLTSCL_Msk >> I2Cx_SCR3_FLTSCL_Pos) + 3U, 0U);
    tmpReg |= I2C_SCR3_FILTSCL(tmpCycle - 3U);

    /* Calculate data valid time. The time is equal to FILTSCL+DATAVD+3 cycles of functional clock.
        So the min value is FILTSCL+3. */
    tmpReg |= I2C_SCR3_DATAVD(
        I2C_GetCyclesForWidth(sourceClock_Hz, I2C_SlaveConfig->dataValidDelay_ns, tmpCycle,
                              tmpCycle + (I2Cx_SCR3_DATAVD_Msk >> I2Cx_SCR3_DATAVD_Pos), 0U) - tmpCycle);

    /* Calculate clock hold time. The time is equal to CLKHOLD+3 cycles of functional clock.
        So the min value is 3. */
    I2Cx->SCR3 =
        tmpReg | I2C_SCR3_CLKHOLD(
                     I2C_GetCyclesForWidth(sourceClock_Hz, I2C_SlaveConfig->clockHoldTime_ns, 3U,
                                           (I2Cx_SCR3_CLKHOLD_Msk >> I2Cx_SCR3_CLKHOLD_Pos) + 3U, 0U) - 3U);

    REGBITS_MODIFY( I2Cx->SCR0 , I2Cx_SCR0_FLTEN_Msk, (I2C_SlaveConfig->filterEnable << I2Cx_SCR0_FLTEN_Pos));

    return RetVal;
}

/**
 * @brief I2C Master模块使能控制
 *
 * @param I2Cx
 * @param NewState  : ENABLE  DISABLE
 */
void I2C_MasterCmd(I2C_TypeDef *I2Cx, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_I2C_ALL_PERIPH(I2Cx));

    if (NewState == ENABLE)
    {
        /* Enable the selected I2C peripheral */
        I2Cx->MCR0_f.MEN = 1;
    }
    else
    {
        /* Disable the selected I2C peripheral */
        I2Cx->MCR0_f.MEN = 0;
    }
}

/**
 * @brief I2C Slave模块使能控制
 *
 * @param I2Cx
 * @param NewState  : ENABLE  DISABLE
 */
void I2C_SlaveCmd(I2C_TypeDef *I2Cx, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_I2C_ALL_PERIPH(I2Cx));

    if (NewState == ENABLE)
    {
        /* Enable the selected I2C peripheral */
        I2Cx->SCR0_f.SEN = 1;
    }
    else
    {
        /* Disable the selected I2C peripheral */
        I2Cx->SCR0_f.SEN = 0;
    }
}

/**
 * @brief I2C 模块去初始化
 *
 * @param I2Cx
 */
void I2C_DeInit(I2C_TypeDef *I2Cx)
{
    assert_param(IS_I2C_ALL_PERIPH(I2Cx));
    // 复位I2C模块
    if(I2Cx == CW_I2C1)
    {
        CW_SYSCTRL->APBRST2_f.I2C1 = 0;
        CW_SYSCTRL->APBRST2_f.I2C1 = 1;
        // 关闭I2C模块的时钟
        __SYSCTRL_I2C1_CLK_DISABLE();
    }
    else
    {
        CW_SYSCTRL->APBRST2_f.I2C2 = 0;
        CW_SYSCTRL->APBRST2_f.I2C2 = 1;
        // 关闭I2C模块的时钟
        __SYSCTRL_I2C2_CLK_DISABLE();
    }
}

/**
 * @brief 清除I2C主机标志位
 *
 * @param I2Cx I2C外设指针
 * @param I2C_FLAG 要清除的标志位，可以是以下值之一及其组合：
 *                 - I2C_IT_MATCH
 *                 - I2C_IT_PINLOW
 *                 - I2C_IT_FIFO
 *                 - I2C_IT_ARBI
 *                 - I2C_IT_NACK
 *                 - I2C_IT_STOP
 *                 - I2C_IT_PACKET
 *                 - I2C_IT_FRAME
 *
 */
void I2C_MasterClearFlag(I2C_TypeDef *I2Cx, uint32_t I2C_FLAG)
{
    assert_param(IS_I2C_ALL_PERIPH(I2Cx));
    assert_param(IS_I2C_CLEAR_FLAG(I2C_FLAG));

    REGBITS_CLR(I2Cx->MICR, I2C_FLAG);
}

/**
 * @brief I2C主模式轮询标志位函数
 *
 * 该函数用于在I2C主模式下，通过轮询方式检查指定的I2C标志位状态。
 *
 * @param I2Cx 指向I2C外设的指针
 * @param I2C_FLAG 需要检查的I2C标志位，可以是以下值之一及其组合：
 *                 - I2C_IT_BUSBUSY
 *                 - I2C_IT_MSTBUSY
 *                 - I2C_IT_MATCH
 *                 - I2C_IT_PINLOW
 *                 - I2C_IT_FIFO
 *                 - I2C_IT_ARBI
 *                 - I2C_IT_NACK
 *                 - I2C_IT_STOP
 *                 - I2C_IT_PACKET
 *                 - I2C_IT_FRAME
 *                 - I2C_IT_RXNE
 *                 - I2C_IT_TXE
 * @param reverse 标志位检测逻辑，true表示取反检测，false表示直接检测
 *
 * @return 如果检测到标志位，返回I2C_Status_Success；否则，返回错误代码
 *
 * @note 调用此函数前，需确保传入的I2Cx指针指向有效的I2C外设，I2C_FLAG为有效的I2C标志位。
 *       如果等待超时仍未检测到标志位，返回I2C_Status_Timeout。
 */
static uint8_t I2C_MasterPollingFlagCommon(I2C_TypeDef *I2Cx, uint32_t I2C_FLAG, boolean_t reverse)
{
    assert_param(IS_I2C_ALL_PERIPH(I2Cx));
    assert_param(IS_I2C_FLAG(I2C_FLAG));

    for (uint32_t waitTimes = I2C_WAIT_TIMEOUT; waitTimes > 0; --waitTimes)
    {
        uint32_t regVal = I2Cx->MISR;
        uint32_t flagDetected = reverse ? !(regVal & I2C_FLAG) : (regVal & I2C_FLAG);

        if (flagDetected)
        {
            return I2C_Status_Success;
        }
    }

    return I2C_Status_Timeout;
}

/**
 * @brief 查询I2C主机标志位的状态
 *
 * @param I2Cx I2C模块指针
 * @param I2C_FLAG 需要查询的标志位
 *
 * @return 如果标志位被设置，则返回I2C_Status_Success；否则返回错误代码
 */
uint8_t I2C_MasterPollingFlag(I2C_TypeDef *I2Cx, uint32_t I2C_FLAG)
{
    return I2C_MasterPollingFlagCommon(I2Cx, I2C_FLAG, FALSE);
}

/**
 * @brief 反转查询I2C主模式标志位
 *
 * @param I2Cx I2C接口指针
 * @param I2C_FLAG 要反转的标志位
 *
 * @return 如果标志位未被设置，则返回I2C_Status_Success；否则返回错误代码
 */
uint8_t I2C_MasterPollingFlagReverse(I2C_TypeDef *I2Cx, uint32_t I2C_FLAG)
{
    return I2C_MasterPollingFlagCommon(I2Cx, I2C_FLAG, TRUE);
}

/**
 * @brief 获取I2C主模式标志位状态
 *
 * @param I2Cx 指向I2C外设的指针
 * @param I2C_FLAG 要检查的I2C标志位
 *
 * @return 标志位状态
 *
 * @note 调用此函数前，请确保I2Cx和I2C_FLAG参数有效。
 */
uint32_t I2C_MasterGetFlag(I2C_TypeDef *I2Cx, uint32_t I2C_FLAG)
{
    assert_param(IS_I2C_ALL_PERIPH(I2Cx));
    assert_param(IS_I2C_FLAG(I2C_FLAG));

    // 获取并返回指定的标志位状态
    return (I2Cx->MISR & I2C_FLAG);
}

/**
 * @brief 主机发送START信号和从机地址到I2C总线上
 *
 * @param I2Cx: I2C
 * @param slaveAddr:  7-bit从设备地址 .
 * @param dataMode：主机传输的方向，I2C_MODE_WRITE或者I2C_MODE_READ
 * @retval
 */
uint8_t I2C_MasterSTART_Addr7b(I2C_TypeDef *I2Cx, uint8_t slaveAddr, I2C_ModeTypeDef dataMode)
{
    assert_param(IS_I2C_ALL_PERIPH(I2Cx));
    assert_param(slaveAddr < 0x80);

    if (I2C_MasterGetFlag(I2Cx, I2C_IT_BUSBUSY | I2C_IT_MSTBUSY))
    {
        return I2C_Status_Busy;
    }

    // 清IT标志
    I2Cx->MICR &= CLEAR_IT_MASK;

    // 发送start+SLA+R/W命令
    I2Cx->MTDR = I2C_Cmd_Tx_RepSTART_Data_Rx_ACK | (slaveAddr << 1) | dataMode;
    return (I2C_MasterPollingFlagReverse(I2Cx, I2C_IT_NACK));
}

/**
 * @brief 主机向I2C从设备发送ReSTART信号和7位地址，并指定数据传输模式
 *
 * @param I2Cx I2C外设指针，指向要使用的I2C外设
 * @param slaveAddr 从设备地址，7位地址，范围为0到127
 * @param dataMode 数据传输模式，指定传输方向（读或写）
 *
 * @return 如果函数成功发送ReSTART信号并设置从设备地址，则返回0；否则返回错误代码
 *
 */
uint8_t I2C_MasterReSTART_Addr7b(I2C_TypeDef *I2Cx, uint8_t slaveAddr, I2C_ModeTypeDef dataMode)
{
    assert_param(IS_I2C_ALL_PERIPH(I2Cx));
    assert_param(slaveAddr <= MAX_SLAVE_ADDR_7B);

    // 清IT标志
    I2Cx->MICR &= CLEAR_IT_MASK;

    // 发送start+SLA+R/W命令
    I2Cx->MTDR = I2C_Cmd_Tx_RepSTART_Data_Rx_ACK | (slaveAddr << 1) | dataMode;
    return (I2C_MasterPollingFlagReverse(I2Cx, I2C_IT_NACK));
}

/**
 * @brief 主机向I2C从设备发送10位地址的起始信号和数据模式
 *
 * @param I2Cx I2C模块指针，指向要操作的I2C模块
 * @param slaveAddr 从设备的10位地址
 * @param dataMode 数据模式，指定是读操作还是写操作
 *
 * @return 返回函数执行状态
 *
 */
uint8_t I2C_MasterSTART_Addr10b(I2C_TypeDef *I2Cx, uint16_t slaveAddr, I2C_ModeTypeDef dataMode)
{
    assert_param(IS_I2C_ALL_PERIPH(I2Cx));

    if (I2C_MasterGetFlag(I2Cx, I2C_IT_BUSBUSY | I2C_IT_MSTBUSY))
    {
        return I2C_Status_Busy;
    }

    // 清IT标志
    I2Cx->MICR &= CLEAR_IT_MASK;

    // 发送start+SLA_MSB+R/W命令
    I2Cx->MTDR = I2C_Cmd_Tx_RepSTART_Data_Rx_ACK | ((uint8_t)((slaveAddr & I2C_SLAVE_ADDR10B_MSB_MASK) >> (8 - 1))) | I2C_SLAVE_ADDR10B_MSB | dataMode;

    if (I2C_MasterPollingFlagReverse(I2Cx, I2C_IT_NACK))
    {
        return I2C_Status_Nak;
    }

    // 发送SLA_LSB命令
    I2Cx->MTDR = (uint8_t)(slaveAddr & I2C_SLAVE_ADDR10B_LSB_MASK);

    return (I2C_MasterPollingFlagReverse(I2Cx, I2C_IT_NACK));
}

/**
 * @brief 主机发送10位地址的I2C主机ReSTART命令
 *
 * @param I2Cx I2C外设的指针
 * @param slaveAddr 从机地址，10位
 * @param dataMode 数据模式，读写标志
 *
 * @return 返回状态码，成功返回I2C_Status_Success，NACK响应返回I2C_Status_Nak，超时返回I2C_Status_Timeout
 */
uint8_t I2C_MasterReSTART_Addr10b(I2C_TypeDef *I2Cx, uint16_t slaveAddr, I2C_ModeTypeDef dataMode)
{
    assert_param(IS_I2C_ALL_PERIPH(I2Cx));

    // 清IT标志
    I2Cx->MICR &= CLEAR_IT_MASK;

    // 发送start+SLA_MSB+R/W命令
    I2Cx->MTDR = I2C_Cmd_Tx_RepSTART_Data_Rx_ACK | ((uint8_t)((slaveAddr & I2C_SLAVE_ADDR10B_MSB_MASK) >> (8 - 1))) | I2C_SLAVE_ADDR10B_MSB | dataMode;

    if (I2C_MasterPollingFlagReverse(I2Cx, I2C_IT_NACK))
    {
        return I2C_Status_Nak;
    }

    return I2C_Status_Success;
}

/**
 * @brief 主机发送STOP信号到I2C总线
 *
 * @retval
 */
uint8_t I2C_MasterSTOP(I2C_TypeDef *I2Cx)
{
    assert_param(IS_I2C_ALL_PERIPH(I2Cx));

    if (I2C_MasterPollingFlag(I2Cx, I2C_IT_TXE) != I2C_Status_Success)
    {
        return I2C_Status_Timeout;
    }

    // 发送STOP命令
    I2Cx->MTDR = I2C_Cmd_Tx_STOP;
    // return I2C_Status_Success;
    return (I2C_MasterPollingFlag(I2Cx, I2C_IT_STOP));
}

/**
 * @brief I2C 主机接收数据
 *
 * @param I2Cx
 * @param rxBuff:  接收缓存地址
 * @param rxSize： 接收长度
 * @retval 接收状态，成功返回I2C_Status_Success，失败返回I2C_Status_Error或其他错误码
 */
uint8_t I2C_MasterReceive(I2C_TypeDef *I2Cx, uint8_t *rxBuff, uint16_t rxSize)
{
    uint8_t *buf;

    if ((rxBuff == NULL) || (rxSize == 0))
    {
        return I2C_Status_Error;
    }

    if (I2C_MasterPollingFlag(I2Cx, I2C_IT_TXE) != I2C_Status_Success)
    {
        return I2C_Status_Timeout; // 超时错误
    }

    // 发送接收数据命令
    I2Cx->MTDR = I2C_Cmd_Rx_DataToMRDR | (rxSize - 1);

    // 接收数据
    buf = rxBuff;

    while (rxSize--)
    {
        if (I2C_MasterPollingFlag(I2Cx, I2C_IT_RXNE) != I2C_Status_Success)
        {
            return I2C_Status_Timeout; // 超时错误
        }

        *buf++ = (uint8_t)I2Cx->MRDR;
    }

    return I2C_Status_Success;
}

/**
 * @brief I2C 主机发送数据
 *
 * @param I2Cx
 * @param txBuff:  发送缓存地址
 * @param txSize： 发送长度
 * @retval 发送状态，成功返回I2C_Status_Success，失败返回I2C_Status_Error或其他错误码
 */
uint8_t I2C_MasterSend(I2C_TypeDef *I2Cx, uint8_t *txBuff, uint16_t txSize)
{
    uint8_t *buf;

    // 检查输入参数
    if (txBuff == NULL || txSize == 0)
    {
        return I2C_Status_Error;
    }

    buf = txBuff;

    while (txSize--)
    {
        // 等待TXE标志位被置位，表示发送缓冲区空闲
        if (I2C_MasterPollingFlag(I2Cx, I2C_IT_TXE) != I2C_Status_Success)
        {
            return I2C_Status_Timeout; // 超时错误
        }

        // 发送数据
        I2Cx->MTDR = *buf++;

        if (I2C_MasterPollingFlagReverse(I2Cx, I2C_IT_NACK) != I2C_Status_Success)
        {
            return I2C_Status_Timeout; // 超时错误
        }
    }

    return I2C_Status_Success;
}

/**
 * @brief MCU对I2C总线进行软件复位
 *
 * 该函数用于对指定的I2C总线进行软件复位操作。
 * 将所有主机逻辑和寄存器重置为默认状态，I2C_MCR0 寄存器自身除外
 *
 * @param I2Cx 指向I2C控制器的指针
 */
void I2C_MasterSoftReset(I2C_TypeDef *I2Cx)
{
    I2Cx->MCR0 = I2Cx_MCR0_RESET_Msk;
    I2Cx->MCR0 = 0;
}

/**
 * @brief MCU对I2C从设备进行软件复位
 *
 * 该函数用于对指定的I2C从设备进行软件复位。
 * 将所有从机逻辑和寄存器重置为默认状态，I2C_SCR0寄存器自身除外。
 *
 * @param I2Cx I2C设备的指针，指向需要复位的I2C设备。
 */
void I2C_SlaveSoftReset(I2C_TypeDef *I2Cx)
{
    I2Cx->SCR0 = I2Cx_SCR0_RESET_Msk;
    I2Cx->SCR0 = 0;
}

/**
 * @brief 重置I2C主模式发送FIFO
 *
 * 该函数用于重置I2C主模式的发送FIFO。
 * 写 1 复位发送 FIFO,FIFO 复位后，该 FIFO 为空。

 * @param I2Cx I2C模块指针，指向I2C_TypeDef结构体
 */
void I2C_MasterTxFIFOReset(I2C_TypeDef *I2Cx)
{
    I2Cx->MCR0_f.TXFIFORST = 1;
    I2Cx->MCR0_f.TXFIFORST = 0;
}

/**
 * @brief 重置I2C从机接收FIFO
 *
 * 该函数用于重置指定I2C从机的接收FIFO。
 *
 * @param I2Cx I2C控制器指针，指向要操作的I2C控制器。
 */
void I2C_MasterRxFIFOReset(I2C_TypeDef *I2Cx)
{
    I2Cx->MCR0_f.RXFIFORST = 1;
    I2Cx->MCR0_f.RXFIFORST = 0;
}

/**
 * @brief 重置I2C从机发送FIFO
 *
 * 该函数用于重置I2C从机的发送FIFO。
 * 写 1 复位发送 FIFO,FIFO 复位后，该 FIFO 为空。
 *
 * @param I2Cx I2C设备指针
 */
void I2C_SlaveTxFIFOReset(I2C_TypeDef *I2Cx)
{
    I2Cx->SCR0_f.TXFIFORST = 1;
    I2Cx->SCR0_f.TXFIFORST = 0;
}

/**
 * @brief 重置I2C从机接收FIFO
 *
 * 重置指定的I2C从机接收FIFO。
 * 写 1 复位接收数据寄存器。数据寄存器复位后，该数据寄存器为空。
 *
 * @param I2Cx I2C设备指针
 */
void I2C_SlaveRxFIFOReset(I2C_TypeDef *I2Cx)
{
    I2Cx->SCR0_f.RXFIFORST = 1;
    I2Cx->SCR0_f.RXFIFORST = 0;
}

/**
 * @brief 设置I2C总线滤波器
 *
 * 该函数用于设置I2C总线的SDA和SCL滤波器的值。
 *
 * @param SDAFilter SDA滤波器的值，取值范围为0到15
 * @param SCLFilter SCL滤波器的值，取值范围为0到15
 *
 * @return 无返回值
 *
 * @note 调用此函数前需确保SDAFilter和SCLFilter的值小于16，否则会触发断言失败
 */
void I2C_MasterBusFilter(I2C_TypeDef *I2Cx, uint8_t SDAFilter, uint8_t SCLFilter)
{
    assert_param(SDAFilter < 16);
    assert_param(SCLFilter < 16);
    REGBITS_MODIFY(I2Cx->MCR3, I2Cx_MCR3_FLTSDA_Msk | I2Cx_MCR3_FLTSCL_Msk,
                   ((SDAFilter & 0x0F) << I2Cx_MCR3_FLTSDA_Pos) | ((SCLFilter & 0x0F) << I2Cx_MCR3_FLTSCL_Pos));
}

/**
 * @brief 设置I2C从设备总线滤波参数
 *
 * 该函数用于设置I2C从设备的SDA和SCL线的滤波参数，以提高通信的稳定性。
 *
 * @param SDAFilter SDA线的滤波参数，取值范围为0到15。
 * @param SCLFilter SCL线的滤波参数，取值范围为0到15。
 *
 * @note SDAFilter和SCLFilter的值必须小于16，否则会触发断言错误。
 * 如果SDAFilter的值小于SCLFilter，则SDAFilter将被设置为SCLFilter的值。
 *
 * @retval 无返回值
 */
void I2C_SlaveBusFilter(I2C_TypeDef *I2Cx, uint8_t SDAFilter, uint8_t SCLFilter)
{
    assert_param(SDAFilter < 16);
    assert_param(SCLFilter < 16);

    if (SDAFilter < SCLFilter)
    {
        SDAFilter = SCLFilter;
    }

    REGBITS_MODIFY(I2Cx->SCR3, I2Cx_SCR3_FLTSDA_Msk | I2Cx_SCR3_FLTSCL_Msk,
                   ((SDAFilter & 0x0F) << I2Cx_SCR3_FLTSDA_Pos) | ((SCLFilter & 0x0F) << I2Cx_SCR3_FLTSCL_Pos));
}

/**
 * @brief 使能或禁用I2C主模式接收数据匹配功能
 *
 * @param I2Cx I2C模块指针，指向要操作的I2C模块
 * @param NewState 功能状态，使能或禁用接收数据匹配功能
 *        - ENABLE: 使能接收数据匹配功能
 *        - DISABLE: 禁用接收数据匹配功能
 *
 * @return 无
 *
 * @note 在调用此函数之前，请确保已经正确初始化了I2C模块。
 */
void I2C_MasterRxDataMatch(I2C_TypeDef *I2Cx, FunctionalState NewState)
{
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    // 获取当前状态
    FunctionalState CurrentState = (I2Cx->MCR1_f.RDMO == 1) ? ENABLE : DISABLE;

    // 如果状态需要改变
    if (CurrentState != NewState)
    {
        // 确保临界区安全
        __disable_irq(); // 禁用全局中断，防止竞态条件

        if (NewState == ENABLE)
        {
            /* 从禁止更改为使能时，先清除 MICR.MATCH 标志再使能 RDMO */
            I2Cx->MICR_f.MATCH = 0;
            I2Cx->MCR1_f.RDMO = 1;
        }
        else
        {
            /* 从使能更改为禁止时，先禁止 RDMO 再清除 MICR.MATCH 标志 */
            I2Cx->MCR1_f.RDMO = 0;
            I2Cx->MICR_f.MATCH = 0;
        }

        __enable_irq(); // 恢复全局中断
    }
}

/**
 * @brief 设置I2C主设备发送FIFO自动加载功能
 *
 * @param I2Cx I2C设备指针，指向要操作的I2C设备
 * @param NewState 新的功能状态，ENABLE表示启用，DISABLE表示禁用
 *
 * @retval 无
 */
void I2C_MasterTxFIFO_Autoload(I2C_TypeDef *I2Cx, FunctionalState NewState)
{
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState == ENABLE)
    {
        I2Cx->MCR1_f.CIRFIFO = 1;
    }
    else
    {
        I2Cx->MCR1_f.CIRFIFO = 0;
    }
}

/**
 * @brief 初始化I2C主机配置结构
 *
 * 初始化I2C主机配置结构，并将其设置为默认配置。
 *
 * @param masterConfig 指向I2C_MasterConfigTypeDef类型的指针，指向需要初始化的配置结构
 */
void I2C_MasterInitDefault(I2C_MasterConfigTypeDef *masterConfig)
{
    /* Initializes the configure structure to zero. */
    (void)memset(masterConfig, 0, sizeof(*masterConfig));

    masterConfig->I2C_BaudRate_Hz = 400000U;
    masterConfig->I2C_ClockSouce = I2C_CLK_SRC_PCLK;
    masterConfig->I2C_DataMatchMode = I2C_ByteMatch_Prohibit;
    masterConfig->I2C_PinsConfig = 0;
    masterConfig->I2C_SDA_Source = I2C_SDA_INSRC_GPIO;
    masterConfig->I2C_SCL_Source = I2C_SCL_INSRC_GPIO;
    masterConfig->I2C_InDebugState = FALSE;
    masterConfig->I2C_InDeepSleepState = TRUE;
    masterConfig->I2C_RxDataMatch = FALSE;
    masterConfig->I2C_TxFIFO_Autoload = FALSE;
    masterConfig->ignoreAck = FALSE;
    masterConfig->sclGlitchFilterWidth_ns = 0; /* Set to 0 to disable the function */
    masterConfig->sdaGlitchFilterWidth_ns = 0;
    masterConfig->busIdleTimeout_ns = 0;
    masterConfig->pinLowTimeout_ns = 0;
    masterConfig->hostRequest.enable = FALSE;
    masterConfig->hostRequest.source = kLPI2C_HostRequestExternalPin;
    masterConfig->hostRequest.polarity = kLPI2C_HostRequestPinActiveHigh;
}

/**
 * @brief 初始化I2C从设备配置
 *
 * 初始化I2C从设备的配置结构，将其重置为默认值。
 *
 * @param slaveConfig 指向I2C_SlaveConfigTypeDef结构的指针，用于存储从设备配置
 */
void I2C_SlaveInitDefault(I2C_SlaveConfigTypeDef *slaveConfig)
{
    /* Initializes the configure structure to zero. */
    (void)memset(slaveConfig, 0, sizeof(*slaveConfig));

    slaveConfig->addressMatchMode = I2C_Addr0_7bit;
    slaveConfig->I2C_Addr0 = 0x00;
    slaveConfig->I2C_Addr1 = 0x00;
    slaveConfig->I2C_ClockSouce = I2C_CLK_SRC_PCLK_SLV;
    slaveConfig->I2C_RegAddr = I2C_REG_ADDR_8BIT;
    slaveConfig->filterEnable = TRUE;
    slaveConfig->enableGeneralCall = FALSE;
    slaveConfig->sclStall.enableAck = FALSE;
    slaveConfig->sclStall.enableTx = TRUE;
    slaveConfig->sclStall.enableRx = TRUE;
    slaveConfig->sclStall.enableAddress = FALSE;
    slaveConfig->ignoreAck = FALSE;
    slaveConfig->enableReceivedAddressRead = FALSE;
    slaveConfig->sdaGlitchFilterWidth_ns = 0U; /* Set to 0 to disable the function */
    slaveConfig->sclGlitchFilterWidth_ns = 0U;
    slaveConfig->dataValidDelay_ns = 0U;
    /* When enabling the slave tx SCL stall, set the default clock hold time to 250ns according
       to I2C spec for standard mode baudrate(100k). User can manually change it to 100ns or 50ns
       for fast-mode(400k) or fast-mode+(1M). */
    slaveConfig->clockHoldTime_ns = 100U; //Default fast-mode(400k)
}

/**
 * @brief 从EEPROM读取数据
 *
 * 通过I2C总线从EEPROM读取指定地址的数据。
 *
 * @param I2Cx I2C接口指针
 * @param regAddr EEPROM中的起始地址
 * @param regAddrType 地址类型，用于指定地址的长度和格式
 * @param pu8Data 存储读取数据的缓冲区指针
 * @param u16Len 要读取的数据长度
 *
 * @return 读取操作的状态码，成功为0，失败为非0
 */
uint8_t I2C_MasterReadEepromData(I2C_TypeDef *I2Cx, uint32_t regAddr, I2C_SlaveRegAddrTypeDef regAddrType, uint8_t *pu8Data, uint16_t u16Len)
{
    uint8_t u8State;
    uint8_t dataTemp[4];
    uint8_t j;

    switch (regAddrType)
    {
    case I2C_REG_ADDR_8BIT:
        j = 1;
        break;
    case I2C_REG_ADDR_16BIT:
        j = 2;
        break;
    case I2C_REG_ADDR_24BIT:
        j = 3;
        break;
    case I2C_REG_ADDR_32BIT:
        j = 4;
        break;
    default:
        return I2C_Status_Error;
    }

    for (uint8_t i = 0; i < j; i++)
    {
        dataTemp[i] = (uint8_t)((regAddr >> (8 * (j - 1 - i))) & 0xFF); //MSB first
    }

    u8State = I2C_MasterReceiveDataFromSlave(I2Cx, I2C_SLAVEADDRESS_EEPROM, dataTemp, regAddrType, pu8Data, u16Len);
    return u8State;
}

/**
 * @brief 向EEPROM写入数据
 *
 * 通过I2C总线向EEPROM写入数据。
 *
 * @param I2Cx I2C总线接口指针
 * @param regAddr EEPROM寄存器地址
 * @param regAddrType 寄存器地址类型
 * @param pu8Data 待写入的数据指针
 * @param u16Len 待写入的数据长度
 *
 * @return 函数执行状态，返回I2C_Status_OK表示成功，否则表示失败
 */
uint8_t I2C_MasterWriteEepromData(I2C_TypeDef *I2Cx, uint32_t regAddr, I2C_SlaveRegAddrTypeDef regAddrType, uint8_t *pu8Data, uint16_t u16Len)
{
    if (I2Cx == NULL || pu8Data == NULL || u16Len == 0 || u16Len > MAX_DATA_LEN_EEPROM)
    {
        return I2C_Status_Error;
    }

    uint8_t dataTemp[4 + MAX_DATA_LEN_EEPROM];
    uint8_t j;

    switch (regAddrType)
    {
    case I2C_REG_ADDR_8BIT:
        j = 1;
        break;
    case I2C_REG_ADDR_16BIT:
        j = 2;
        break;
    case I2C_REG_ADDR_24BIT:
        j = 3;
        break;
    case I2C_REG_ADDR_32BIT:
        j = 4;
        break;
    default:
        return I2C_Status_Error;
    }

    for (uint8_t i = 0; i < j; i++)
    {
        dataTemp[i] = (uint8_t)((regAddr >> (8 * (j - 1 - i))) & 0xFF); //MSB first
    }

    memcpy(dataTemp + j, pu8Data, u16Len);
    return I2C_MasterSendDataToSlave(I2Cx, I2C_SLAVEADDRESS_EEPROM, dataTemp, u16Len + j);
}

/**
 * @brief 向I2C从设备发送数据,查询方式。
 *
 * 使用I2C总线向指定的从设备发送数据。
 *
 * @param I2Cx I2C模块指针，指向具体的I2C模块
 * @param slaveAddr 从设备地址，支持7位和10位地址
 * @param pu8Data 指向要发送的数据的指针
 * @param u16Len 要发送的数据长度
 *
 * @return 发送状态，成功返回I2C_Status_Success，失败返回I2C_Status_Error或其他错误码
 */
uint8_t I2C_MasterSendDataToSlave(I2C_TypeDef *I2Cx, uint16_t slaveAddr, uint8_t *pu8Data, uint16_t u16Len)
{
    uint8_t u8State;

    // 检查输入参数的有效性
    if (I2Cx == NULL || pu8Data == NULL || u16Len == 0 || slaveAddr > MAX_SLAVE_ADDR_10B)
    {
        return I2C_Status_Error;
    }

    // 根据从机地址长度选择对应的起始信号和地址发送函数
    if (slaveAddr & I2C_SLAVE_ADDR10B_MSB_MASK)
    {
        // 10-bit地址
        u8State = I2C_MasterSTART_Addr10b(I2Cx, slaveAddr, I2C_MODE_WRITE);
    }
    else
    {
        // 7-bit地址
        u8State = I2C_MasterSTART_Addr7b(I2Cx, (uint8_t)slaveAddr, I2C_MODE_WRITE);
    }

    if (u8State != I2C_Status_Success)
    {
        return u8State;
    }

    // 发送数据: N Byte address + User data, N = 0 ~ 2.
    u8State = I2C_MasterSend(I2Cx, pu8Data, u16Len);

    if (u8State != I2C_Status_Success)
    {
        // 出错时发送停止信号
        I2C_MasterSTOP(I2Cx);
        return u8State;
    }

    // 发送停止信号并返回成功状态
    u8State = I2C_MasterSTOP(I2Cx);
    return u8State;
}

/**
 * @brief 从I2C从设备接收数据
 *
 * 该函数通过I2C接口从指定的从设备读取数据:顺序读方式
 *
 * @param I2Cx I2C模块指针
 * @param slaveAddr 从设备地址
 * @param pu8RegAddr 数据寄存器的地址指针
 * @param regAddrType 寄存器地址类型（8位或16位）
 * @param pu8Data 接收数据的缓冲区指针
 * @param u16Len 要接收的数据长度
 *
 * @return 返回I2C操作状态，成功返回I2C_Status_Success，否则返回相应的错误代码
 */
uint8_t I2C_MasterReceiveDataFromSlave(I2C_TypeDef *I2Cx, uint16_t slaveAddr, uint8_t *pu8RegAddr, I2C_SlaveRegAddrTypeDef regAddrType, uint8_t *pu8Data, uint16_t u16Len)
{
    uint8_t u8State;
    uint8_t j;

    // 检查输入参数的有效性
    if (I2Cx == NULL || pu8Data == NULL || u16Len == 0 || slaveAddr > MAX_SLAVE_ADDR_10B || pu8RegAddr == NULL)
    {
        return I2C_Status_Error;
    }

    // 根据从机地址长度选择对应的起始信号和地址发送函数
    if (slaveAddr & I2C_SLAVE_ADDR10B_MSB_MASK)
    {
        // 10-bit地址
        u8State = I2C_MasterSTART_Addr10b(I2Cx, slaveAddr, I2C_MODE_WRITE);
    }
    else
    {
        // 7-bit地址
        u8State = I2C_MasterSTART_Addr7b(I2Cx, (uint8_t)slaveAddr, I2C_MODE_WRITE);
    }

    if (u8State != I2C_Status_Success)
    {
        return u8State;
    }

    // 发送Data地址
    switch (regAddrType)
    {
    case I2C_REG_ADDR_8BIT:
        j = 1;
        break;
    case I2C_REG_ADDR_16BIT:
        j = 2;
        break;
    case I2C_REG_ADDR_24BIT:
        j = 3;
        break;
    case I2C_REG_ADDR_32BIT:
        j = 4;
        break;
    default:
        return I2C_Status_Error;
    }

    u8State = I2C_MasterSend(I2Cx, pu8RegAddr, j);
    if (u8State != I2C_Status_Success)
    {
        // 出错时发送停止信号
        I2C_MasterSTOP(I2Cx);
        return I2C_Status_Error;
    }

#if 1
    /*
    *Tip: It is advisable to adjust the delay-time prior to issuing the I2C ReSTART-cmd based on factors such as:
    *the MCU's HCLK, the I2C's bps, the compiler used, and the level of optimization.
    */
    FirmwareDelay(16000); /* ~0.5ms @96MHz, 重复起始前留够地址字节时间 */
#endif

    // 发送重复起始信号和从机地址（读模式）
    if (slaveAddr & I2C_SLAVE_ADDR10B_MSB_MASK)
    {
        // 10-bit地址
        u8State = I2C_MasterReSTART_Addr10b(I2Cx, slaveAddr, I2C_MODE_READ);
    }
    else
    {
        // 7-bit地址
        u8State = I2C_MasterReSTART_Addr7b(I2Cx, (uint8_t)slaveAddr, I2C_MODE_READ);
    }

    if (u8State != I2C_Status_Success)
    {
        return u8State;
    }

    // 接收数据
    u8State = I2C_MasterReceive(I2Cx, pu8Data, u16Len);

    if (u8State != I2C_Status_Success)
    {
        // 出错时发送停止信号
        I2C_MasterSTOP(I2Cx);
        return I2C_Status_Error;
    }

    // 发送停止信号并返回成功状态
    u8State = I2C_MasterSTOP(I2Cx);
    return u8State;
}

/**
 * @brief I2C从设备接收数据
 *
 * 指定的I2C从设备接收数据，并将接收到的数据存储到提供的缓冲区中。
 *
 * @param I2Cx I2C设备指针
 * @param rxBuff 存储接收数据的缓冲区指针
 * @param rxSize 存储接收数据量的指针
 *
 * @return 接收状态，成功返回I2C_Status_Success，错误则返回相应的错误状态
 * - I2C_Status_Error：函数参数无效
 * - I2C_Status_FifoError：缓冲区即将溢出
 * - I2C_Status_Timeout：接收超时
 */
uint8_t I2C_SlaveReceive(I2C_TypeDef *I2Cx, uint8_t *rxBuff, uint16_t *rxSize)
{
    uint8_t *buf;
    uint32_t cntRx = 0;
    uint32_t timeout = I2C_WAIT_TIMEOUT; // 使用定义的超时值，便于调整

    if ((I2Cx == NULL) || (rxBuff == NULL) || (rxSize == NULL))
    {
        return I2C_Status_Error;
    }

    buf = rxBuff;

    while (timeout--)
    {
        uint8_t flagStatus = I2C_SlavePollingFlag(I2Cx, I2C_IT_RXNE_SLV | I2C_IT_STOP_SLV | I2C_IT_RESTART_SLV);

        if (flagStatus == I2C_Status_Success)
        {
            if (I2C_SlaveGetFlag(I2Cx, I2C_IT_RXNE_SLV))
            {
                if (cntRx >= MAX_DATA_LEN_RX)
                {
                    // 缓冲区即将溢出，停止接收并返回错误
                    *rxSize = cntRx;
                    return I2C_Status_FifoError;
                }

                *buf++ = (uint8_t)I2Cx->SRDR;
                cntRx++;
            }
            else
            {
                // 存在 STOP 或 RESTART 标志位，接收结束
                *rxSize = cntRx;
                return I2C_Status_Success;
            }
        }
        else if (timeout == 0)
        {
            // 超时，返回当前接收的数据量
            *rxSize = cntRx;
            return I2C_Status_Timeout;
        }
    }

    // 如果退出循环不是因为成功接收或超时，则返回错误状态
    *rxSize = cntRx;
    return I2C_Status_Error;
}

/**
 * @brief I2C从设备发送数据
 *
 * 该函数用于从设备通过指定的I2C总线向主机发送数据。
 *
 * @param I2Cx I2C总线实例指针
 * @param txBuff 指向要发送数据的缓冲区指针
 * @param txSize 要发送的数据字节数
 *
 * @return 发送状态，成功返回I2C_Status_Success，失败返回相应的错误代码
 *         - I2C_Status_Error: 输入参数无效
 *         - I2C_Status_Timeout: 发送数据时发生超时
 */
uint8_t I2C_SlaveSend(I2C_TypeDef *I2Cx, uint8_t *txBuff, uint16_t txSize)
{
    uint8_t *buf;

    // 检查输入参数
    if (txBuff == NULL || txSize == 0)
    {
        return I2C_Status_Error;
    }

    buf = txBuff;

    while (txSize--)
    {
        // 等待TXE标志位被置位，表示发送缓冲区空闲
        if (I2C_SlavePollingFlag(I2Cx, I2C_IT_TXE_SLV) != I2C_Status_Success)
        {
            return I2C_Status_Timeout; // 超时错误
        }

        // 发送数据
        I2Cx->STDR = *buf++;
    }

    return I2C_Status_Success;
}

/**
 * @brief 清除I2C从机标志位
 *
 * 该函数用于清除指定的I2C从机标志位。
 *
 * @param I2Cx 指向I2C外设的指针
 * @param I2C_FLAG 要清除的I2C从机标志位，可以是以下值之一及其组合：
 *                 - I2C_IT_FIFO_SLV
 *                 - I2C_IT_BIT_SLV
 *                 - I2C_IT_STOP_SLV
 *                 - I2C_IT_RESTART_SLV
 *
 * @return 无返回值
 */
void I2C_SlaveClearFlag(I2C_TypeDef *I2Cx, uint32_t I2C_FLAG)
{
    assert_param(IS_I2C_ALL_PERIPH(I2Cx));
    assert_param(IS_I2C_CLEAR_FLAG_SLV(I2C_FLAG));

    REGBITS_CLR(I2Cx->SICR, I2C_FLAG);
}

/**
 * @brief 轮询I2C从机状态标志位
 *
 * 此函数用于轮询指定的I2C从机状态标志位，并根据输入参数决定是等待标志位被设置还是清除。
 *
 * @param I2Cx I2C外设指针
 * @param I2C_FLAG 需要轮询的标志位，可以是以下值之一及其组合：
 *                 - I2C_IT_BUSBUSY_SLV
 *                 - I2C_IT_SLVBUSY_SLV
 *                 - I2C_IT_ALERT_SLV
 *                 - I2C_IT_GC_SLV
 *                 - I2C_IT_AM1_SLV
 *                 - I2C_IT_AM0_SLV
 *                 - I2C_IT_FIFO_SLV
 *                 - I2C_IT_BIT_SLV
 *                 - I2C_IT_STOP_SLV
 *                 - I2C_IT_RESTART_SLV
 *                 - I2C_IT_TACK_SLV
 *                 - I2C_IT_ADDR_SLV
 *                 - I2C_IT_RXNE_SLV
 *                 - I2C_IT_TXE_SLV
 *
 * @param reverse 标志位检测逻辑：TRUE表示等待标志位清除，FALSE表示等待标志位设置
 *
 * @return 返回状态码：
 *         - I2C_Status_Success：成功检测到标志位状态
 *         - I2C_Status_Timeout：轮询超时，未检测到标志位状态
 */
static uint8_t I2C_SlavePollingFlagCommon(I2C_TypeDef *I2Cx, uint32_t I2C_FLAG, boolean_t reverse)
{
    assert_param(IS_I2C_ALL_PERIPH(I2Cx));
    assert_param(IS_I2C_FLAG_SLV(I2C_FLAG));

    for (uint32_t waitTimes = I2C_WAIT_TIMEOUT; waitTimes > 0; --waitTimes)
    {
        uint32_t regVal = I2Cx->SISR;
        uint32_t flagDetected = reverse ? !(regVal & I2C_FLAG) : (regVal & I2C_FLAG);

        if (flagDetected)
        {
            return I2C_Status_Success;
        }
    }

    return I2C_Status_Timeout;
}

/**
 * @brief 轮询从机I2C标志位
 *
 * 轮询指定的I2C标志位，如果标志位被设置则返回1，否则返回0。
 *
 * @param I2Cx I2C接口指针
 * @param I2C_FLAG 需要轮询的标志位
 *
 * @return 如果标志位被设置，则返回I2C_Status_Success；否则返回I2C_Status_Timeout
 */
uint8_t I2C_SlavePollingFlag(I2C_TypeDef *I2Cx, uint32_t I2C_FLAG)
{
    return I2C_SlavePollingFlagCommon(I2Cx, I2C_FLAG, FALSE);
}

/**
 * @brief 反转I2C从设备轮询标志位
 *
 * 该函数用于反转I2C从设备的轮询标志位。
 *
 * @param I2Cx I2C外设指针
 * @param I2C_FLAG 要轮询的标志位
 *
 * @return 反转后的标志位状态，I2C_Status_Success表示未触发，I2C_Status_Timeout表示已触发
 */
uint8_t I2C_SlavePollingFlagReverse(I2C_TypeDef *I2Cx, uint32_t I2C_FLAG)
{
    return I2C_SlavePollingFlagCommon(I2Cx, I2C_FLAG, TRUE);
}

/**
 * @brief 获取I2C从设备标志位状态
 *
 * 根据提供的I2C外设和标志位，获取并返回该标志位的状态。
 *
 * @param I2Cx 指向I2C外设的指针
 * @param I2C_FLAG 要检查的I2C从设备标志位
 *
 * @return 如果标志位被设置，则返回非零值；否则返回0
 *
 */
uint32_t I2C_SlaveGetFlag(I2C_TypeDef *I2Cx, uint32_t I2C_FLAG)
{
    assert_param(IS_I2C_ALL_PERIPH(I2Cx));
    assert_param(IS_I2C_FLAG_SLV(I2C_FLAG));

    // 获取并返回指定的标志位状态
    return (I2Cx->SISR & I2C_FLAG);
}

/**
 * @brief 设置 I2C 从机在发送响应阶段是否发出 NACK
 *
 * @param I2Cx 指向 I2C 模块的指针
 * @param stateNAck 是否发出 NACK 的状态
 *     - ENABLE: 从机在发送响应阶段发出 NACK
 *     - DISABLE: 从机在发送响应阶段发出 ACK
 */
void I2C_SlaveTxNAck(I2C_TypeDef *I2Cx, FunctionalState stateNAck)
{
    if (stateNAck == ENABLE)
    {
        REGBITS_SET(I2Cx->STAR, I2Cx_STAR_TXNACK_Msk); // 从机在发送响应阶段，发出 NACK
    }
    else
    {
        REGBITS_CLR(I2Cx->STAR, I2Cx_STAR_TXNACK_Msk); // 从机在发送响应阶段，发出 ACK
    }
}

/**
 * @brief 根据给定的时钟频率、脉冲宽度、最小和最大周期数以及预分频器计算I2C总线上的周期数
 *
 * @param sourceClock_Hz 源时钟频率（Hz）
 * @param width_ns 脉冲宽度（纳秒）
 * @param minCycles 最小周期数
 * @param maxCycles 最大周期数
 * @param prescaler 预分频器值
 *
 * @return 计算出的周期数
 *
 * @note 确保sourceClock_Hz大于0
 */
static uint32_t I2C_GetCyclesForWidth(
    uint32_t sourceClock_Hz, uint32_t width_ns, uint32_t minCycles, uint32_t maxCycles, uint32_t prescaler)
{
    assert_param(sourceClock_Hz > 0U);

    uint32_t divider = 1U;

    while (prescaler != 0U)
    {
        divider *= 2U;
        prescaler--;
    }

    uint32_t busCycle_ns = 1000000U / (sourceClock_Hz / divider / 1000U);
    /* Calculate the cycle count, round up the calculated value. */
    uint32_t cycles = (width_ns * 10U / busCycle_ns + 5U) / 10U;

    /* If the calculated value is smaller than the minimum value, use the minimum value */
    if (cycles < minCycles)
    {
        cycles = minCycles;
    }

    /* If the calculated value is larger than the maximum value, use the maxmum value */
    if (cycles > maxCycles)
    {
        cycles = maxCycles;
    }

    return cycles;
}

/**
 * @brief 设置I2C主机模式的波特率
 *
 * @param I2Cx I2C接口指针，指向I2C_TypeDef结构体
 * @param sourceClock_Hz I2C接口时钟源频率（单位：Hz）
 * @param baudRate_Hz 期望的波特率（单位：Hz）
 *
 * @note 在设置波特率之前，会先禁用I2C主模式，设置完成后会恢复I2C主模式。
 */
void I2C_MasterSetBaudRate(I2C_TypeDef *I2Cx, uint32_t sourceClock_Hz, uint32_t baudRate_Hz)
{
    boolean_t wasEnabled;
    uint8_t filtScl = (uint8_t)((I2Cx->MCR3 & I2Cx_MCR3_FLTSCL_Msk) >> I2Cx_MCR3_FLTSCL_Pos);

    uint8_t divider = 1U;
    uint8_t bestDivider = 1U;
    uint8_t prescale = 0U;
    uint8_t bestPre = 0U;

    uint8_t clkCycle;
    uint8_t bestclkCycle = 0U;

    uint32_t absError = 0U;
    uint32_t bestError = 0xffffffffu;
    uint32_t computedRate;

    uint32_t tmpReg = 0U;

    /* Disable master mode. */
    wasEnabled = (0U != ((I2Cx->MCR0 & I2Cx_MCR0_MEN_Msk) >> I2Cx_MCR0_MEN_Pos));
    I2C_MasterCmd(I2Cx, DISABLE);

    for (prescale = 0U; prescale <= 7U; prescale++)
    {
        /* Calculate the clkCycle, clkCycle = CLKLO + CLKHI, divider = 2 ^ prescale */
        clkCycle = (uint8_t)((10U * sourceClock_Hz / divider / baudRate_Hz + 5U) / 10U - (2U + filtScl) / divider - 2U);

        if (clkCycle > (120U - (2U + filtScl) / divider))
        {
            divider *= 2U;
            continue;
        }

        /* Calculate the computed baudrate and compare it with the desired baudrate */
        computedRate = (sourceClock_Hz / (uint32_t)divider) /
                       ((uint32_t)clkCycle + 2U + (2U + (uint32_t)filtScl) / (uint32_t)divider);
        absError = baudRate_Hz > computedRate ? baudRate_Hz - computedRate : computedRate - baudRate_Hz;

        if (absError < bestError)
        {
            bestPre = prescale;
            bestDivider = divider;
            bestclkCycle = clkCycle;
            bestError = absError;

            /* If the error is 0, then we can stop searching because we won't find a better match. */
            if (absError == 0U)
            {
                break;
            }
        }

        divider *= 2U;
    }

    uint8_t tmpHigh = (bestclkCycle - (2U + filtScl) / bestDivider) / 2U;

    while (tmpHigh > (bestclkCycle - 52U * sourceClock_Hz / baudRate_Hz / bestDivider / 100U + 1U))
    {
        tmpHigh = tmpHigh - 1U;
    }

    /* Calculate DATAVD and SETHOLD.
       To meet the timing requirement of I2C spec for standard mode, fast mode and fast mode plus: */
    uint8_t tmpHold = (uint8_t)(sourceClock_Hz / baudRate_Hz / bestDivider / 2U) - 1U;

    uint8_t tmpDataVd = (uint8_t)(sourceClock_Hz / baudRate_Hz / bestDivider / 4U) - 1U;

    if ((sourceClock_Hz / baudRate_Hz / 20U) > (bestDivider + 2U))
    {
        /* Read out the FILTSDA configuration, if it is smaller than expected, change the setting. */
        uint8_t filtSda = (uint8_t)((I2Cx->MCR3 & I2Cx_MCR3_FLTSDA_Msk) >> I2Cx_MCR3_FLTSDA_Pos);

        if (filtSda < (sourceClock_Hz / baudRate_Hz / 20U - bestDivider - 2U))
        {
            filtSda = (uint8_t)(sourceClock_Hz / baudRate_Hz / 20U) - bestDivider - 2U;
        }

        I2Cx->MCR3 = (I2Cx->MCR3 & ~I2Cx_MCR3_FLTSDA_Msk) | I2C_MCR3_FILTSDA(filtSda);
    }

    /* Set CLKHI, CLKLO, SETHOLD, DATAVD value. */
    tmpReg = I2C_MCCR0_CLKHI((uint32_t)tmpHigh) |
             I2C_MCCR0_CLKLO((uint32_t)((uint32_t)bestclkCycle - (uint32_t)tmpHigh)) |
             I2C_MCCR0_SETHOLD((uint32_t)tmpHold) | I2C_MCCR0_DATAVD((uint32_t)tmpDataVd);
    I2Cx->MCCR = tmpReg;

    /* Set PRESCALE value. */
    I2Cx->MCR2 = (I2Cx->MCR2 & ~I2Cx_MCR2_PRESCALE_Msk) | I2C_MCR2_PRESCALE(bestPre);

    /* Restore master mode. */
    if (wasEnabled)
    {
        I2C_MasterCmd(I2Cx, ENABLE);
    }
}

/**
 * @brief 获取LPI2C实例编号
 *
 * 根据给定的I2C类型指针获取其对应的LPI2C实例编号。
 *
 * @param I2Cx I2C类型指针
 *
 * @return 返回LPI2C实例编号，如果找不到对应的实例则返回超出范围的编号
 */
uint32_t LPI2C_GetInstance(I2C_TypeDef *I2Cx)
{
    uint32_t instance;
    for (instance = 0U; instance < ARRAY_SZ(kLpi2cBases); ++instance)
    {
        if (kLpi2cBases[instance] == I2Cx)
        {
            break;
        }
    }

    assert_param(instance < ARRAY_SZ(kLpi2cBases));
    return instance;
}

/**
 * @brief 检查并清除LPI2C主机错误
 *
 * 检查并清除LPI2C主机错误。在发生错误时，将自动发送停止信号。必须清除错误才能开始新的传输。
 *
 * @param I2Cx LPI2C模块指针
 * @param status LPI2C状态标志
 *
 * @return 返回状态码。成功返回kStatus_Success，错误返回对应的错误码。
 */
status_t LPI2C_MasterCheckAndClearError(I2C_TypeDef *I2Cx, uint32_t status)
{
    status_t result = kStatus_Success;

    /* Check for error. These errors cause a stop to automatically be sent. We must */
    /* clear the errors before a new transfer can start. */
    status &= (uint32_t)kLPI2C_MasterErrorFlags;
    if (0U != status)
    {
        /* Select the correct error code. Ordered by severity, with bus issues first. */
        if (0U != (status & (uint32_t)kLPI2C_MasterPinLowTimeoutFlag))
        {
            result = kStatus_LPI2C_PinLowTimeout;
        }
        else if (0U != (status & (uint32_t)kLPI2C_MasterArbitrationLostFlag))
        {
            result = kStatus_LPI2C_ArbitrationLost;
        }
        else if (0U != (status & (uint32_t)kLPI2C_MasterNackDetectFlag))
        {
            result = kStatus_LPI2C_Nak;
        }
        else if (0U != (status & (uint32_t)kLPI2C_MasterFifoErrFlag))
        {
            result = kStatus_LPI2C_FifoError;
        }
        else
        {
            ; /* Intentional empty */
        }

        /* Clear the flags. */
        I2C_MasterClearFlag(I2Cx, status);

        /* Reset fifos. These flags clear automatically. */
        I2Cx->MCR0 |= I2Cx_MCR0_RXFIFORST_Msk | I2Cx_MCR0_TXFIFORST_Msk;
    }
    else
    {
        ; /* Intentional empty */
    }

    return result;
}

/**
 * @brief 等待LPI2C主机发送缓冲区准备就绪
 *
 * 该函数等待直到LPI2C主机的发送缓冲区准备就绪，或者发生错误。
 *
 * @param I2Cx I2C外设地址
 *
 * @return 返回函数执行状态，成功返回kStatus_Success，其他值表示错误。
 */
static status_t LPI2C_MasterWaitForTxReady(I2C_TypeDef *I2Cx)
{
    status_t result = kStatus_Success;
    uint32_t status;
    size_t txCount;
    size_t txFifoSize = (size_t)FSL_FEATURE_I2C_FIFO_SIZEn(I2Cx);

#if I2C_RETRY_TIMES != 0U
    uint32_t waitTimes = I2C_RETRY_TIMES;
#endif
    do
    {
        /* Get the number of words in the tx fifo and compute empty slots. */
        LPI2C_MasterGetFifoCounts(I2Cx, NULL, &txCount);
        txCount = txFifoSize - txCount;

        /* Check for error flags. */
        status = LPI2C_MasterGetStatusFlags(I2Cx);
        result = LPI2C_MasterCheckAndClearError(I2Cx, status);
        if (kStatus_Success != result)
        {
            break;
        }
#if I2C_RETRY_TIMES != 0U
        waitTimes--;
    } while ((0U == txCount) && (0U != waitTimes));

    if (0U == waitTimes)
    {
        result = kStatus_LPI2C_Timeout;
    }
#else
    } while (0U == txCount);
#endif

    return result;
}

/**
 * @brief 检查I2C总线是否繁忙
 *
 * 检查指定的I2C总线是否处于繁忙状态。
 *
 * @param I2Cx I2C模块指针
 *
 * @return 返回状态码
 *         - kStatus_Success: 总线不繁忙
 *         - kStatus_LPI2C_Busy: 总线繁忙
 */
status_t LPI2C_CheckForBusyBus(I2C_TypeDef *I2Cx)
{
    status_t ret = kStatus_Success;

    uint32_t status = LPI2C_MasterGetStatusFlags(I2Cx);
    if ((0U != (status & (uint32_t)kLPI2C_MasterBusBusyFlag)) && (0U == (status & (uint32_t)kLPI2C_MasterBusyFlag)))
    {
        ret = kStatus_LPI2C_Busy;
    }

    return ret;
}

/**
 * @brief I2C 主模式去初始化函数
 *
 * 该函数用于将指定的 I2C 端口从主模式去初始化，恢复到复位状态。
 *
 * @param I2Cx I2C 端口指针，指向要操作的 I2C 端口。
 */
void I2C_MasterDeinit(I2C_TypeDef *I2Cx)
{
    /* Restore to reset state. */
    I2C_MasterSoftReset(I2Cx);

    I2C_DeInit(I2Cx);
}

/**
 * @brief 配置I2C主机模式的数据匹配参数
 *
 * 该函数用于配置I2C主机模式的数据匹配参数。
 *
 * @param I2Cx I2C模块指针，指向要配置的I2C模块
 * @param matchConfig 指向数据匹配配置参数的指针
 */
void LPI2C_MasterConfigureDataMatch(I2C_TypeDef *I2Cx, const lpi2c_data_match_config_t *matchConfig)
{
    /* Disable master mode. */
    boolean_t wasEnabled = (0U != ((I2Cx->MCR0 & I2Cx_MCR0_MEN_Msk) >> I2Cx_MCR0_MEN_Pos));
    I2C_MasterCmd(I2Cx, DISABLE);

    I2Cx->MCR2 = (I2Cx->MCR2 & ~I2Cx_MCR2_MATCFG_Pos) | I2C_MCR2_MATCFG(matchConfig->matchMode);
    I2Cx->MCR1 = (I2Cx->MCR1 & ~I2Cx_MCR1_RDMO_Msk) | I2C_MCR1_RDMO(matchConfig->rxDataMatchOnly);
    I2Cx->MMATCH   = I2C_MDMR_MATCH0(matchConfig->match0) | I2C_MDMR_MATCH1(matchConfig->match1);

    /* Restore master mode. */
    if (wasEnabled)
    {
        I2C_MasterCmd(I2Cx, ENABLE);
    }
}

/**
 * @brief 主设备停止I2C总线传输
 *
 * 停止指定的I2C总线上的数据传输。
 *
 * @param I2Cx I2C设备指针
 *
 * @return 返回状态码，成功返回kStatus_Success，失败返回相应的错误码
 */
status_t LPI2C_MasterStop(I2C_TypeDef *I2Cx)
{
    /* Wait until there is room in the fifo. */
    status_t result = LPI2C_MasterWaitForTxReady(I2Cx);
    if (kStatus_Success == result)
    {
        /* Send the STOP signal */
        I2Cx->MTDR = (uint32_t)kStopCmd;

        /* Wait for the stop detected flag to set, indicating the transfer has completed on the bus. */
        /* Also check for errors while waiting. */
#if I2C_RETRY_TIMES != 0U
        uint32_t waitTimes = I2C_RETRY_TIMES;
#endif

#if I2C_RETRY_TIMES != 0U
        while ((result == kStatus_Success) && (0U != waitTimes))
        {
            waitTimes--;
#else
        while (result == kStatus_Success)
        {
#endif
            uint32_t status = LPI2C_MasterGetStatusFlags(I2Cx);

            /* Check for error flags. */
            result = LPI2C_MasterCheckAndClearError(I2Cx, status);

            /* Check if the stop was sent successfully. */
            if ((0U != (status & (uint32_t)kLPI2C_MasterStopDetectFlag)) &&
                (0U != (status & (uint32_t)kLPI2C_MasterTxReadyFlag)))
            {
                I2C_MasterClearFlag(I2Cx, (uint32_t)kLPI2C_MasterStopDetectFlag);
                break;
            }
        }

#if I2C_RETRY_TIMES != 0U
        if (0U == waitTimes)
        {
            result = kStatus_LPI2C_Timeout;
        }
#endif
    }

    return result;
}

/**
 * @brief 主机从I2C总线上接收数据
 *
 * 从指定的I2C总线上接收数据，并存储到提供的缓冲区中。
 *
 * @param I2Cx I2C设备指针
 * @param rxBuff 用于存储接收到的数据的缓冲区指针
 * @param rxSize 需要接收的数据大小
 *
 * @return 返回状态码，成功返回kStatus_Success，失败返回相应的错误码
 */
status_t LPI2C_MasterReceive(I2C_TypeDef *I2Cx, void *rxBuff, size_t rxSize)
{
    assert_param(NULL != rxBuff);

    status_t result = kStatus_Success;
    uint8_t *buf;
    size_t tmpRxSize = rxSize;
#if I2C_RETRY_TIMES != 0U
    uint32_t waitTimes;
#endif

    /* Check transfer data size. */
    if (rxSize > (256UL * (uint32_t)FSL_FEATURE_I2C_FIFO_SIZEn(I2Cx)))
    {
        return kStatus_InvalidArgument;
    }

    /* Handle empty read. */
    if (rxSize != 0U)
    {
        /* Wait until there is room in the command fifo. */
        result = LPI2C_MasterWaitForTxReady(I2Cx);
        if (kStatus_Success == result)
        {
            /* Issue command to receive data. A single write to MTDR can issue read operation of 0xFFU + 1 byte of data
               at most, so when the rxSize is larger than 0x100U, push multiple read commands to MTDR until rxSize is
               reached. */
            while (tmpRxSize != 0U)
            {
                if (tmpRxSize > 256U)
                {
                    I2Cx->MTDR = (uint32_t)(kRxDataCmd) | (uint32_t)I2C_MTDR_DATA(0xFFU);
                    tmpRxSize -= 256U;
                }
                else
                {
                    I2Cx->MTDR = (uint32_t)(kRxDataCmd) | (uint32_t)I2C_MTDR_DATA(tmpRxSize - 1U);
                    tmpRxSize  = 0U;
                }
            }

            /* Receive data */
            buf = (uint8_t *)rxBuff;
            while (0U != (rxSize--))
            {
#if I2C_RETRY_TIMES != 0U
                waitTimes = I2C_RETRY_TIMES;
#endif
                /* Read LPI2C receive fifo register. The register includes a flag to indicate whether */
                /* the FIFO is empty, so we can both get the data and check if we need to keep reading */
                /* using a single register read. */
                uint32_t value = 0U;
                do
                {
                    /* Check for errors. */
                    result = LPI2C_MasterCheckAndClearError(I2Cx, LPI2C_MasterGetStatusFlags(I2Cx));
                    if (kStatus_Success != result)
                    {
                        break;
                    }

                    value = I2Cx->MRDR;
#if I2C_RETRY_TIMES != 0U
                    waitTimes--;
                } while ((0U != (value & I2Cx_MRDR_EMPTY_Msk)) && (0U != waitTimes));
                if (0U == waitTimes)
                {
                    result = kStatus_LPI2C_Timeout;
                }
#else
                } while (0U != (value & I2Cx_MRDR_EMPTY_Msk));
#endif
                if ((status_t)kStatus_Success != result)
                {
                    break;
                }

                *buf++ = (uint8_t)(value & I2Cx_MRDR_DATA_Msk);
            }
        }
    }

    return result;
}

/**
 * @brief 主机向 I2C 总线发送数据
 *
 * 该函数用于通过 I2C 总线发送数据。
 *
 * @param I2Cx I2C 控制器实例指针
 * @param txBuff 指向要发送的数据缓冲区的指针
 * @param txSize 要发送的数据大小（字节数）
 *
 * @return 发送操作的状态码。如果成功，返回 kStatus_Success；否则返回相应的错误代码。
 */
status_t LPI2C_MasterSend(I2C_TypeDef *I2Cx, void *txBuff, size_t txSize)
{
    status_t result = kStatus_Success;
    uint8_t *buf    = (uint8_t *)txBuff;

    assert_param(NULL != txBuff);

    /* Send data buffer */
    while (0U != (txSize--))
    {
        /* Wait until there is room in the fifo. This also checks for errors. */
        result = LPI2C_MasterWaitForTxReady(I2Cx);
        if (kStatus_Success != result)
        {
            break;
        }

        /* Write byte into LPI2C master data register. */
        I2Cx->MTDR = *buf++;
    }

    return result;
}

/**
 * @brief 主机在阻塞模式下通过I2C总线进行主从设备数据传输
 *
 * 该函数在阻塞模式下，通过I2C总线进行主从设备之间的数据传输。它支持读写操作，
 * 并处理数据传输过程中的各种细节，如子地址的发送、数据传输的开始和结束等。
 *
 * @param I2Cx I2C模块的外设基地址指针
 * @param transfer 指向传输配置的指针，包含传输的方向、数据大小、子地址等信息
 *
 * @return 传输状态，成功返回kStatus_Success，失败返回相应的错误代码
 */
status_t LPI2C_MasterTransferBlocking(I2C_TypeDef *I2Cx, lpi2c_master_transfer_t *transfer)
{
    assert_param(NULL != transfer);
    assert_param(transfer->subaddressSize <= sizeof(transfer->subaddress));

    status_t result = kStatus_Success;
    uint16_t commandBuffer[7];
    uint32_t cmdCount = 0U;

    /* Check transfer data size in read operation. */
    if ((transfer->direction == I2C_MODE_READ) &&
        (transfer->dataSize > (256UL * (uint32_t)FSL_FEATURE_I2C_FIFO_SIZEn(I2Cx))))
    {
        return kStatus_InvalidArgument;
    }

    /* Return an error if the bus is already in use not by us. */
    result = LPI2C_CheckForBusyBus(I2Cx);
    if (kStatus_Success == result)
    {
        /* Clear all flags. */
        I2C_MasterClearFlag(I2Cx, (uint32_t)kLPI2C_MasterClearFlags);

        /* Turn off auto-stop option. */
        I2Cx->MCR2 &= ~I2Cx_MCR2_AUTOSTOP_Msk;

        I2C_ModeTypeDef direction = (0U != transfer->subaddressSize) ? I2C_MODE_WRITE : transfer->direction;
        if (0U == (transfer->flags & (uint32_t)kLPI2C_TransferNoStartFlag))
        {
            if (transfer->slaveAddress & I2C_SLAVE_ADDR10B_MSB_MASK)
            {
                //10-bit slave address
                //slave MSB address
                commandBuffer[cmdCount++] =
                    (uint16_t)kStartCmd |
                        (uint16_t)((uint16_t)((((uint16_t)transfer->slaveAddress & I2C_SLAVE_ADDR10B_MSB_MASK) >> (8 - 1)) | I2C_SLAVE_ADDR10B_MSB) | (uint16_t)direction);

                //slave LSB address
                commandBuffer[cmdCount++] =
                    (uint16_t)((uint16_t)transfer->slaveAddress & I2C_SLAVE_ADDR10B_LSB_MASK);
            }
            else
            {
                //7-bit slave address
                commandBuffer[cmdCount++] =
                    (uint16_t)kStartCmd |
                        (uint16_t)((uint16_t)((uint16_t)transfer->slaveAddress << 1U) | (uint16_t)direction);
            }
        }

        /* Subaddress, MSB first. */
        if (0U != transfer->subaddressSize)
        {
            uint32_t subaddressRemaining = transfer->subaddressSize;
            while (0U != subaddressRemaining--)
            {
                uint8_t subaddressByte    = (uint8_t)((transfer->subaddress >> (8U * subaddressRemaining)) & 0xffU);
                commandBuffer[cmdCount++] = subaddressByte;
            }
        }

        /* Reads need special handling. */
        if ((0U != transfer->dataSize) && (transfer->direction == I2C_MODE_READ))
        {
            /* Need to send repeated start if switching directions to read. */
            if (direction == I2C_MODE_WRITE)
            {
                if (transfer->slaveAddress & I2C_SLAVE_ADDR10B_MSB_MASK)
                {
                    //10-bit slave address
                    //slave MSB address
                    commandBuffer[cmdCount++] =
                        (uint16_t)kStartCmd |
                            (uint16_t)((uint16_t)((((uint16_t)transfer->slaveAddress & I2C_SLAVE_ADDR10B_MSB_MASK) >> (8 - 1)) | I2C_SLAVE_ADDR10B_MSB) | (uint16_t)I2C_MODE_READ);
                }
                else
                {
                    //7-bit slave address
                    commandBuffer[cmdCount++] =
                        (uint16_t)kStartCmd |
                            (uint16_t)((uint16_t)((uint16_t)transfer->slaveAddress << 1U) | (uint16_t)I2C_MODE_READ);
                }
            }
        }

        /* Send command buffer */
        uint32_t index = 0U;
        while (0U != cmdCount--)
        {
            /* Wait until there is room in the fifo. This also checks for errors. */
            result = LPI2C_MasterWaitForTxReady(I2Cx);
            if (kStatus_Success != result)
            {
                break;
            }

            /* Write byte into LPI2C master data register. */
            I2Cx->MTDR = commandBuffer[index];
            index++;
        }

        if (kStatus_Success == result)
        {
            /* Transmit data. */
            if ((transfer->direction == I2C_MODE_WRITE) && (transfer->dataSize > 0U))
            {
                /* Send Data. */
                result = LPI2C_MasterSend(I2Cx, transfer->data, transfer->dataSize);
            }

            /* Receive Data. */
            if ((transfer->direction == I2C_MODE_READ) && (transfer->dataSize > 0U))
            {
                result = LPI2C_MasterReceive(I2Cx, transfer->data, transfer->dataSize);
            }

            if (kStatus_Success == result)
            {
                if ((transfer->flags & (uint32_t)kLPI2C_TransferNoStopFlag) == 0U)
                {
                    result = LPI2C_MasterStop(I2Cx);
                }
            }
        }
    }

    return result;
}

/**
 * @brief 创建I2C主设备传输句柄
 *
 * 该函数用于初始化并创建一个I2C主设备传输句柄，并设置相关的回调函数和用户数据。
 *
 * @param I2Cx I2C设备实例指针
 * @param handle 指向I2C主设备传输句柄的指针
 * @param callback I2C主设备传输完成的回调函数
 * @param userData 用户自定义数据指针，将传递给回调函数
 */
void LPI2C_MasterTransferCreateHandle(I2C_TypeDef *I2Cx,
                                      lpi2c_master_handle_t *handle,
                                      lpi2c_master_transfer_callback_t callback,
                                      void *userData)
{
    uint32_t instance;

    assert_param(NULL != handle);

    /* Clear out the handle. */
    (void)memset(handle, 0, sizeof(*handle));

    /* Look up instance number */
    instance = LPI2C_GetInstance(I2Cx);

    /* Save base and instance. */
    handle->completionCallback = callback;
    handle->userData           = userData;

    /* Save this handle for IRQ use. */
    s_lpi2cMasterHandle[instance] = handle;

    /* Set irq handler. */
    s_lpi2cMasterIsr = LPI2C_MasterTransferHandleIRQ;

    /* Clear internal IRQ enables and enable NVIC IRQ. */
    LPI2C_MasterDisableInterrupts(I2Cx, (uint32_t)kLPI2C_MasterIrqFlags);

    /* Enable NVIC IRQ, this only enables the IRQ directly connected to the NVIC.*/
    NVIC_ClearPendingIRQ(kLpi2cIrqs[instance]);
    NVIC_EnableIRQ(kLpi2cIrqs[instance]);
}

/**
 * @brief 运行I2C传输状态机
 *
 * 此函数用于控制I2C传输状态机，包括发送命令、发送/接收数据以及发送停止命令等状态。
 *
 * @param I2Cx I2C模块基地址指针
 * @param handle I2C主模式句柄指针
 * @param isDone 用于指示传输是否完成的布尔指针
 *
 * @return 返回状态码，kStatus_Success表示成功，其他值表示错误
 */
static status_t LPI2C_RunTransferStateMachine(I2C_TypeDef *I2Cx, lpi2c_master_handle_t *handle, boolean_t *isDone)
{
    uint32_t status;
    status_t result = kStatus_Success;
    lpi2c_master_transfer_t *xfer;
    size_t txCount;
    size_t rxCount;
    size_t txFifoSize = (size_t)FSL_FEATURE_I2C_FIFO_SIZEn(I2Cx);
    boolean_t state_complete = FALSE;
    uint16_t sendval;

    /* Set default isDone return value. */
    *isDone = FALSE;

    /* Check for errors. */
    status = LPI2C_MasterGetStatusFlags(I2Cx);

    /* Get fifo counts. */
    LPI2C_MasterGetFifoCounts(I2Cx, &rxCount, &txCount);

    /* Get pointer to private data. */
    xfer = &handle->transfer;

    /* For the last byte, nack flag is expected.
       Do not check and clear kLPI2C_MasterNackDetectFlag for the last byte,
       in case FIFO is emptied when stop command has not been sent. */
    if (handle->remainingBytes == 0U)
    {
        /* When data size is not zero which means it is not only one byte of address is sent, and */
        /* when the txfifo is empty, or have one byte which is the stop command, then the nack status can be ignored. */
        if ((xfer->dataSize != 0U) &&
            ((txCount == 0U) || ((txCount == 1U) && (handle->state == (uint8_t)kWaitForCompletionState) &&
                                 ((xfer->flags & (uint32_t)kLPI2C_TransferNoStopFlag) == 0U))))
        {
            status &= ~(uint32_t)kLPI2C_MasterNackDetectFlag;
        }
    }

    result = LPI2C_MasterCheckAndClearError(I2Cx, status);

    if (kStatus_Success == result)
    {
        /* Compute room in tx fifo */
        txCount = txFifoSize - txCount;

        while (!state_complete)
        {
            /* Execute the state. */
            switch (handle->state)
            {
                case (uint8_t)kSendCommandState:
                    /* Make sure there is room in the tx fifo for the next command. */
                    if (0U == txCount--)
                    {
                        state_complete = TRUE;
                        break;
                    }

                    /* Issue command. buf is a uint8_t* pointing at the uint16 command array. */
                    sendval    = ((uint16_t)handle->buf[0]) | (((uint16_t)handle->buf[1]) << 8U);
                    I2Cx->MTDR = sendval;
                    handle->buf++;
                    handle->buf++;

                    /* Count down until all commands are sent. */
                    if (--handle->remainingBytes == 0U)
                    {
                        /* Choose next state and set up buffer pointer and count. */
                        if (0U != xfer->dataSize)
                        {
                            /* Either a send or receive transfer is next. */
                            handle->state          = (uint8_t)kTransferDataState;
                            handle->buf            = (uint8_t *)xfer->data;
                            handle->remainingBytes = (uint16_t)xfer->dataSize;
                            if (xfer->direction == I2C_MODE_READ)
                            {
                                /* Disable TX interrupt */
                                LPI2C_MasterDisableInterrupts(I2Cx, (uint32_t)kLPI2C_MasterTxReadyFlag);
                                /* Issue command to receive data. A single write to MTDR can issue read operation of
                                   0xFFU + 1 byte of data at most, so when the dataSize is larger than 0x100U, push
                                   multiple read commands to MTDR until dataSize is reached. */
                                size_t tmpRxSize = xfer->dataSize;
                                while (tmpRxSize != 0U)
                                {
                                    LPI2C_MasterGetFifoCounts(I2Cx, NULL, &txCount);
                                    while (txFifoSize == txCount)
                                    {
                                        LPI2C_MasterGetFifoCounts(I2Cx, NULL, &txCount);
                                    }

                                    if (tmpRxSize > 256U)
                                    {
                                        I2Cx->MTDR = (uint32_t)(kRxDataCmd) | (uint32_t)I2C_MTDR_DATA(0xFFU);
                                        tmpRxSize -= 256U;
                                    }
                                    else
                                    {
                                        I2Cx->MTDR = (uint32_t)(kRxDataCmd) | (uint32_t)I2C_MTDR_DATA(tmpRxSize - 1U);
                                        tmpRxSize  = 0U;
                                    }
                                }
                            }
                        }
                        else
                        {
                            /* No transfer, so move to stop state. */
                            handle->state = (uint8_t)kStopState;
                        }
                    }
                    break;

                case (uint8_t)kIssueReadCommandState:
                    /* Make sure there is room in the tx fifo for the read command. */
                    if (0U == txCount--)
                    {
                        state_complete = TRUE;
                        break;
                    }

                    I2Cx->MTDR = (uint32_t)kRxDataCmd | I2C_MTDR_DATA(xfer->dataSize - 1U);

                    /* Move to transfer state. */
                    handle->state = (uint8_t)kTransferDataState;
                    if (xfer->direction == I2C_MODE_READ)
                    {
                        /* Disable TX interrupt */
                        LPI2C_MasterDisableInterrupts(I2Cx, (uint32_t)kLPI2C_MasterTxReadyFlag);
                    }
                    break;

                case (uint8_t)kTransferDataState:
                    if (xfer->direction == I2C_MODE_WRITE)
                    {
                        /* Make sure there is room in the tx fifo. */
                        if (0U == txCount--)
                        {
                            state_complete = TRUE;
                            break;
                        }

                        /* Put byte to send in fifo. */
                        I2Cx->MTDR = *(handle->buf)++;
                    }
                    else
                    {
                        /* XXX handle receive sizes > 256, use kIssueReadCommandState */
                        /* Make sure there is data in the rx fifo. */
                        if (0U == rxCount--)
                        {
                            state_complete = TRUE;
                            break;
                        }

                        /* Read byte from fifo. */
                        *(handle->buf)++ = (uint8_t)(I2Cx->MRDR & I2Cx_MRDR_DATA_Msk);
                    }

                    /* Move to stop when the transfer is done. */
                    if (--handle->remainingBytes == 0U)
                    {
                        if (xfer->direction == I2C_MODE_WRITE)
                        {
                            state_complete = TRUE;
                        }
                        handle->state = (uint8_t)kStopState;
                    }
                    break;

                case (uint8_t)kStopState:
                    /* Only issue a stop transition if the caller requested it. */
                    if ((xfer->flags & (uint32_t)kLPI2C_TransferNoStopFlag) == 0U)
                    {
                        /* Make sure there is room in the tx fifo for the stop command. */
                        if (0U == txCount--)
                        {
                            state_complete = TRUE;
                            break;
                        }

                        I2Cx->MTDR = (uint32_t)kStopCmd;
                    }
                    else
                    {
                        /* If all data is read and no stop flag is required to send, we are done. */
                        if (xfer->direction == I2C_MODE_READ)
                        {
                            *isDone = TRUE;
                        }
                        state_complete = TRUE;
                    }
                    handle->state = (uint8_t)kWaitForCompletionState;
                    break;

                case (uint8_t)kWaitForCompletionState:
                    if ((xfer->flags & (uint32_t)kLPI2C_TransferNoStopFlag) == 0U)
                    {
                        /* We stay in this state until the stop state is detected. */
                        if (0U != (status & (uint32_t)kLPI2C_MasterStopDetectFlag))
                        {
                            *isDone = TRUE;
                        }
                    }
                    else
                    {
                        /* If all data is pushed to FIFO and no stop flag is required to send, we need to make sure they
                           are all send out to bus. */
                        if ((xfer->direction == I2C_MODE_WRITE) && ((I2Cx->MFIFOSR & I2Cx_MFIFOSR_TXCNT_Msk) == 0U))
                        {
                            /* We stay in this state until the data is sent out to bus. */
                            *isDone = TRUE;
                        }
                    }
                    state_complete = TRUE;
                    break;
                default:
                    assert_param(FALSE);
                    break;
            }
        }
    }
    return result;
}

/**
 * @brief 初始化I2C传输状态机
 *
 * 该函数用于初始化I2C传输状态机，根据传入的参数设置传输的状态、数据和地址等信息。
 *
 * @param handle I2C传输句柄指针
 */
static void LPI2C_InitTransferStateMachine(lpi2c_master_handle_t *handle)
{
    lpi2c_master_transfer_t *xfer = &handle->transfer;

    /* Handle no start option. */
    if (0U != (xfer->flags & (uint32_t)kLPI2C_TransferNoStartFlag))
    {
        if (xfer->direction == I2C_MODE_READ)
        {
            /* Need to issue read command first. */
            handle->state = (uint8_t)kIssueReadCommandState;
        }
        else
        {
            /* Start immediately in the data transfer state. */
            handle->state = (uint8_t)kTransferDataState;
        }

        handle->buf            = (uint8_t *)xfer->data;
        handle->remainingBytes = (uint16_t)xfer->dataSize;
    }
    else
    {
        uint16_t *cmd     = (uint16_t *)&handle->commandBuffer;
        uint32_t cmdCount = 0U;

        /* Initial direction depends on whether a subaddress was provided, and of course the actual */
        /* data transfer direction. */
        I2C_ModeTypeDef direction = (0U != xfer->subaddressSize) ? I2C_MODE_WRITE : xfer->direction;

        /* Start command. */
        if(xfer->slaveAddress & I2C_SLAVE_ADDR10B_MSB_MASK)
        {
            // 10-bit slave address
            cmd[cmdCount++] =
                (uint16_t)kStartCmd | (uint16_t)((uint16_t)((((uint16_t)xfer->slaveAddress & I2C_SLAVE_ADDR10B_MSB_MASK) >> (8 - 1)) | I2C_SLAVE_ADDR10B_MSB) | (uint16_t)direction);
            cmd[cmdCount++] =
                (uint16_t)((uint16_t)((uint16_t)xfer->slaveAddress & I2C_SLAVE_ADDR10B_LSB_MASK));
        }
        else
        {
            // 7-bit slave address
            cmd[cmdCount++] =
                (uint16_t)kStartCmd | (uint16_t)((uint16_t)((uint16_t)xfer->slaveAddress << 1U) | (uint16_t)direction);
        }

        /* Subaddress, MSB first. */
        if (0U != xfer->subaddressSize)
        {
            uint32_t subaddressRemaining = xfer->subaddressSize;
            while (0U != (subaddressRemaining--))
            {
                uint8_t subaddressByte = (uint8_t)((xfer->subaddress >> (8U * subaddressRemaining)) & 0xffU);
                cmd[cmdCount++]        = subaddressByte;
            }
        }

        /* Reads need special handling. */
        if ((0U != xfer->dataSize) && (xfer->direction == I2C_MODE_READ))
        {
            /* Need to send repeated start if switching directions to read. */
            if (direction == I2C_MODE_WRITE)
            {
                if(xfer->slaveAddress & I2C_SLAVE_ADDR10B_MSB_MASK)
                {
                    // 10-bit slave address
                    cmd[cmdCount++] =
                        (uint16_t)kStartCmd | (uint16_t)((uint16_t)((((uint16_t)xfer->slaveAddress & I2C_SLAVE_ADDR10B_MSB_MASK) >> (8 - 1)) | I2C_SLAVE_ADDR10B_MSB) | (uint16_t)I2C_MODE_READ);
                }
                else
                {
                    // 7-bit slave address
                    cmd[cmdCount++] = (uint16_t)kStartCmd |
                        (uint16_t)((uint16_t)((uint16_t)xfer->slaveAddress << 1U) | (uint16_t)I2C_MODE_READ);
                }
            }
        }

        /* Set up state machine for transferring the commands. */
        handle->state          = (uint8_t)kSendCommandState;
        handle->remainingBytes = (uint16_t)cmdCount;
        handle->buf            = (uint8_t *)&handle->commandBuffer;
    }
}

/**
 * @brief 以非阻塞方式执行I2C主机传输
 *
 * 此函数用于以非阻塞方式通过I2C主机接口执行数据传输。它可以处理读操作和写操作。
 *
 * @param I2Cx I2C模块的外设基地址
 * @param handle 指向I2C主机句柄的指针，该句柄包含传输的状态信息
 * @param transfer 指向包含传输配置的指针
 *
 * @return 返回操作的状态码。可能的状态码包括：
 *          - kStatus_Success: 操作成功
 *          - kStatus_InvalidArgument: 参数无效，例如传输的数据大小超出限制
 *          - kStatus_LPI2C_Busy: I2C总线正在忙，无法启动新的传输
 */
status_t LPI2C_MasterTransferNonBlocking(I2C_TypeDef *I2Cx,
                                         lpi2c_master_handle_t *handle,
                                         lpi2c_master_transfer_t *transfer)
{
    assert_param(NULL != handle);
    assert_param(NULL != transfer);
    assert_param(transfer->subaddressSize <= sizeof(transfer->subaddress));

    status_t result;

    /* Check transfer data size in read operation. */
    if ((transfer->direction == I2C_MODE_READ) &&
        (transfer->dataSize > (256U * (uint32_t)FSL_FEATURE_I2C_FIFO_SIZEn(I2Cx))))
    {
        return kStatus_InvalidArgument;
    }

    /* Return busy if another transaction is in progress. */
    if (handle->state != (uint8_t)kIdleState)
    {
        result = kStatus_LPI2C_Busy;
    }
    else
    {
        result = LPI2C_CheckForBusyBus(I2Cx);
    }

    if ((status_t)kStatus_Success == result)
    {
        /* Disable LPI2C IRQ sources while we configure stuff. */
        LPI2C_MasterDisableInterrupts(I2Cx, (uint32_t)kLPI2C_MasterIrqFlags);

        /* Reset FIFO in case there are data. */
        I2Cx->MCR0 |= I2Cx_MCR0_RXFIFORST_Msk | I2Cx_MCR0_TXFIFORST_Msk;

        /* Save transfer into handle. */
        handle->transfer = *transfer;

        /* Generate commands to send. */
        LPI2C_InitTransferStateMachine(handle);

        /* Clear all flags. */
        I2C_MasterClearFlag(I2Cx, (uint32_t)kLPI2C_MasterClearFlags);

        /* Turn off auto-stop option. */
        I2Cx->MCR2 &= ~I2Cx_MCR2_AUTOSTOP_Msk;

        /* Enable LPI2C internal IRQ sources. NVIC IRQ was enabled in CreateHandle() */
        LPI2C_MasterEnableInterrupts(I2Cx, (uint32_t)kLPI2C_MasterIrqFlags);
    }

    return result;
}

/**
 * @brief 获取I2C主机在传输过程中已传输的字节数
 *
 * 该函数用于获取I2C主机在传输过程中已经传输的字节数。
 *
 * @param I2Cx I2C模块的指针
 * @param handle I2C主机传输句柄
 * @param count 指向一个size_t类型的指针，用于存储已传输的字节数
 *
 * @return 返回操作状态，成功时返回kStatus_Success，失败时返回相应的错误码
 *         - kStatus_Success: 操作成功
 *         - kStatus_InvalidArgument: 输入参数无效
 *         - kStatus_NoTransferInProgress: 当前没有正在进行的传输
 */
status_t LPI2C_MasterTransferGetCount(I2C_TypeDef *I2Cx, lpi2c_master_handle_t *handle, size_t *count)
{
    status_t result = kStatus_Success;

    assert_param(NULL != handle);

    if (NULL == count)
    {
        result = kStatus_InvalidArgument;
    }

    /* Catch when there is not an active transfer. */
    else if (handle->state == (uint8_t)kIdleState)
    {
        *count = 0;
        result = kStatus_NoTransferInProgress;
    }
    else
    {
        uint8_t state;
        uint16_t remainingBytes;
        uint32_t dataSize;

        /* Cache some fields with IRQs disabled. This ensures all field values */
        /* are synchronized with each other during an ongoing transfer. */
        uint32_t irqs = LPI2C_MasterGetEnabledInterrupts(I2Cx);
        LPI2C_MasterDisableInterrupts(I2Cx, irqs);
        state          = handle->state;
        remainingBytes = handle->remainingBytes;
        dataSize       = handle->transfer.dataSize;
        LPI2C_MasterEnableInterrupts(I2Cx, irqs);

        /* Get transfer count based on current transfer state. */
        switch (state)
        {
            case (uint8_t)kIdleState:
            case (uint8_t)kSendCommandState:
            case (uint8_t)
                kIssueReadCommandState: /* XXX return correct value for this state when >256 reads are supported */
                *count = 0;
                break;

            case (uint8_t)kTransferDataState:
                *count = dataSize - remainingBytes;
                break;

            case (uint8_t)kStopState:
            case (uint8_t)kWaitForCompletionState:
            default:
                *count = dataSize;
                break;
        }
    }

    return result;
}

/**
 * @brief 中止I2C主机传输
 *
 * 中止I2C主机传输，并重置相关状态。
 *
 * @param I2Cx I2C模块指针
 * @param handle I2C主机句柄指针
 */
void LPI2C_MasterTransferAbort(I2C_TypeDef *I2Cx, lpi2c_master_handle_t *handle)
{
    if (handle->state != (uint8_t)kIdleState)
    {
        /* Disable internal IRQ enables. */
        LPI2C_MasterDisableInterrupts(I2Cx, (uint32_t)kLPI2C_MasterIrqFlags);

        /* Reset fifos. */
        I2Cx->MCR0 |= I2Cx_MCR0_RXFIFORST_Msk | I2Cx_MCR0_TXFIFORST_Msk;

        /* If master is still busy and has not send out stop signal yet. */
        if ((LPI2C_MasterGetStatusFlags(I2Cx) & ((uint32_t)kLPI2C_MasterStopDetectFlag |
                                                 (uint32_t)kLPI2C_MasterBusyFlag)) == (uint32_t)kLPI2C_MasterBusyFlag)
        {
            /* Send a stop command to finalize the transfer. */
            I2Cx->MTDR = (uint32_t)kStopCmd;
        }

        /* Reset handle. */
        handle->state = (uint8_t)kIdleState;
    }
}

/**
 * @brief 处理LPI2C主模式传输的中断请求
 *
 * 该函数处理LPI2C主模式传输的中断请求，调用状态机处理传输，并根据处理结果进行相应的操作。
 *
 * @param I2Cx LPI2C模块指针
 * @param lpi2cMasterHandle LPI2C主模式句柄指针
 */
void LPI2C_MasterTransferHandleIRQ(I2C_TypeDef *I2Cx, void *lpi2cMasterHandle)
{
    assert_param(lpi2cMasterHandle != NULL);

	lpi2c_master_handle_t *handle = (lpi2c_master_handle_t*) lpi2cMasterHandle;
	boolean_t isDone = FALSE;
	status_t result;

    /* Don't do anything if we don't have a valid handle. */
    if (NULL != handle)
    {
        if (handle->state != (uint8_t)kIdleState)
        {
            result = LPI2C_RunTransferStateMachine(I2Cx, handle, &isDone);

            if ((result != kStatus_Success) || isDone)
            {
                /* Handle error, terminate xfer */
                if (result != kStatus_Success)
                {
                    LPI2C_MasterTransferAbort(I2Cx, handle);
                }

                /* Disable internal IRQ enables. */
                LPI2C_MasterDisableInterrupts(I2Cx, (uint32_t)kLPI2C_MasterIrqFlags);

                /* Set handle to idle state. */
                handle->state = (uint8_t)kIdleState;

                /* Invoke callback. */
                if (NULL != handle->completionCallback)
                {
                    handle->completionCallback(I2Cx, handle, result, handle->userData);
                }
            }
        }
    }
}

/**
 * @brief I2C从设备去初始化函数
 *
 * 该函数用于停止I2C从设备并将其关闭。
 *
 * @param I2Cx I2C外设指针，指向需要停止并关闭的I2C外设。
 */
void I2C_SlaveDeinit(I2C_TypeDef *I2Cx)
{
    I2C_SlaveSoftReset(I2Cx);

    I2C_DeInit(I2Cx);
}

/**
 * @brief 检查并清除LPI2C从机错误
 *
 * 检查指定I2C设备上的从机错误标志，并清除这些错误。
 *
 * @param I2Cx 指向I2C设备的指针
 * @param flags 需要检查的错误标志位掩码
 *
 * @return 如果检测到错误，则返回相应的错误状态码；否则返回kStatus_Success
 */
static status_t LPI2C_SlaveCheckAndClearError(I2C_TypeDef *I2Cx, uint32_t flags)
{
    status_t result = kStatus_Success;

    flags &= (uint32_t)kLPI2C_SlaveErrorFlags;
    if (0U != flags)
    {
        if (0U != (flags & (uint32_t)kLPI2C_SlaveBitErrFlag))
        {
            result = kStatus_LPI2C_BitError;
        }
        else if (0U != (flags & (uint32_t)kLPI2C_SlaveFifoErrFlag))
        {
            result = kStatus_LPI2C_FifoError;
        }
        else
        {
            ; /* Intentional empty */
        }

        /* Clear the errors. */
        I2C_SlaveClearFlag(I2Cx, flags);
    }
    else
    {
        ; /* Intentional empty */
    }

    return result;
}

/**
 * @brief I2C从设备以查询方式发送数据
 *
 * 使用I2C从设备发送数据。此函数会尝试将数据从缓冲区txBuff发送出去，直到所有数据被发送完毕或发生错误。
 *
 * @param I2Cx I2C模块指针
 * @param txBuff 待发送数据的缓冲区指针
 * @param txSize 待发送数据的字节数
 * @param actualTxSize 实际发送的字节数指针（可以为NULL）
 *
 * @return 发送状态，成功返回kStatus_Success，否则返回相应的错误码
 */
status_t LPI2C_SlaveSend(I2C_TypeDef *I2Cx, void *txBuff, size_t txSize, size_t *actualTxSize)
{
    status_t result  = kStatus_Success;
    uint8_t *buf     = (uint8_t *)txBuff;
    size_t remaining = txSize;

    assert_param(NULL != txBuff);

#if I2C_RETRY_TIMES != 0U
    uint32_t waitTimes = I2C_RETRY_TIMES;
#endif

    /* Clear stop flag. */
    I2C_SlaveClearFlag(I2Cx,
                       (uint32_t)kLPI2C_SlaveStopDetectFlag | (uint32_t)kLPI2C_SlaveRepeatedStartDetectFlag);

    while (0U != remaining)
    {
        uint32_t flags;

        /* Wait until we can transmit. */
        do
        {
            /* Check for errors */
            flags  = LPI2C_SlaveGetStatusFlags(I2Cx);
            result = LPI2C_SlaveCheckAndClearError(I2Cx, flags);
            if (kStatus_Success != result)
            {
                if (NULL != actualTxSize)
                {
                    *actualTxSize = txSize - remaining;
                }
                break;
            }
#if I2C_RETRY_TIMES != 0U
            waitTimes--;
        } while ((0U == (flags & ((uint32_t)kLPI2C_SlaveTxReadyFlag | (uint32_t)kLPI2C_SlaveStopDetectFlag |
                                  (uint32_t)kLPI2C_SlaveRepeatedStartDetectFlag))) &&
                 (0U != waitTimes));
        if (0U == waitTimes)
        {
            result = kStatus_LPI2C_Timeout;
        }
#else
        } while (0U == (flags & ((uint32_t)kLPI2C_SlaveTxReadyFlag | (uint32_t)kLPI2C_SlaveStopDetectFlag |
                                 (uint32_t)kLPI2C_SlaveRepeatedStartDetectFlag)));
#endif

        if (kStatus_Success != result)
        {
            break;
        }

        /* Send a byte. */
        if (0U != (flags & (uint32_t)kLPI2C_SlaveTxReadyFlag))
        {
            I2Cx->STDR = *buf++;
            --remaining;
        }

        /* Exit loop if we see a stop or restart in transfer*/
        if ((0U != (flags & ((uint32_t)kLPI2C_SlaveStopDetectFlag | (uint32_t)kLPI2C_SlaveRepeatedStartDetectFlag))) &&
            (remaining != 0U))
        {
            I2C_SlaveClearFlag(
                I2Cx, (uint32_t)kLPI2C_SlaveStopDetectFlag | (uint32_t)kLPI2C_SlaveRepeatedStartDetectFlag);
            break;
        }
    }

    if (NULL != actualTxSize)
    {
        *actualTxSize = txSize - remaining;
    }

    return result;
}

/**
 * @brief I2C从设备以查询方式接收数据
 *
 * 该函数指定的I2C从设备接收指定数量的字节数据。
 *
 * @param I2Cx I2C设备指针，指向要操作的I2C设备。
 * @param rxBuff 接收数据的缓冲区指针。
 * @param rxSize 要接收的数据字节数。
 * @param actualRxSize 实际接收到的数据字节数指针（可选）。
 *
 * @return 接收操作的状态。成功返回kStatus_Success，失败返回相应的错误代码。
 */
status_t LPI2C_SlaveReceive(I2C_TypeDef *I2Cx, void *rxBuff, size_t rxSize, size_t *actualRxSize)
{
    status_t result  = kStatus_Success;
    uint8_t *buf     = (uint8_t *)rxBuff;
    size_t remaining = rxSize;

    assert_param(NULL != rxBuff);

#if I2C_RETRY_TIMES != 0U
    uint32_t waitTimes = I2C_RETRY_TIMES;
#endif

    /* Clear stop flag. */
    I2C_SlaveClearFlag(I2Cx,
                       (uint32_t)kLPI2C_SlaveStopDetectFlag | (uint32_t)kLPI2C_SlaveRepeatedStartDetectFlag);

    while (0U != remaining)
    {
        uint32_t flags;

        /* Wait until we can receive. */
        do
        {
            /* Check for errors */
            flags  = LPI2C_SlaveGetStatusFlags(I2Cx);
            result = LPI2C_SlaveCheckAndClearError(I2Cx, flags);
            if (kStatus_Success != result)
            {
                if (NULL != actualRxSize)
                {
                    *actualRxSize = rxSize - remaining;
                }
                break;
            }
#if I2C_RETRY_TIMES != 0U
            waitTimes--;
        } while ((0U == (flags & ((uint32_t)kLPI2C_SlaveRxReadyFlag | (uint32_t)kLPI2C_SlaveStopDetectFlag |
                                  (uint32_t)kLPI2C_SlaveRepeatedStartDetectFlag))) &&
                 (0U != waitTimes));
        if (0U == waitTimes)
        {
            result = kStatus_LPI2C_Timeout;
        }
#else
        } while (0U == (flags & ((uint32_t)kLPI2C_SlaveRxReadyFlag | (uint32_t)kLPI2C_SlaveStopDetectFlag |
                                 (uint32_t)kLPI2C_SlaveRepeatedStartDetectFlag)));
#endif

        if ((status_t)kStatus_Success != result)
        {
            break;
        }

        /* Receive a byte. */
        if (0U != (flags & (uint32_t)kLPI2C_SlaveRxReadyFlag))
        {
            *buf++ = (uint8_t)(I2Cx->SRDR & I2Cx_SRDR_DATA_Msk);
            --remaining;
        }

        /* Exit loop if we see a stop or restart */
        if ((0U != (flags & ((uint32_t)kLPI2C_SlaveStopDetectFlag | (uint32_t)kLPI2C_SlaveRepeatedStartDetectFlag))) &&
            (remaining != 0U))
        {
            I2C_SlaveClearFlag(
                I2Cx, (uint32_t)kLPI2C_SlaveStopDetectFlag | (uint32_t)kLPI2C_SlaveRepeatedStartDetectFlag);
            break;
        }
    }

    if (NULL != actualRxSize)
    {
        *actualRxSize = rxSize - remaining;
    }

    return result;
}

/**
 * @brief 创建I2C从设备传输句柄
 *
 * 创建一个I2C从设备传输句柄，并初始化相关参数。
 *
 * @param I2Cx I2C设备指针，指向一个I2C_TypeDef类型的结构体，表示要使用的I2C设备。
 * @param handle 指向一个lpi2c_slave_handle_t类型的指针，用于存储句柄信息。
 * @param callback 指向一个lpi2c_slave_transfer_callback_t类型的回调函数，用于处理传输完成后的回调。
 * @param userData 用户自定义数据指针，可以在回调函数中使用。
 */
void LPI2C_SlaveTransferCreateHandle(I2C_TypeDef *I2Cx,
                                     lpi2c_slave_handle_t *handle,
                                     lpi2c_slave_transfer_callback_t callback,
                                     void *userData)
{
    uint32_t instance;

    assert_param(NULL != handle);

    /* Clear out the handle. */
    (void)memset(handle, 0, sizeof(*handle));

    /* Look up instance number */
    instance = LPI2C_GetInstance(I2Cx);

    /* Save base and instance. */
    handle->callback = callback;
    handle->userData = userData;

    /* Save this handle for IRQ use. */
    s_lpi2cSlaveHandle[instance] = handle;

    /* Set irq handler. */
    s_lpi2cSlaveIsr = LPI2C_SlaveTransferHandleIRQ;

    /* Clear internal IRQ enables and enable NVIC IRQ. */
    LPI2C_SlaveDisableInterrupts(I2Cx, (uint32_t)kLPI2C_SlaveIrqFlags);
    //(void)EnableIRQ(kLpi2cIrqs[instance]);
    NVIC_ClearPendingIRQ(kLpi2cIrqs[instance]);
    NVIC_EnableIRQ(kLpi2cIrqs[instance]);

    /* Nack by default. */
    I2Cx->STAR = I2Cx_STAR_TXNACK_Msk;
}

/**
 * @brief 以非阻塞方式执行从设备I2C传输
 *
 * 该函数用于以非阻塞方式执行从设备的I2C传输操作。如果当前没有正在进行的传输，函数将配置传输，并启动传输过程。
 * 如果当前有传输正在进行或总线已被其他设备占用，则返回相应的错误状态。
 *
 * @param I2Cx I2C模块指针，指向要使用的I2C模块的寄存器地址
 * @param handle 从设备句柄，指向存储从设备配置和状态的结构体
 * @param eventMask 事件掩码，指定需要监听的事件类型
 *
 * @return 函数执行状态。如果成功，则返回kStatus_Success；如果失败，则返回相应的错误代码
 */
status_t LPI2C_SlaveTransferNonBlocking(I2C_TypeDef *I2Cx, lpi2c_slave_handle_t *handle, uint32_t eventMask)
{
    status_t result = kStatus_Success;

    assert_param(NULL != handle);

    /* Return busy if another transaction is in progress. */
    if (handle->isBusy)
    {
        result = kStatus_LPI2C_Busy;
    }
    else
    {
        /* Return an error if the bus is already in use not by us. */
        uint32_t status = LPI2C_SlaveGetStatusFlags(I2Cx);
        if ((0U != (status & (uint32_t)kLPI2C_SlaveBusBusyFlag)) && (0U == (status & (uint32_t)kLPI2C_SlaveBusyFlag)))
        {
            result = kStatus_LPI2C_Busy;
        }
    }

    if ((status_t)kStatus_Success == result)
    {
        /* Disable LPI2C IRQ sources while we configure stuff. */
        LPI2C_SlaveDisableInterrupts(I2Cx, (uint32_t)kLPI2C_SlaveIrqFlags);

        /* Clear transfer in handle. */
        (void)memset(&handle->transfer, 0, sizeof(handle->transfer));

        /* Record that we're busy. */
        handle->isBusy = TRUE;

        /* Set up event mask. tx and rx are always enabled. */
        handle->eventMask = eventMask | (uint32_t)kLPI2C_SlaveTransmitEvent | (uint32_t)kLPI2C_SlaveReceiveEvent;

        /* Ack by default. */
        I2Cx->STAR = 0U;

        /* Clear all flags. */
        I2C_SlaveClearFlag(I2Cx, (uint32_t)kLPI2C_SlaveClearFlags);

        /* Enable LPI2C internal IRQ sources. NVIC IRQ was enabled in CreateHandle() */
        LPI2C_SlaveEnableInterrupts(I2Cx, (uint32_t)kLPI2C_SlaveIrqFlags);
    }

    return result;
}

/**
 * @brief 获取 I2C 从设备传输的数据量
 *
 * 此函数用于获取 I2C 从设备当前传输的数据量。
 *
 * @param I2Cx I2C 外设的基地址指针
 * @param handle I2C 从设备的句柄指针
 * @param count 用于存储获取到的传输数据量的指针
 *
 * @return 函数执行状态。
 *         - kStatus_Success: 成功获取传输数据量。
 *         - kStatus_InvalidArgument: 传入参数无效。
 *         - kStatus_NoTransferInProgress: 当前没有传输正在进行。
 */
status_t LPI2C_SlaveTransferGetCount(I2C_TypeDef *I2Cx, lpi2c_slave_handle_t *handle, size_t *count)
{
    status_t status = kStatus_Success;

    assert_param(NULL != handle);

    if (count == NULL)
    {
        status = kStatus_InvalidArgument;
    }

    /* Catch when there is not an active transfer. */
    else if (!handle->isBusy)
    {
        *count = 0;
        status = kStatus_NoTransferInProgress;
    }

    /* For an active transfer, just return the count from the handle. */
    else
    {
        *count = handle->transferredCount;
    }

    return status;
}

/**
 * @brief 终止LPI2C从机传输
 *
 * 终止指定的LPI2C从机传输。
 *
 * @param I2Cx LPI2C模块指针
 * @param handle 从机传输句柄指针
 */
void LPI2C_SlaveTransferAbort(I2C_TypeDef *I2Cx, lpi2c_slave_handle_t *handle)
{
    assert_param(NULL != handle);

    /* Return idle if no transaction is in progress. */
    if (handle->isBusy)
    {
        /* Disable LPI2C IRQ sources. */
        LPI2C_SlaveDisableInterrupts(I2Cx, (uint32_t)kLPI2C_SlaveIrqFlags);

        /* Nack by default. */
        I2Cx->STAR = I2Cx_STAR_TXNACK_Msk;

        /* Reset transfer info. */
        (void)memset(&handle->transfer, 0, sizeof(handle->transfer));

        /* We're no longer busy. */
        handle->isBusy = FALSE;
    }
}

/**
 * @brief 处理I2C从机传输中断
 *
 * 处理I2C从机传输中断，根据中断状态进行相应的处理。
 *
 * @param I2Cx I2C模块指针
 * @param handle I2C从机传输句柄指针
 */
void LPI2C_SlaveTransferHandleIRQ(I2C_TypeDef *I2Cx, lpi2c_slave_handle_t *handle)
{
    uint32_t flags;
    lpi2c_slave_transfer_t *xfer;

    /* Check for a valid handle in case of a spurious interrupt. */
    if (NULL != handle)
    {
        xfer = &handle->transfer;

        /* Get status flags. */
        flags = LPI2C_SlaveGetStatusFlags(I2Cx);

        if (0U != (flags & ((uint32_t)kLPI2C_SlaveBitErrFlag | (uint32_t)kLPI2C_SlaveFifoErrFlag)))
        {
            xfer->event            = kLPI2C_SlaveCompletionEvent;
            xfer->completionStatus = LPI2C_SlaveCheckAndClearError(I2Cx, flags);

            if ((0U != (handle->eventMask & (uint32_t)kLPI2C_SlaveCompletionEvent)) && (NULL != handle->callback))
            {
                handle->callback(I2Cx, xfer, handle->userData);
            }
        }
        else
        {
            if (0U !=
                (flags & (((uint32_t)kLPI2C_SlaveRepeatedStartDetectFlag) | ((uint32_t)kLPI2C_SlaveStopDetectFlag))))
            {
                xfer->event = (0U != (flags & (uint32_t)kLPI2C_SlaveRepeatedStartDetectFlag)) ?
                                  kLPI2C_SlaveRepeatedStartEvent :
                                  kLPI2C_SlaveCompletionEvent;
                xfer->receivedAddress  = 0U;
                xfer->completionStatus = kStatus_Success;
                xfer->transferredCount = handle->transferredCount;

                if (xfer->event == kLPI2C_SlaveCompletionEvent)
                {
                    handle->isBusy = FALSE;
                }

                if (handle->wasTransmit)
                {
                    /* Subtract one from the transmit count to offset the fact that LPI2C asserts the */
                    /* tx flag before it sees the nack from the master-receiver, thus causing one more */
                    /* count that the master actually receives. */
                    --xfer->transferredCount;
                    handle->wasTransmit = FALSE;
                }

                /* Clear the flag. */
                I2C_SlaveClearFlag(I2Cx, flags & ((uint32_t)kLPI2C_SlaveRepeatedStartDetectFlag |
                                                           (uint32_t)kLPI2C_SlaveStopDetectFlag));

                /* Revert to sending an Ack by default, in case we sent a Nack for receive. */
                I2Cx->STAR = 0U;

                if ((0U != (handle->eventMask & (uint32_t)xfer->event)) && (NULL != handle->callback))
                {
                    handle->callback(I2Cx, xfer, handle->userData);
                }

                if (0U != (flags & (uint32_t)kLPI2C_SlaveStopDetectFlag))
                {
                    /* Clean up transfer info on completion, after the callback has been invoked. */
                    (void)memset(&handle->transfer, 0, sizeof(handle->transfer));
                }
            }
            if (0U != (flags & (uint32_t)kLPI2C_SlaveAddressValidFlag))
            {
                xfer->event           = kLPI2C_SlaveAddressMatchEvent;
                xfer->receivedAddress = (uint8_t)(I2Cx->SASR & I2Cx_SASR_RADDR_Msk);

                /* Update handle status to busy because slave is addressed. */
                handle->isBusy = TRUE;
                if ((0U != (handle->eventMask & (uint32_t)kLPI2C_SlaveAddressMatchEvent)) && (NULL != handle->callback))
                {
                    handle->callback(I2Cx, xfer, handle->userData);
                }
            }
            if (0U != (flags & (uint32_t)kLPI2C_SlaveTransmitAckFlag))
            {
                xfer->event = kLPI2C_SlaveTransmitAckEvent;

                if ((0U != (handle->eventMask & (uint32_t)kLPI2C_SlaveTransmitAckEvent)) && (NULL != handle->callback))
                {
                    handle->callback(I2Cx, xfer, handle->userData);
                }
            }

            /* Handle transmit and receive. */
            if (0U != (flags & (uint32_t)kLPI2C_SlaveTxReadyFlag))
            {
                handle->wasTransmit = TRUE;

                /* If we're out of data, invoke callback to get more. */
                if ((NULL == xfer->data) || (0U == xfer->dataSize))
                {
                    xfer->event = kLPI2C_SlaveTransmitEvent;
                    if (NULL != handle->callback)
                    {
                        handle->callback(I2Cx, xfer, handle->userData);
                    }

                    /* Clear the transferred count now that we have a new buffer. */
                    handle->transferredCount = 0U;
                }

                /* Transmit a byte. */
                if ((NULL != xfer->data) && (0U != xfer->dataSize))
                {
                    I2Cx->STDR = *xfer->data++;
                    --xfer->dataSize;
                    ++handle->transferredCount;
                }
            }
            if (0U != (flags & (uint32_t)kLPI2C_SlaveRxReadyFlag))
            {
                /* If we're out of room in the buffer, invoke callback to get another. */
                if ((NULL == xfer->data) || (0U == xfer->dataSize))
                {
                    xfer->event = kLPI2C_SlaveReceiveEvent;
                    if (NULL != handle->callback)
                    {
                        handle->callback(I2Cx, xfer, handle->userData);
                    }

                    /* Clear the transferred count now that we have a new buffer. */
                    handle->transferredCount = 0U;
                }

                /* Receive a byte. */
                if ((NULL != xfer->data) && (0U != xfer->dataSize))
                {
                    *xfer->data++ = (uint8_t)I2Cx->SRDR;
                    --xfer->dataSize;
                    ++handle->transferredCount;
                }
                else
                {
                    /* We don't have any room to receive more data, so send a nack. */
                    I2Cx->STAR = I2Cx_STAR_TXNACK_Msk;
                }
            }
        }
    }
}

/**
 * @brief 通用I2C中断处理函数
 *
 * 此函数用于处理I2C总线的中断请求。根据I2C的工作模式（主模式或从模式），调用相应的中断服务程序。
 *
 * @param I2Cx 指向I2C模块的指针
 * @param instance I2C实例编号
 */
void LPI2C_CommonIRQHandler(I2C_TypeDef *I2Cx, uint32_t instance)
{
    /* Check for master IRQ. */
    if ((0U != (I2Cx->MCR0 & I2Cx_MCR0_MEN_Msk)) && (NULL != s_lpi2cMasterIsr))
    {
        /* Master mode. */
        s_lpi2cMasterIsr(I2Cx, s_lpi2cMasterHandle[instance]);
    }

    /* Check for slave IRQ. */
    if ((0U != (I2Cx->SCR0 & I2Cx_SCR0_SEN_Msk)) && (NULL != s_lpi2cSlaveIsr))
    {
        /* Slave mode. */
			  s_lpi2cSlaveIsr(I2Cx, s_lpi2cSlaveHandle[instance]);
    }

}

