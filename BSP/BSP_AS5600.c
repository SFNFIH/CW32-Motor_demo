/**
 * @file    BSP_AS5600.c
 * @brief   AS5600 via HW I2C1 (PC14=SDA, PC15=SCL)
 *          对齐 cw32l012_i2c_master_int 已验证写法
 */
#include "BSP_AS5600.h"
#include "cw32l012_gpio.h"
#include "cw32l012_sysctrl.h"
#include "cw32l012_lpi2c.h"
#include "system_cw32l012.h"

#define AS5600_REG_STATUS   0x0BU
#define AS5600_REG_ANGLE_H  0x0EU
#define AS5600_STATUS_MD    0x20U

#define I2C1_SCL_GPIO_PORT  CW_GPIOC
#define I2C1_SCL_GPIO_PIN   GPIO_PIN_15
#define I2C1_SDA_GPIO_PORT  CW_GPIOC
#define I2C1_SDA_GPIO_PIN   GPIO_PIN_14

#define I2C1_SCL_LOW()      PC15_SETLOW()
#define I2C1_SDA_LOW()      PC14_SETLOW()
#define I2C1_SCL_HIGH()     PC15_SETHIGH()
#define I2C1_SDA_HIGH()     PC14_SETHIGH()
#define I2C1_SCL2GPIO()     PC15_AFx_GPIO()
#define I2C1_SDA2GPIO()     PC14_AFx_GPIO()
#define GPIO2I2C1_SCL()     PC15_AFx_I2C1SCL()
#define GPIO2I2C1_SDA()     PC14_AFx_I2C1SDA()

static BSP_AS5600_State_t s_st;
static uint16_t s_prev_raw;
static uint8_t  s_have_prev;
static uint32_t s_last_ms;
static int32_t  s_cum_at_last;
static uint8_t  s_rpm_init;
static uint8_t  s_last_err;
static uint8_t  s_idle_pins;
static uint8_t  s_found_addr;
static uint32_t s_pclk_hz;

volatile uint32_t g_millis;

static void I2C_Stop_SW(void)
{
    I2C1_SCL_LOW();
    I2C1_SDA_LOW();
    FirmwareDelay(4);
    I2C1_SCL_HIGH();
    FirmwareDelay(4);
    I2C1_SDA_HIGH();
}

static void I2C_ExceptionHandling(void)
{
    I2C1_SCL2GPIO();
    I2C1_SDA2GPIO();
    I2C_Stop_SW();
    FirmwareDelay(1000);
    GPIO2I2C1_SCL();
    GPIO2I2C1_SDA();
}

static void LPI2C_MasterInitHw(uint32_t pclk_hz)
{
    I2C_MasterConfigTypeDef cfg;

    I2C_MasterDeinit(CW_I2C1);
    I2C_MasterInitDefault(&cfg);
    cfg.I2C_BaudRate_Hz = 100000UL;
    I2C_MasterInit(CW_I2C1, &cfg, pclk_hz);
}

static uint8_t ReadRegs(uint8_t reg, uint8_t *buf, uint16_t len)
{
    uint8_t regAddr = reg;
    uint8_t st;

    st = I2C_MasterReceiveDataFromSlave(CW_I2C1, BSP_AS5600_I2C_ADDR, &regAddr,
                                        I2C_REG_ADDR_8BIT, buf, len);
    s_last_err = st;
    if (st != I2C_Status_Success)
    {
        I2C_MasterSoftReset(CW_I2C1);
        LPI2C_MasterInitHw(s_pclk_hz);
        I2C_MasterCmd(CW_I2C1, ENABLE);
        return 0U;
    }
    return 1U;
}

void BSP_AS5600_Init(uint32_t pclk_hz)
{
    GPIO_InitTypeDef gpio = {0};

    s_pclk_hz = (pclk_hz == 0U) ? 96000000UL : pclk_hz;
    s_st.raw = 0U;
    s_st.status = 0U;
    s_st.cum_raw = 0;
    s_st.rpm_x10 = 0;
    s_st.ok = 0U;
    s_st.mag_ok = 0U;
    s_have_prev = 0U;
    s_last_ms = 0U;
    s_cum_at_last = 0;
    s_rpm_init = 0U;
    s_last_err = 0U;
    s_found_addr = 0U;
    s_idle_pins = 0U;

    __SYSCTRL_GPIOC_CLK_ENABLE();
    __SYSCTRL_I2C1_CLK_ENABLE();

    GPIO2I2C1_SCL();
    GPIO2I2C1_SDA();
    gpio.Pins = I2C1_SCL_GPIO_PIN | I2C1_SDA_GPIO_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_OD;
    gpio.IT = GPIO_IT_NONE;
    GPIO_Init(I2C1_SCL_GPIO_PORT, &gpio);
    PC14_PUR_ENABLE();
    PC15_PUR_ENABLE();

    s_idle_pins = (uint8_t)((PC14_GETVALUE() ? 1U : 0U) | (PC15_GETVALUE() ? 2U : 0U));

    LPI2C_MasterInitHw(s_pclk_hz);
    I2C_MasterCmd(CW_I2C1, ENABLE);

    if (I2C_MasterPollingFlag(CW_I2C1, I2C_IT_BUSBUSY) == I2C_Status_Success)
    {
        I2C_ExceptionHandling();
    }

    {
        uint16_t st = 0U;
        if (BSP_AS5600_ReadStatus(&st) != 0U)
        {
            s_found_addr = BSP_AS5600_I2C_ADDR;
            s_st.ok = 1U;
            s_st.status = st;
            s_st.mag_ok = ((st & AS5600_STATUS_MD) != 0U) ? 1U : 0U;
        }
    }
}

