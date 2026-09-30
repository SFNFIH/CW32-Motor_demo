/**
 * @file    BSP_MotorDetect.c
 * @brief   VESC-style motor parameter detection adapted to CW32 V/f + ABC inject
 *
 * Algorithm notes (vedderb/bldc):
 * - Resistance: lock electrical axis, inject current, R = |V|/|I|
 *   Here: DC inject on A/B (C at mid) → Van = Vbus*d/ARR, R = Van / Ia
 * - Inductance: VESC uses HFI+FFT; we use short voltage pulses + di/dt
 *   L ≈ V*dt/ΔI with R correction via RL equation; scale 0.9 like VESC
 * - Flux: open-loop spin, λ ≈ (V - I*R)/ωe - I*L (same formula as VESC)
 * - Current PI: kp = L*bw, ki = R*bw, bw = 1/(tc*1e-6), tc=1500 µs
 */
#include "BSP_MotorDetect.h"
#include "BSP_MOTOR.h"
#include "BSP_Current.h"
#include "BSP_Vbus.h"
#include "BSP_FOC.h"
#include "BSP_AS5600.h"
#include "BSP_motor_params.h"
#include "cw32l012.h"

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define DETECT_TC_US          1500.0f
#define DETECT_IND_SCALE      0.9f
#define DETECT_I_MIN_A        0.08f
#define DETECT_I_ABS_MAX_A    1.60f
#define DETECT_DUTY_MAX       0.35f
#define DETECT_PULSE_US       200U
#define DETECT_SETTLE_MS      50U
#define DETECT_RAMP_MS        1U

extern volatile uint32_t g_millis;

static BSP_MotorDetect_Result_t s_res;
static float s_rs_rt = MOTOR_RS_OHM;
static float s_ls_rt = MOTOR_LS_H;
static float s_flux_rt = 0.0f;
static uint8_t s_busy;

static void delay_ms(uint32_t ms)
{
    uint32_t t0 = g_millis;
    while ((g_millis - t0) < ms)
    {
    }
}

/* Busy-wait microseconds using SysTick VAL (HCLK ticks). */
static void delay_us(uint32_t us)
{
    uint32_t ticks = (SystemCoreClock / 1000000UL) * us;
    uint32_t load = SysTick->LOAD + 1U;
    uint32_t start = SysTick->VAL;
    uint32_t elapsed = 0U;

    if (ticks == 0U)
    {
        return;
    }
    while (elapsed < ticks)
    {
        uint32_t now = SysTick->VAL;
        uint32_t delta = (now <= start) ? (start - now) : (start + load - now);
        elapsed += delta;
        start = now;
    }
}

static float read_vbus(void)
{
    float v = BSP_Vbus_ReadVolt();
    if (v < 6.0f || v > 30.0f)
    {
        v = MOTOR_VBUS_V;
    }
    return v;
}

static uint8_t read_ia_ib(float *ia, float *ib)
{
    return BSP_Current_Read(ia, ib);
}

/**
 * DC inject along alpha (A vs B), C at mid — FOC phase_override≈0 equivalent.
 * d_counts: half of line-line duty counts; Van = Vbus * d / ARR
 */
static void set_dc_inject(int16_t d_counts)
{
    const uint16_t mid = (uint16_t)(BSP_MOTOR_PWM_ARR / 2U);
    int32_t da = (int32_t)mid + (int32_t)d_counts;
    int32_t db = (int32_t)mid - (int32_t)d_counts;
    int32_t dc = (int32_t)mid;

    if (da < 0) { da = 0; }
    if (db < 0) { db = 0; }
    if (da > (int32_t)BSP_MOTOR_PWM_ARR) { da = (int32_t)BSP_MOTOR_PWM_ARR; }
    if (db > (int32_t)BSP_MOTOR_PWM_ARR) { db = (int32_t)BSP_MOTOR_PWM_ARR; }
    BSP_MOTOR_SetPhaseDuty((uint16_t)da, (uint16_t)db, (uint16_t)dc);
}

static void pwm_idle(void)
{
    uint16_t mid = (uint16_t)(BSP_MOTOR_PWM_ARR / 2U);
    BSP_MOTOR_SetPhaseDuty(mid, mid, mid);
}

static float duty_to_vphase(float vbus, uint16_t d_counts)
{
    return vbus * ((float)d_counts / (float)BSP_MOTOR_PWM_ARR);
}

