/**
 * @file    BSP_DebugSnap.h
 * @brief   PyOCD 可读调试快照 (固定布局, 主循环刷新)
 */
#ifndef BSP_DEBUGSNAP_H
#define BSP_DEBUGSNAP_H

#include <stdint.h>
#include "BSP_FOC.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DBG_SNAP_MAGIC  0x532FC60DU

/* host → target: 写 g_dbg.cmd */
#define DBG_CMD_NONE        0U
#define DBG_CMD_START       1U
#define DBG_CMD_STOP        2U
#define DBG_CMD_TOGGLE_DIR  3U
#define DBG_CMD_DETECT      4U  /* 电机自整定 R/L/Flux (需停机) */

typedef struct __attribute__((packed))
{
    uint32_t magic;
    uint32_t seq;
    uint16_t state;
    uint16_t step;
    uint16_t bemf;
    uint16_t mid;
    uint16_t duty;
    uint16_t pot;
    uint16_t zc;
    uint16_t miss;
    uint16_t period;
    int16_t  dir;
    uint16_t as_raw;
    uint16_t as_status;
    int32_t  as_cum;
    int32_t  as_rpm_x10;
    uint8_t  as_ok;
    uint8_t  motor_on;
    uint16_t reserved;
    volatile uint32_t cmd;
} DbgSnap_t;

extern volatile DbgSnap_t g_dbg;
/** 0=跟电位器; 非0=PyOCD 强制占空比 */
extern volatile uint16_t g_force_duty;

void BSP_DebugSnap_Init(void);
void BSP_DebugSnap_Publish(const BSP_FOC_State_t *foc, uint16_t pot, uint8_t motor_on);
uint32_t BSP_DebugSnap_TakeCmd(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_DEBUGSNAP_H */