uint8_t BSP_AS5600_Probe(void)
{
    uint16_t st = 0U;
    return BSP_AS5600_ReadStatus(&st);
}

uint8_t BSP_AS5600_ReadStatus(uint16_t *status)
{
    uint8_t b = 0U;
    if (status == NULL) { return 0U; }
    if (ReadRegs(AS5600_REG_STATUS, &b, 1U) == 0U) { return 0U; }
    *status = b;
    return 1U;
}

uint8_t BSP_AS5600_ReadRaw(uint16_t *raw)
{
    uint8_t buf[2];
    if (raw == NULL) { return 0U; }
    if (ReadRegs(AS5600_REG_ANGLE_H, buf, 2U) == 0U) { return 0U; }
    *raw = (uint16_t)(((uint16_t)(buf[0] & 0x0FU) << 8) | buf[1]);
    return 1U;
}

void BSP_AS5600_Update(void)
{
    uint16_t raw = 0U;
    uint16_t status = 0U;
    int32_t delta;

    if (BSP_AS5600_ReadStatus(&status) == 0U)
    {
        s_st.ok = 0U;
        s_st.mag_ok = 0U;
        s_st.status = (uint16_t)(((uint16_t)s_idle_pins << 8) | s_last_err);
        s_st.rpm_x10 = (s_st.rpm_x10 * 3) / 4;
        return;
    }
    if (BSP_AS5600_ReadRaw(&raw) == 0U)
    {
        s_st.ok = 0U;
        s_st.mag_ok = 0U;
        s_st.status = (uint16_t)(((uint16_t)s_idle_pins << 8) | s_last_err);
        s_st.rpm_x10 = (s_st.rpm_x10 * 3) / 4;
        return;
    }

    s_found_addr = BSP_AS5600_I2C_ADDR;
    s_st.ok = 1U;
    s_st.status = status;
    s_st.mag_ok = ((status & AS5600_STATUS_MD) != 0U) ? 1U : 0U;
    s_st.raw = raw;

    if (s_have_prev == 0U)
    {
        s_prev_raw = raw;
        s_have_prev = 1U;
        s_last_ms = g_millis;
        s_st.cum_raw = (int32_t)raw;
        s_cum_at_last = s_st.cum_raw;
        s_rpm_init = 1U;
        return;
    }

    delta = (int32_t)raw - (int32_t)s_prev_raw;
    if (delta > 2048) { delta -= 4096; }
    else if (delta < -2048) { delta += 4096; }
    s_st.cum_raw += delta;
    s_prev_raw = raw;

    {
        uint32_t now = g_millis;
        uint32_t dt = now - s_last_ms;
        int32_t inst;

        if (s_rpm_init == 0U)
        {
            s_last_ms = now;
            s_cum_at_last = s_st.cum_raw;
            s_rpm_init = 1U;
            s_st.rpm_x10 = 0;
            return;
        }

        /* 低速拉长窗口减噪声; 高速 40ms 够用 */
        if (dt >= 40U)
        {
            int32_t dcum = s_st.cum_raw - s_cum_at_last;
            inst = (dcum * 600000L) / ((int32_t)dt * 4096L);
            if (inst > 25000L) { inst = 25000L; }
            if (inst < -25000L) { inst = -25000L; }
            s_st.rpm_x10 = (s_st.rpm_x10 * 3 + inst) / 4;
            s_cum_at_last = s_st.cum_raw;
            s_last_ms = now;
        }
    }
}

const BSP_AS5600_State_t *BSP_AS5600_GetState(void) { return &s_st; }
uint8_t BSP_AS5600_GetLastErr(void) { return s_last_err; }
uint8_t BSP_AS5600_GetIdlePins(void) { return s_idle_pins; }
uint8_t BSP_AS5600_GetFoundAddr(void) { return s_found_addr; }
