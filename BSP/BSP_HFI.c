/**
 * @file    BSP_HFI.c
 * @brief   Six-vector HFI inductance (port of VESC mcpwm_foc HFI measure path)
 *
 * Algorithm (vedderb/bldc motor/mcpwm_foc.c ~4938-4964 + measure_inductance FFT):
 *   For angle k = 0..31:
 *     apply -Vhfi·(cos,sin) for one PWM period, sample iαβ → prev
 *     apply +Vhfi·(cos,sin) for one PWM period, sample iαβ → now
 *     di = now_proj - prev_proj
 *     invL[k] = f_zv * di / Vhfi
 *   FFT bin0 → offset (mean 1/L)
 *   FFT bin2 → 2nd harmonic of 1/L around circle
 *   amp = 2*|bin2|
 *   Ld = 1/(offset+amp), Lq = 1/(offset-amp)   (assume Ld < Lq)
 *   scale 0.9 like VESC
 */
#include "BSP_HFI.h"
#include "BSP_MOTOR.h"
#include "BSP_Current.h"
#include "BSP_Vbus.h"
#include "BSP_motor_params.h"
#include "cw32l012.h"

#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define HFI_N              32
#define HFI_SCALE          0.9f
#define HFI_SQRT3_BY_2     0.8660254037844386f
#define HFI_ONE_BY_SQRT3   0.5773502691896257f
#define HFI_FZV_HZ         ((float)BSP_MOTOR_PWM_HZ)  /* 15 kHz center-aligned period */
#define HFI_HALF_US        (1000000UL / (BSP_MOTOR_PWM_HZ)) /* one PWM period */
#define HFI_ERR_PARAM      (-5)
#define HFI_ERR_CURRENT    (-2)
#define HFI_ERR_VBUS       (-3)

/* From VESC util/utils_math.c — fundamental & 2nd-harmonic tables */
static const float s_sin1[HFI_N] = {
    0.000000f, 0.195090f, 0.382683f, 0.555570f, 0.707107f, 0.831470f, 0.923880f, 0.980785f,
    1.000000f, 0.980785f, 0.923880f, 0.831470f, 0.707107f, 0.555570f, 0.382683f, 0.195090f,
    0.000000f, -0.195090f, -0.382683f, -0.555570f, -0.707107f, -0.831470f, -0.923880f, -0.980785f,
    -1.000000f, -0.980785f, -0.923880f, -0.831470f, -0.707107f, -0.555570f, -0.382683f, -0.195090f
};
static const float s_cos1[HFI_N] = {
    1.000000f, 0.980785f, 0.923880f, 0.831470f, 0.707107f, 0.555570f, 0.382683f, 0.195090f,
    0.000000f, -0.195090f, -0.382683f, -0.555570f, -0.707107f, -0.831470f, -0.923880f, -0.980785f,
    -1.000000f, -0.980785f, -0.923880f, -0.831470f, -0.707107f, -0.555570f, -0.382683f, -0.195090f,
    -0.000000f, 0.195090f, 0.382683f, 0.555570f, 0.707107f, 0.831470f, 0.923880f, 0.980785f
};
static const float s_sin2[HFI_N] = {
    0.000000f, 0.382683f, 0.707107f, 0.923880f, 1.000000f, 0.923880f, 0.707107f, 0.382683f,
    0.000000f, -0.382683f, -0.707107f, -0.923880f, -1.000000f, -0.923880f, -0.707107f, -0.382683f,
    -0.000000f, 0.382683f, 0.707107f, 0.923880f, 1.000000f, 0.923880f, 0.707107f, 0.382683f,
    0.000000f, -0.382683f, -0.707107f, -0.923880f, -1.000000f, -0.923880f, -0.707107f, -0.382683f
};
static const float s_cos2[HFI_N] = {
    1.000000f, 0.923880f, 0.707107f, 0.382683f, 0.000000f, -0.382683f, -0.707107f, -0.923880f,
    -1.000000f, -0.923880f, -0.707107f, -0.382683f, -0.000000f, 0.382683f, 0.707107f, 0.923880f,
    1.000000f, 0.923880f, 0.707107f, 0.382683f, 0.000000f, -0.382683f, -0.707107f, -0.923880f,
    -1.000000f, -0.923880f, -0.707107f, -0.382683f, -0.000000f, 0.382683f, 0.707107f, 0.923880f
};

