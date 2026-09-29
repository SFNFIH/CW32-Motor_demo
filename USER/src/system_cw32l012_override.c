/**
 * @brief  覆盖库中的 SystemInit() — 跳过 TRIM 校准
 * @note   原 SystemInit 读取 FLASH 信息块 (0x001007C0) 可能导致 HardFault
 *         CW32L012 HSI 使用默认 TRIM 值即可正常工作
 */
#include "system_cw32l012.h"

/* 提供 SystemCoreClock 和 SystemCoreClockUpdate 的实现 */
uint32_t SystemCoreClock = 96000000;  /* HSI DIV1, 与参考无感工程一致 */

void SystemCoreClockUpdate(void)
{
    SystemCoreClock = 96000000;
}

void SystemInit(void)
{
    /* 不做 TRIM 校准, 避免访问 FLASH 信息块 */
}

void FirmwareDelay(uint32_t DlyCnt)
{
    volatile uint32_t thisCnt = DlyCnt;
    while (thisCnt--) { ; }
}