static int begin_inject(void)
{
    const BSP_FOC_State_t *st = BSP_FOC_GetState();
    if (st->running != 0U)
    {
        return BSP_DETECT_ERR_RUNNING;
    }
    BSP_MOTOR_DisablePwmIrq();
    BSP_MOTOR_Stop();
    delay_ms(5U);
    BSP_Current_Calibrate();
    BSP_MOTOR_Start();
    pwm_idle();
    delay_ms(2U);
    return BSP_DETECT_OK;
}

static void end_inject(void)
{
    pwm_idle();
    delay_ms(2U);
    BSP_MOTOR_Stop();
}

/**
 * Measure phase resistance (VESC mcpwm_foc_measure_resistance physics).
 * Ramp voltage until |Ia|≈current_a, then average V/I.
 */
int BSP_MotorDetect_MeasureR(float current_a, int samples, float *r_ohm)
{
    float vbus;
    float ia;
    float ib;
    float i_abs;
    float v_sum = 0.0f;
    float i_sum = 0.0f;
    int n = 0;
    int d = 0;
    int d_max;
    int i;
    int rc;

    if ((r_ohm == 0) || (current_a < DETECT_I_MIN_A) || (samples < 1))
    {
        return BSP_DETECT_ERR_PARAM;
    }
    if (current_a > DETECT_I_ABS_MAX_A)
    {
        current_a = DETECT_I_ABS_MAX_A;
    }

    rc = begin_inject();
    if (rc != BSP_DETECT_OK)
    {
        return rc;
    }

    vbus = read_vbus();
    d_max = (int)(DETECT_DUTY_MAX * (float)BSP_MOTOR_PWM_ARR);
    if (d_max < 20)
    {
        d_max = 20;
    }

    /* Ramp duty until target current (VESC ramps Iq the same way). */
    for (d = 2; d <= d_max; d += 2)
    {
        set_dc_inject((int16_t)d);
        delay_ms(DETECT_RAMP_MS);
        if (read_ia_ib(&ia, &ib) == 0U)
        {
            end_inject();
            return BSP_DETECT_ERR_CURRENT;
        }
        i_abs = fabsf(ia);
        if (i_abs >= current_a)
        {
            break;
        }
    }

    delay_ms(DETECT_SETTLE_MS);

    if (samples < 5)
    {
        samples = 5;
    }
    for (i = 0; i < samples; i++)
    {
        float vph;
        if (read_ia_ib(&ia, &ib) == 0U)
        {
            continue;
        }
        i_abs = fabsf(ia);
        if (i_abs < 0.02f)
        {
            continue;
        }
        vph = duty_to_vphase(vbus, (uint16_t)d);
        v_sum += vph;
        i_sum += i_abs;
        n++;
        delay_ms(1U);
    }

    end_inject();

    if (n < 3)
    {
        return BSP_DETECT_ERR_CURRENT;
    }
    *r_ohm = (v_sum / (float)n) / (i_sum / (float)n);
    s_res.i_meas_a = i_sum / (float)n;
    s_res.vbus_v = vbus;
    return BSP_DETECT_OK;
}

/**
 * Single short pulse → inductance estimate.
 * From zero: I(t)=(V/R)(1-e^{-tR/L}) ⇒ L = -t R / ln(1 - I R / V)
 * Fallback L = V*t/I when R unknown / near-linear.
 */
static float pulse_l_h(float vbus, float rs, int16_t d_counts, uint32_t pulse_us,
                       float *i_peak_out)
{
    float ia0;
    float ib0;
    float ia1;
    float ib1;
    float i0;
    float i1;
    float di;
    float vph;
    float l;
    float x;
    uint16_t d_abs = (uint16_t)((d_counts >= 0) ? d_counts : (int16_t)(-d_counts));

    pwm_idle();
    delay_us(400U);
    if (read_ia_ib(&ia0, &ib0) == 0U)
    {
        return -1.0f;
    }
    i0 = ia0;

    set_dc_inject(d_counts);
    delay_us(pulse_us);
    if (read_ia_ib(&ia1, &ib1) == 0U)
    {
        pwm_idle();
        return -1.0f;
    }
    i1 = ia1;
    pwm_idle();

    di = i1 - i0;
    if (fabsf(di) < 0.01f)
    {
        return -1.0f;
    }
    vph = duty_to_vphase(vbus, d_abs);
    if (vph < 0.05f)
    {
        return -1.0f;
    }
    if (i_peak_out)
    {
        *i_peak_out = fabsf(di);
    }

    if (rs > 0.05f)
    {
        x = (fabsf(di) * rs) / vph;
        if (x > 0.02f && x < 0.95f)
        {
            l = -((float)pulse_us * 1.0e-6f) * rs / logf(1.0f - x);
            if (l > 1.0e-6f && l < 0.1f)
            {
                return l;
            }
        }
    }
    /* Linear fallback (short pulse): L = V dt / dI */
    l = (vph * ((float)pulse_us * 1.0e-6f)) / fabsf(di);
    return l;
}