extern volatile uint32_t g_millis;

static void delay_us(uint32_t us)
{
    uint32_t ticks = (SystemCoreClock / 1000000UL) * us;
    uint32_t load = SysTick->LOAD + 1U;
    uint32_t start = SysTick->VAL;
    uint32_t elapsed = 0U;
    if (ticks == 0U) { return; }
    while (elapsed < ticks)
    {
        uint32_t now = SysTick->VAL;
        uint32_t delta = (now <= start) ? (start - now) : (start + load - now);
        elapsed += delta;
        start = now;
    }
}

static void delay_ms(uint32_t ms)
{
    uint32_t t0 = g_millis;
    while ((g_millis - t0) < ms) { }
}

static float read_vbus(void)
{
    float v = BSP_Vbus_ReadVolt();
    if (v < 6.0f || v > 30.0f) { v = MOTOR_VBUS_V; }
    return v;
}

static uint8_t read_iab(float *ia, float *ib)
{
    return BSP_Current_Read(ia, ib);
}

/** Clarke: iα=ia, iβ=(ia+2·ib)/√3 */
static void clarke(float ia, float ib, float *ialpha, float *ibeta)
{
    *ialpha = ia;
    *ibeta = (ia + 2.0f * ib) * HFI_ONE_BY_SQRT3;
}

/**
 * Apply αβ voltage via inverse Clarke + midpoint injection → phase duties.
 * Matches VESC mod_alpha/beta * Vbus*(2/3) phase voltage path in spirit.
 */
static void set_ab_voltage(float v_alpha, float v_beta, float vbus)
{
    float va = v_alpha;
    float vb = -0.5f * v_alpha + HFI_SQRT3_BY_2 * v_beta;
    float vc = -0.5f * v_alpha - HFI_SQRT3_BY_2 * v_beta;
    float vmax = va;
    float vmin = va;
    float vcom;
    float scale;
    const int32_t mid = (int32_t)(BSP_MOTOR_PWM_ARR / 2U);
    int32_t da;
    int32_t db;
    int32_t dc;

    if (vb > vmax) { vmax = vb; }
    if (vc > vmax) { vmax = vc; }
    if (vb < vmin) { vmin = vb; }
    if (vc < vmin) { vmin = vc; }
    vcom = 0.5f * (vmax + vmin);
    va -= vcom;
    vb -= vcom;
    vc -= vcom;

    if (vbus < 1.0f) { vbus = 1.0f; }
    scale = (float)BSP_MOTOR_PWM_ARR / vbus;
    da = mid + (int32_t)(va * scale);
    db = mid + (int32_t)(vb * scale);
    dc = mid + (int32_t)(vc * scale);
    if (da < 8) { da = 8; }
    if (db < 8) { db = 8; }
    if (dc < 8) { dc = 8; }
    if (da > (int32_t)BSP_MOTOR_PWM_ARR - 8) { da = (int32_t)BSP_MOTOR_PWM_ARR - 8; }
    if (db > (int32_t)BSP_MOTOR_PWM_ARR - 8) { db = (int32_t)BSP_MOTOR_PWM_ARR - 8; }
    if (dc > (int32_t)BSP_MOTOR_PWM_ARR - 8) { dc = (int32_t)BSP_MOTOR_PWM_ARR - 8; }
    BSP_MOTOR_SetPhaseDuty((uint16_t)da, (uint16_t)db, (uint16_t)dc);
}

