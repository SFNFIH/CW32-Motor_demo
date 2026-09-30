/**
 * @file    BSP_HFI.h
 * @brief   VESC-style six-vector HFI inductance measurement
 *
 * 对照 vedderb/bldc:
 *   FOC_AMB_MODE_SIX_VECTOR + HFI_SAMPLES_32
 *   buffer[i] = f_zv * di / Vhfi   (inverse inductance)
 *   Ld = 1/(offset+amp), Lq = 1/(offset-amp)  (FFT bin0 / bin2)
 */
#ifndef BSP_HFI_H
#define BSP_HFI_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    float l_avg_h;       /* (Ld+Lq)/2, already *0.9 */
    float ld_h;
    float lq_h;
    float ld_lq_diff_h;  /* Lq - Ld, *0.9 */
    float i_avg_a;       /* mean |di| */
    float v_hfi_v;       /* injection voltage used */
    uint8_t ok;
} BSP_HFI_LResult_t;

/**
 * Six-vector HFI measure (rotor locked / standstill).
 * @param duty_frac  0.02..0.5 like VESC measure_inductance duty search
 * @param sweeps     number of full 32-point revolutions to average (>=1)
 */
int BSP_HFI_MeasureInductance(float duty_frac, int sweeps, BSP_HFI_LResult_t *out);

/**
 * Duty search toward current_goal then measure (mcpwm_foc_measure_inductance_current).
 */
int BSP_HFI_MeasureInductanceCurrent(float current_goal_a, int sweeps,
                                     BSP_HFI_LResult_t *out);

#ifdef __cplusplus
}
#endif

#endif /* BSP_HFI_H */