int BSP_MotorDetect_MeasureL(float current_goal_a, int samples,
                             float *l_uh, float *ld_lq_diff_uh)
{
    float vbus;
    float rs = s_rs_rt;
    float l_sum = 0.0f;
    float i_sum = 0.0f;
    int ok = 0;
    int d;
    int d_try;
    int i;
    int rc;

    if ((l_uh == 0) || (samples < 1))
    {
        return BSP_DETECT_ERR_PARAM;
    }
    if (current_goal_a < DETECT_I_MIN_A)
    {
        current_goal_a = DETECT_I_MIN_A;
    }
    if (current_goal_a > DETECT_I_ABS_MAX_A)
    {
        current_goal_a = DETECT_I_ABS_MAX_A;
    }

    rc = begin_inject();
    if (rc != BSP_DETECT_OK)
    {
        return rc;
    }

    vbus = read_vbus();

    /* Find duty that reaches ~current_goal (VESC duty search 0.02..0.5). */
    d = 8;
    for (d_try = 4; d_try < (int)(0.45f * (float)BSP_MOTOR_PWM_ARR); d_try = (d_try * 3) / 2 + 1)
    {
        float ip = 0.0f;
        float ltmp = pulse_l_h(vbus, rs, (int16_t)d_try, DETECT_PULSE_US, &ip);
        (void)ltmp;
        d = d_try;
        if (ip >= current_goal_a * 0.7f)
        {
            break;
        }
        delay_ms(5U);
    }

    if (samples < 10)
    {
        samples = 10;
    }

    for (i = 0; i < samples; i++)
    {
        float ip = 0.0f;
        float l;
        int16_t signed_d = ((i & 1) == 0) ? (int16_t)d : (int16_t)(-d);
        l = pulse_l_h(vbus, rs, signed_d, DETECT_PULSE_US, &ip);
        if (l > 0.0f)
        {
            l_sum += l;
            i_sum += ip;
            ok++;
        }
        delay_ms(5U);
    }

    end_inject();

    if (ok < 3)
    {
        return BSP_DETECT_ERR_CURRENT;
    }

    /* VESC scales inductance by 0.9 (observer prefers underestimate). */
    *l_uh = (l_sum / (float)ok) * 1.0e6f * DETECT_IND_SCALE;
    if (ld_lq_diff_uh)
    {
        *ld_lq_diff_uh = 0.0f; /* HFI not available on this board */
    }
    s_res.i_meas_a = i_sum / (float)ok;
    s_res.vbus_v = vbus;
    return BSP_DETECT_OK;
}

/**
 * Flux linkage via open-loop V/f spin + AS5600 ω.
 * VESC: λ = (|V| - R|I|)/ωe - |I|*L
 * Here |V| ≈ commanded V/f amplitude as phase voltage estimate.
 */
