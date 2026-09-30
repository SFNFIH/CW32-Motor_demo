/**
 * @file    BSP_MathHw.h
 * @brief   CW32L012 硬件加速数学: CORDIC + EAU
 *
 * 对照官方例程:
 *   CORDIC: cos / atan2 / sqrt / hypot (Doc 上传 CORDIC_*.zip)
 *   EAU:    signed/unsigned div / integer sqrt (EAU_*.zip)
 *
 * 自整定路径用 hypot/sqrt/div 替代 libm, 减轻 Cortex-M0+ 软浮点负担。
 */
#ifndef BSP_MATH_HW_H
#define BSP_MATH_HW_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void BSP_MathHw_Init(void);

/** sqrt(x), x>=0; 优先 EAU 定点开方 */
float BSP_MathHw_Sqrt(float x);

/** hypot(x,y)=sqrt(x²+y²); CORDIC HYPOT */
float BSP_MathHw_Hypot(float x, float y);

/** atan2(y,x) → 弧度; CORDIC (Z 以 π 为单位) */
float BSP_MathHw_Atan2(float y, float x);

/** cos/sin(angle_rad); CORDIC */
void BSP_MathHw_CosSin(float angle_rad, float *c, float *s);

/** num/den; EAU 有符号定点除法 */
float BSP_MathHw_Div(float num, float den);

/** 整数开方 (EAU); 返回 floor(sqrt(n)) */
uint32_t BSP_MathHw_ISqrt(uint32_t n);

#ifdef __cplusplus
}
#endif

#endif /* BSP_MATH_HW_H */