static void pwm_idle(void)
{
    uint16_t mid = (uint16_t)(BSP_MOTOR_PWM_ARR / 2U);
    BSP_MOTOR_SetPhaseDuty(mid, mid, mid);
}

/* VESC utils_fft32_bin0 / bin2 */
static void fft_bin0(const float *x, float *real, float *imag)
{
    int i;
    float s = 0.0f;
    for (i = 0; i < HFI_N; i++) { s += x[i]; }
    *real = s / (float)HFI_N;
    *imag = 0.0f;
}

static void fft_bin2(const float *x, float *real, float *imag)
{
    int i;
    float re = 0.0f;
    float im = 0.0f;
    for (i = 0; i < HFI_N; i++)
    {
        re += x[i] * s_cos2[i];
        im -= x[i] * s_sin2[i];
    }
    *real = re / (float)HFI_N;
    *imag = im / (float)HFI_N;
}

/**
 * One 32-point six-vector sweep → invL[] and di[].
 * Timing: each polarity held for ~1 PWM period (HFI_HALF_US), matching
 * VESC is_samp_n toggle at foc_f_zv.
 */
static int hfi_sweep(float v_hfi, float vbus, float *inv_l, float *di_buf, float *i_acc)
{
    int k;
    float ia;
    float ib;
    float ialpha;
    float ibeta;
    float prev;
    float now;
    float di;
    int filled = 0;

    *i_acc = 0.0f;
    memset(inv_l, 0, sizeof(float) * (size_t)HFI_N);
    memset(di_buf, 0, sizeof(float) * (size_t)HFI_N);

    for (k = 0; k < HFI_N; k++)
    {
        float c = s_cos1[k];
        float s = s_sin1[k];

        /* is_samp_n == 0: apply -V, capture prev after settle */
        set_ab_voltage(-v_hfi * c, -v_hfi * s, vbus);
        delay_us(HFI_HALF_US);
        if (read_iab(&ia, &ib) == 0U) { return HFI_ERR_CURRENT; }
        clarke(ia, ib, &ialpha, &ibeta);
        prev = c * ialpha + s * ibeta;

        /* is_samp_n == 1: apply +V, capture now, di = now - prev */
        set_ab_voltage(v_hfi * c, v_hfi * s, vbus);
        delay_us(HFI_HALF_US);
        if (read_iab(&ia, &ib) == 0U) { return HFI_ERR_CURRENT; }
        clarke(ia, ib, &ialpha, &ibeta);
        now = c * ialpha + s * ibeta;
        di = now - prev;
        di_buf[k] = di;

        /* VESC only stores when di > 0.01 */
        if (di > 0.01f)
        {
            inv_l[k] = (HFI_FZV_HZ * di) / v_hfi;
            filled++;
        }
        else if (di < -0.01f)
        {
            /* polarity flip: still usable as |1/L| */
            inv_l[k] = (HFI_FZV_HZ * (-di)) / v_hfi;
            filled++;
        }
        *i_acc += fabsf(di);
    }

    pwm_idle();
    if (filled < (HFI_N / 2))
    {
        return HFI_ERR_CURRENT;
    }
    *i_acc /= (float)HFI_N;
    return 0;
}