int BSP_MotorDetect_MeasureFlux(float current_a, float erpm_target,
                                float rs, float ls, float *flux_wb)
{
    float vbus;
    float amp_frac;
    float v_est;
    float ia;
    float ib;
    float i_mag;
    float rpm;
    float omega_e;
    float lambda;
    uint32_t t0;
    int i;
    const BSP_AS5600_State_t *as;

    if ((flux_wb == 0) || (rs <= 0.0f) || (ls <= 0.0f) || (erpm_target < 500.0f))
    {
        return BSP_DETECT_ERR_PARAM;
    }
    if (BSP_FOC_GetState()->running != 0U)
    {
        return BSP_DETECT_ERR_RUNNING;
    }

    vbus = read_vbus();
    /* Map erpm → speed permille roughly: 1350 rpm mech max ≈ 1350*7=9450 erpm */
    {
        float rpm_mech = erpm_target / (float)MOTOR_POLE_PAIRS;
        float pm = (rpm_mech - 15.0f) * 1000.0f / (1350.0f - 15.0f);
        uint16_t sp;
        if (pm < 80.0f) { pm = 80.0f; }
        if (pm > 700.0f) { pm = 700.0f; }
        sp = (uint16_t)pm;

        /* Limit Imax so voltage stays near current_a * rs scale */
        {
            float imax_pm = (current_a / 1.5f) * 1000.0f;
            if (imax_pm < 200.0f) { imax_pm = 200.0f; }
            if (imax_pm > 1000.0f) { imax_pm = 1000.0f; }
            BSP_FOC_SetImaxPm((uint16_t)imax_pm);
        }
        BSP_FOC_SetCtrlMode(BSP_FOC_CTRL_SPEED);
        BSP_FOC_SetSpeed(sp);
        BSP_FOC_Start();
    }

    /* Ramp / settle ~2.5 s */
    t0 = g_millis;
    while ((g_millis - t0) < 2500U)
    {
        BSP_AS5600_Update();
        as = BSP_AS5600_GetState();
        BSP_FOC_OnEncoder(as->raw, (as->ok != 0U) && (as->mag_ok != 0U) ? 1U : 0U);
        BSP_FOC_OnEncoderCum(as->cum_raw);
        BSP_FOC_SpeedLoop();
        delay_ms(2U);
    }

    /* Average samples — V_phase ≈ Vbus * amp / ARR (SPWM peak vs mid) */
    {
        float v_acc = 0.0f;
        float i_acc = 0.0f;
        float w_acc = 0.0f;
        int n = 0;
        for (i = 0; i < 400; i++)
        {
            const BSP_FOC_State_t *st;
            uint16_t amp;
            BSP_AS5600_Update();
            as = BSP_AS5600_GetState();
            BSP_FOC_OnEncoder(as->raw, (as->ok != 0U) && (as->mag_ok != 0U) ? 1U : 0U);
            BSP_FOC_OnEncoderCum(as->cum_raw);
            BSP_FOC_SpeedLoop();
            st = BSP_FOC_GetState();
            amp = BSP_FOC_GetAmp();
            (void)read_ia_ib(&ia, &ib);
            i_mag = 0.5f * (fabsf(ia) + fabsf(ib));
            rpm = fabsf(st->rpm);
            amp_frac = (float)amp / (float)BSP_MOTOR_PWM_ARR;
            if (amp_frac > 0.95f) { amp_frac = 0.95f; }
            v_est = vbus * amp_frac;
            omega_e = rpm * (float)MOTOR_POLE_PAIRS * (2.0f * (float)M_PI / 60.0f);
            if (omega_e > 30.0f)
            {
                v_acc += v_est;
                i_acc += i_mag;
                w_acc += omega_e;
                n++;
            }
            delay_ms(2U);
        }
        BSP_FOC_Stop();
        delay_ms(200U);

        if (n < 50)
        {
            return BSP_DETECT_ERR_FLUX;
        }
        v_est = v_acc / (float)n;
        i_mag = i_acc / (float)n;
        omega_e = w_acc / (float)n;
        /* Same formula as conf_general_measure_flux_linkage_openloop */
        lambda = (v_est - rs * i_mag) / omega_e - i_mag * ls;
        if (lambda < 1.0e-5f)
        {
            return BSP_DETECT_ERR_FLUX;
        }
        *flux_wb = lambda;
        s_res.i_meas_a = i_mag;
        s_res.vbus_v = vbus;
    }
    return BSP_DETECT_OK;
}

static void calc_gains(float r, float l, float lambda)
{
    float bw = 1.0f / (DETECT_TC_US * 1.0e-6f);
    s_res.kp = l * bw;
    s_res.ki = r * bw;
    if (lambda > 1.0e-6f)
    {
        s_res.observer_gain = (1.0e-3f / (lambda * lambda)) * 1.0e6f;
    }
    else
    {
        s_res.observer_gain = 0.0f;
    }
}

