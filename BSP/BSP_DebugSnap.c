/**
 * @file    BSP_DebugSnap.c
 */
#include "BSP_DebugSnap.h"
#include "BSP_FOC.h"
#include "BSP_AS5600.h"

volatile DbgSnap_t g_dbg;
volatile uint16_t g_force_duty;

void BSP_DebugSnap_Init(void)
{
    g_dbg.magic = DBG_SNAP_MAGIC;
    g_dbg.seq = 0U;
    g_dbg.cmd = DBG_CMD_NONE;
}

void BSP_DebugSnap_Publish(const BSP_FOC_State_t *foc, uint16_t pot, uint8_t motor_on)
{
    const BSP_AS5600_State_t *as = BSP_AS5600_GetState();

    if (foc == 0)
    {
        return;
    }
    g_dbg.state = foc->mode;
    g_dbg.step = foc->running;
    g_dbg.bemf = (uint16_t)(int16_t)(foc->iq_a * 1000.0f);
    g_dbg.mid = (uint16_t)foc->id_a;
    g_dbg.duty = (uint16_t)(foc->rpm < 0.0f ? -foc->rpm : foc->rpm);
    g_dbg.pot = pot;
    g_dbg.zc = (uint16_t)(foc->theta * 57.3f);
    g_dbg.miss = (uint16_t)foc->isr_cnt;
    g_dbg.period = g_dbg.duty;
    g_dbg.dir = (foc->iq_a >= 0.0f) ? 1 : -1;
    g_dbg.as_raw = as->raw;
    g_dbg.as_status = as->status;
    g_dbg.as_cum = as->cum_raw;
    g_dbg.as_rpm_x10 = as->rpm_x10;
    g_dbg.as_ok = as->ok;
    g_dbg.motor_on = motor_on;
    /* reserved: found_addr[15:8] | idle_pins[7:4] | i2c_err[3:0] */
    g_dbg.reserved = (uint16_t)(
        ((uint16_t)BSP_AS5600_GetFoundAddr() << 8) |
        ((uint16_t)(BSP_AS5600_GetIdlePins() & 0x0FU) << 4) |
        (uint16_t)(BSP_AS5600_GetLastErr() & 0x0FU));
    g_dbg.seq++;
}

uint32_t BSP_DebugSnap_TakeCmd(void)
{
    uint32_t c = g_dbg.cmd;
    if (c != DBG_CMD_NONE)
    {
        g_dbg.cmd = DBG_CMD_NONE;
    }
    return c;
}