int BSP_HFI_MeasureInductance(float duty_frac, int sweeps, BSP_HFI_LResult_t *out)
{
    float vbus;
    float v_hfi;
    float inv_l[HFI_N];
    float di_buf[HFI_N];
    float l_sum = 0.0f;
    float diff_sum = 0.0f;
    float i_sum = 0.0f;
    float ld_sum = 0.0f;
    float lq_sum = 0.0f;
    int n_ok = 0;
    int s;
    int rc;

    if (out == 0) { return HFI_ERR_PARAM; }
    memset(out, 0, sizeof(*out));

    if (duty_frac < 0.02f) { duty_frac = 0.02f; }
    if (duty_frac > 0.50f) { duty_frac = 0.50f; }
    if (sweeps < 1) { sweeps = 1; }
    if (sweeps > 40) { sweeps = 40; }

    BSP_MOTOR_DisablePwmIrq();
    BSP_MOTOR_Stop();
    delay_ms(5U);
    BSP_Current_Calibrate();
    BSP_MOTOR_Start();
    pwm_idle();
    delay_ms(2U);

    vbus = read_vbus();
    /* Same as mcpwm_foc_measure_inductance:
     * Vhfi = duty * Vbus * (2/3) * √3/2 */
    v_hfi = duty_frac * vbus * (2.0f / 3.0f) * HFI_SQRT3_BY_2;
    if (v_hfi < 0.3f)
    {
        BSP_MOTOR_Stop();
        return HFI_ERR_VBUS;
    }

    for (s = 0; s < sweeps; s++)
    {
        float i_acc = 0.0f;
        float real0;
        float imag0;
        float real2;
        float imag2;
        float offset;
        float amplitude;
        float ld;
        float lq;

        rc = hfi_sweep(v_hfi, vbus, inv_l, di_buf, &i_acc);
        if (rc != 0)
        {
            delay_ms(5U);
            continue;
        }

        fft_bin0(inv_l, &real0, &imag0);
        fft_bin2(inv_l, &real2, &imag2);
        offset = real0;
        amplitude = sqrtf(real2 * real2 + imag2 * imag2) * 2.0f;

        if (offset <= amplitude + 1.0f)
        {
            /* not enough saliency / bad SNR — skip */
            delay_ms(5U);
            continue;
        }

        ld = 1.0f / (offset + amplitude);
        lq = 1.0f / (offset - amplitude);
        if (ld < 1.0e-6f || lq < 1.0e-6f || ld > 0.1f || lq > 0.1f)
        {
            delay_ms(5U);
            continue;
        }

        /* VESC assumes Ld < Lq for the (offset±amp) assignment; swap if needed */
        if (ld > lq)
        {
            float tmp = ld;
            ld = lq;
            lq = tmp;
        }

        l_sum += 0.5f * (ld + lq);
        diff_sum += (lq - ld);
        ld_sum += ld;
        lq_sum += lq;
        i_sum += i_acc;
        n_ok++;
        delay_ms(5U);
    }

    pwm_idle();
    delay_ms(2U);
    BSP_MOTOR_Stop();

    if (n_ok < 1)
    {
        return HFI_ERR_CURRENT;
    }

    out->l_avg_h = (l_sum / (float)n_ok) * HFI_SCALE;
    out->ld_lq_diff_h = (diff_sum / (float)n_ok) * HFI_SCALE;
    out->ld_h = (ld_sum / (float)n_ok) * HFI_SCALE;
    out->lq_h = (lq_sum / (float)n_ok) * HFI_SCALE;
    out->i_avg_a = i_sum / (float)n_ok;
    out->v_hfi_v = v_hfi;
    out->ok = 1U;
    return 0;
}

int BSP_HFI_MeasureInductanceCurrent(float current_goal_a, int sweeps,
                                     BSP_HFI_LResult_t *out)
{
    float duty_last = 0.02f;
    float d;
    BSP_HFI_LResult_t tmp;
    int rc;

    if (out == 0) { return HFI_ERR_PARAM; }
    if (current_goal_a < 0.05f) { current_goal_a = 0.05f; }

    /* VESC: for (i = 0.02; i < 0.5; i *= 1.5) until current reached */
    for (d = 0.02f; d < 0.50f; d *= 1.5f)
    {
        if (d > 0.50f) { d = 0.50f; }
        rc = BSP_HFI_MeasureInductance(d, 2, &tmp);
        duty_last = d;
        if (rc == 0 && tmp.i_avg_a >= current_goal_a)
        {
            break;
        }
        if (d >= 0.49f) { break; }
    }

    rc = BSP_HFI_MeasureInductance(duty_last, sweeps, out);
    return rc;
}