int BSP_MotorDetect_RunAll(float max_power_loss)
{
    float r = 0.0f;
    float l_uh = 0.0f;
    float ldq = 0.0f;
    float flux = 0.0f;
    float i_start;
    float i_last;
    float i;
    float r_tmp;
    int rc;

    if (s_busy != 0U)
    {
        return BSP_DETECT_ERR_BUSY;
    }
    if (BSP_FOC_GetState()->running != 0U)
    {
        return BSP_DETECT_ERR_RUNNING;
    }
    if (max_power_loss < 0.5f)
    {
        max_power_loss = 0.5f;
    }
    if (max_power_loss > 12.0f)
    {
        max_power_loss = 12.0f;
    }

    s_busy = 1U;
    s_res.valid = 0U;
    s_res.status = 0;
    s_res.stage = 1U;

    /* --- measure_r_l_imax style current search --- */
    i_start = DETECT_I_MIN_A;
    i_last = i_start;
    for (i = i_start; i < DETECT_I_ABS_MAX_A; i *= 1.5f)
    {
        rc = BSP_MotorDetect_MeasureR(i, 5, &r_tmp);
        if (rc != BSP_DETECT_OK)
        {
            s_res.status = (int8_t)rc;
            s_busy = 0U;
            return rc;
        }
        i_last = i;
        if ((i * i * r_tmp * 1.5f) >= (max_power_loss / 5.0f))
        {
            break;
        }
    }

    rc = BSP_MotorDetect_MeasureR(i_last, 80, &r);
    if (rc != BSP_DETECT_OK)
    {
        s_res.status = (int8_t)rc;
        s_busy = 0U;
        return rc;
    }
    s_rs_rt = r;
    s_res.rs_ohm = r;
    s_res.stage = 2U;

    rc = BSP_MotorDetect_MeasureL(i_last, 40, &l_uh, &ldq);
    if (rc != BSP_DETECT_OK)
    {
        s_res.status = (int8_t)rc;
        s_busy = 0U;
        return rc;
    }
    s_ls_rt = l_uh * 1.0e-6f;
    s_res.ls_uh = l_uh;
    s_res.ls_h = s_ls_rt;
    s_res.i_max_a = sqrtf(max_power_loss / r / 1.5f);
    if (s_res.i_max_a > DETECT_I_ABS_MAX_A)
    {
        s_res.i_max_a = DETECT_I_ABS_MAX_A;
    }
    s_res.stage = 3U;

    /* Flux: open-loop spin at moderate erpm */
    {
        float i_flux = s_res.i_max_a / 2.5f;
        if (i_flux < DETECT_I_MIN_A)
        {
            i_flux = DETECT_I_MIN_A;
        }
        rc = BSP_MotorDetect_MeasureFlux(i_flux, 3500.0f, r, s_ls_rt, &flux);
        if (rc == BSP_DETECT_OK)
        {
            s_flux_rt = flux;
            s_res.flux_wb = flux;
        }
        else
        {
            /* R/L still useful without flux */
            s_res.flux_wb = 0.0f;
            s_res.status = (int8_t)rc;
        }
    }

    calc_gains(r, s_ls_rt, s_res.flux_wb);
    s_res.stage = 4U;
    s_res.valid = 1U;
    if (s_res.status == 0)
    {
        s_res.status = BSP_DETECT_OK;
    }
    s_busy = 0U;
    return (s_res.flux_wb > 0.0f) ? BSP_DETECT_OK : (int)s_res.status;
}

void BSP_MotorDetect_Init(void)
{
    s_res.rs_ohm = MOTOR_RS_OHM;
    s_res.ls_h = MOTOR_LS_H;
    s_res.ls_uh = MOTOR_LS_H * 1.0e6f;
    s_res.flux_wb = 0.0f;
    s_res.valid = 0U;
    s_res.status = 0;
    s_res.stage = 0U;
    s_rs_rt = MOTOR_RS_OHM;
    s_ls_rt = MOTOR_LS_H;
    s_flux_rt = 0.0f;
    BSP_Vbus_Init();
}

const BSP_MotorDetect_Result_t *BSP_MotorDetect_GetResult(void)
{
    return &s_res;
}

void BSP_MotorDetect_Apply(void)
{
    if (s_res.valid != 0U)
    {
        s_rs_rt = s_res.rs_ohm;
        s_ls_rt = s_res.ls_h;
        s_flux_rt = s_res.flux_wb;
    }
}

float BSP_MotorDetect_GetRs(void)
{
    return s_rs_rt;
}

float BSP_MotorDetect_GetLs(void)
{
    return s_ls_rt;
}

float BSP_MotorDetect_GetFlux(void)
{
    return s_flux_rt;
}
