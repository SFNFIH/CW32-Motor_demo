/**
 * @file    BSP_MotorDetect.c
 * @brief   VESC-style motor parameter detection for CW32 V/f + ABC inject
 *
 * Gaps filled vs first port (对照 vedderb/bldc detect_apply_all_foc):
 *  - DC current offset cal at RunAll start (mcpwm_foc_dc_cal)
 *  - Dead-time compensation on resistance voltage
 *  - kp/ki tc = 1000 µs (detect_apply uses 1000, not flux path's 1500)
 *  - Apply i_max → FOC Imax (l_current_max)
 *  - AS5600 encoder offset/ratio/inverted (三轴锁相, 简化 encoder_detect)
 *  - Wait motor stop after flux before encoder step
 *  - Keep inject across R search samples (stop_after=false 风格)
 */
#include "BSP_MotorDetect.h"
#include "BSP_MOTOR.h"
#include "BSP_Current.h"
#include "BSP_Vbus.h"
#include "BSP_FOC.h"
#include "BSP_AS5600.h"
#include "BSP_HFI.h"
#include "BSP_BEMF.h"
#include "BSP_motor_params.h"
#include "cw32l012.h"

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/* detect_apply_all_foc → conf_general_calc_apply_foc_cc_kp_ki_gain(..., 1000) */
#define DETECT_TC_US          1000.0f
#define DETECT_I_MIN_A        0.08f
#define DETECT_I_ABS_MAX_A    1.60f
#define DETECT_DUTY_MAX       0.35f
#define DETECT_SETTLE_MS      50U
#define DETECT_RAMP_MS        1U
/* 死区对有效占空的近似扣除 (timer ticks); 互补中心对齐经验值 */
#define DETECT_DT_COMP        ((float)BSP_MOTOR_PWM_DEADTIME * 0.5f)

extern volatile uint32_t g_millis;

static BSP_MotorDetect_Result_t s_res;
static float s_rs_rt = MOTOR_RS_OHM;
static float s_ls_rt = MOTOR_LS_H;
static float s_flux_rt = 0.0f;
static float s_enc_off_rt;
static uint8_t s_enc_inv_rt;
static uint8_t s_busy;
static uint8_t s_inject_on;

static void delay_ms(uint32_t ms)
{
    uint32_t t0 = g_millis;
    while ((g_millis - t0) < ms)
    {
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

static float ang_diff_deg(float a, float b)
{
    float d = a - b;
    while (d > 180.0f) { d -= 360.0f; }
    while (d < -180.0f) { d += 360.0f; }
    return d;
}

static float raw_to_deg(uint16_t raw)
{
    return (float)raw * (360.0f / 4096.0f);
}

/**
 * axis: 0=A+B- (elec≈0°), 1=B+C- (≈120°), 2=C+A- (≈240°)
 * Matches FOC phase_override axes for encoder_detect / hall_detect style locking.
 */
static void set_dc_inject_axis(uint8_t axis, int16_t d_counts)
{
    const uint16_t mid = (uint16_t)(BSP_MOTOR_PWM_ARR / 2U);
    int32_t da = (int32_t)mid;
    int32_t db = (int32_t)mid;
    int32_t dc = (int32_t)mid;
    int32_t d = (int32_t)d_counts;

    if (axis == 1U)
    {
        db += d;
        dc -= d;
    }
    else if (axis == 2U)
    {
        dc += d;
        da -= d;
    }
    else
    {
        da += d;
        db -= d;
    }

    if (da < 0) { da = 0; }
    if (db < 0) { db = 0; }
    if (dc < 0) { dc = 0; }
    if (da > (int32_t)BSP_MOTOR_PWM_ARR) { da = (int32_t)BSP_MOTOR_PWM_ARR; }
    if (db > (int32_t)BSP_MOTOR_PWM_ARR) { db = (int32_t)BSP_MOTOR_PWM_ARR; }
    if (dc > (int32_t)BSP_MOTOR_PWM_ARR) { dc = (int32_t)BSP_MOTOR_PWM_ARR; }
    BSP_MOTOR_SetPhaseDuty((uint16_t)da, (uint16_t)db, (uint16_t)dc);
}

static void set_dc_inject(int16_t d_counts)
{
    set_dc_inject_axis(0U, d_counts);
}

static void pwm_idle(void)
{
    uint16_t mid = (uint16_t)(BSP_MOTOR_PWM_ARR / 2U);
    BSP_MOTOR_SetPhaseDuty(mid, mid, mid);
}

/** Phase voltage with dead-time compensation (VESC FOC voltages include DT comp). */
static float duty_to_vphase(float vbus, uint16_t d_counts)
{
    float d_eff = (float)d_counts - DETECT_DT_COMP;
    if (d_eff < 1.0f)
    {
        d_eff = 1.0f;
    }
    return vbus * (d_eff / (float)BSP_MOTOR_PWM_ARR);
}

static int begin_inject(void)
{
    const BSP_FOC_State_t *st = BSP_FOC_GetState();
    if (st->running != 0U)
    {
        return BSP_DETECT_ERR_RUNNING;
    }
    if (s_inject_on != 0U)
    {
        return BSP_DETECT_OK;
    }
    BSP_MOTOR_DisablePwmIrq();
    BSP_MOTOR_Stop();
    delay_ms(5U);
    BSP_Current_Calibrate();
    BSP_MOTOR_Start();
    pwm_idle();
    delay_ms(2U);
    s_inject_on = 1U;
    return BSP_DETECT_OK;
}

static void end_inject(void)
{
    if (s_inject_on == 0U)
    {
        return;
    }
    pwm_idle();
    delay_ms(2U);
    BSP_MOTOR_Stop();
    s_inject_on = 0U;
}

/**
 * @param stop_after 0=保持注入(供 R 搜索连续采样, 对齐 VESC stop_after=false)
 */
static int measure_r_inner(float current_a, int samples, int stop_after, float *r_ohm)
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

    if (stop_after != 0)
    {
        end_inject();
    }

    if (n < 3)
    {
        end_inject();
        return BSP_DETECT_ERR_CURRENT;
    }
    *r_ohm = (v_sum / (float)n) / (i_sum / (float)n);
    s_res.i_meas_a = i_sum / (float)n;
    s_res.vbus_v = vbus;
    return BSP_DETECT_OK;
}

int BSP_MotorDetect_MeasureR(float current_a, int samples, float *r_ohm)
{
    return measure_r_inner(current_a, samples, 1, r_ohm);
}

int BSP_MotorDetect_MeasureL(float current_goal_a, int samples,
                             float *l_uh, float *ld_lq_diff_uh)
{
    BSP_HFI_LResult_t hfi;
    int sweeps;
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

    /* VESC measure_inductance averages samples/10 full HFI buffers */
    sweeps = samples / 10;
    if (sweeps < 4) { sweeps = 4; }
    if (sweeps > 20) { sweeps = 20; }

    end_inject(); /* HFI manages its own PWM */
    rc = BSP_HFI_MeasureInductanceCurrent(current_goal_a, sweeps, &hfi);
    if (rc != 0 || hfi.ok == 0U)
    {
        return BSP_DETECT_ERR_CURRENT;
    }

    *l_uh = hfi.l_avg_h * 1.0e6f;
    if (ld_lq_diff_uh)
    {
        *ld_lq_diff_uh = hfi.ld_lq_diff_h * 1.0e6f;
    }
    s_res.ls_h = hfi.l_avg_h;
    s_res.ls_uh = *l_uh;
    s_res.ld_lq_diff_h = hfi.ld_lq_diff_h;
    s_res.i_meas_a = hfi.i_avg_a;
    s_res.vbus_v = read_vbus();
    return BSP_DETECT_OK;
}

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
    float lambda_driven = 0.0f;
    float lambda_ud = 0.0f;
    float lambda;
    uint32_t t0;
    int i;
    int n_driven = 0;
    int n_ud = 0;
    const BSP_AS5600_State_t *as;

    if ((flux_wb == 0) || (rs <= 0.0f) || (ls <= 0.0f) || (erpm_target < 500.0f))
    {
        return BSP_DETECT_ERR_PARAM;
    }
    if (BSP_FOC_GetState()->running != 0U)
    {
        return BSP_DETECT_ERR_RUNNING;
    }
    end_inject();

    vbus = read_vbus();
    {
        float rpm_mech = erpm_target / (float)MOTOR_POLE_PAIRS;
        float pm = (rpm_mech - 15.0f) * 1000.0f / (1350.0f - 15.0f);
        uint16_t sp;
        float imax_pm;
        if (pm < 80.0f) { pm = 80.0f; }
        if (pm > 700.0f) { pm = 700.0f; }
        sp = (uint16_t)pm;
        imax_pm = (current_a / 1.5f) * 1000.0f;
        if (imax_pm < 200.0f) { imax_pm = 200.0f; }
        if (imax_pm > 1000.0f) { imax_pm = 1000.0f; }
        BSP_FOC_SetImaxPm((uint16_t)imax_pm);
        BSP_FOC_SetCtrlMode(BSP_FOC_CTRL_SPEED);
        BSP_FOC_SetSpeed(sp);
        BSP_FOC_Start();
    }

    /* 爬升 */
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

    /* --- driven λ = (|V|-R|I|)/ωe - |I|L --- */
    {
        float v_acc = 0.0f;
        float i_acc = 0.0f;
        float w_acc = 0.0f;
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
                n_driven++;
            }
            delay_ms(2U);
        }
        if (n_driven >= 50)
        {
            v_est = v_acc / (float)n_driven;
            i_mag = i_acc / (float)n_driven;
            omega_e = w_acc / (float)n_driven;
            lambda_driven = (v_est - rs * i_mag) / omega_e - i_mag * ls;
            if (lambda_driven < 0.0f) { lambda_driven = 0.0f; }
            s_res.i_meas_a = i_mag;
        }
    }

    /* --- undriven: stop PWM, sample BEMF while coasting (VESC λ = |V|/ω) --- */
    BSP_FOC_Stop();
    delay_ms(5U); /* H-bridge settle */
    BSP_BEMF_Enter();
    {
        float link_sum = 0.0f;
        t0 = g_millis;
        while ((g_millis - t0) < 2000U)
        {
            float va;
            float vb;
            float vc;
            float valpha;
            float vbeta;
            float vmag;
            float rpm_now;
            float we;

            BSP_AS5600_Update();
            as = BSP_AS5600_GetState();
            rpm_now = fabsf((float)as->rpm_x10) * 0.1f;
            we = rpm_now * (float)MOTOR_POLE_PAIRS * (2.0f * (float)M_PI / 60.0f);

            if (we < 25.0f)
            {
                /* 转速过低则结束无驱窗口 */
                if (n_ud > 10) { break; }
                delay_ms(2U);
                continue;
            }

            if (BSP_BEMF_ReadVolt(&va, &vb, &vc) != 0U)
            {
                BSP_BEMF_Clarke(va, vb, vc, &valpha, &vbeta);
                vmag = sqrtf(valpha * valpha + vbeta * vbeta);
                if (vmag > 0.05f)
                {
                    link_sum += vmag / we;
                    n_ud++;
                }
            }
            delay_ms(1U);
        }
        BSP_BEMF_Exit();
        s_res.flux_ud_samples = (uint16_t)n_ud;
        if (n_ud > 0)
        {
            lambda_ud = link_sum / (float)n_ud;
        }
    }

    /* 等停转再继续后续 encoder 步骤 */
    t0 = g_millis;
    while ((g_millis - t0) < 3000U)
    {
        BSP_AS5600_Update();
        as = BSP_AS5600_GetState();
        if (fabsf((float)as->rpm_x10) < 300.0f)
        {
            break;
        }
        delay_ms(20U);
    }
    delay_ms(100U);

    s_res.flux_driven_wb = lambda_driven;
    s_res.flux_undriven_wb = lambda_ud;
    s_res.vbus_v = vbus;

    /* VESC: undriven_samples > 60 → 采用无驱 */
    if (n_ud > 60 && lambda_ud > 1.0e-5f)
    {
        lambda = lambda_ud;
    }
    else if (lambda_driven > 1.0e-5f)
    {
        lambda = lambda_driven;
    }
    else if (lambda_ud > 1.0e-5f)
    {
        lambda = lambda_ud;
    }
    else
    {
        return BSP_DETECT_ERR_FLUX;
    }

    *flux_wb = lambda;
    return BSP_DETECT_OK;
}

/**
 * Simplified mcpwm_foc_encoder_detect / hall-style axis lock:
 * lock elec 0°/120°/240°, read AS5600, derive offset, ratio≈pole_pairs, inverted.
 */
int BSP_MotorDetect_MeasureEncoder(float current_a)
{
    float vbus;
    float ia;
    float ib;
    float deg[3];
    int d = 0;
    int d_max;
    int axis;
    int rc;
    float d01;
    float d12;
    float expect_mech;
    float ratio;
    const BSP_AS5600_State_t *as;

    if (current_a < DETECT_I_MIN_A)
    {
        current_a = DETECT_I_MIN_A;
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
    (void)vbus;
    d_max = (int)(DETECT_DUTY_MAX * (float)BSP_MOTOR_PWM_ARR);

    for (axis = 0; axis < 3; axis++)
    {
        for (d = 2; d <= d_max; d += 2)
        {
            set_dc_inject_axis((uint8_t)axis, (int16_t)d);
            delay_ms(DETECT_RAMP_MS);
            if (read_ia_ib(&ia, &ib) == 0U)
            {
                end_inject();
                return BSP_DETECT_ERR_CURRENT;
            }
            if (fabsf(ia) + fabsf(ib) >= current_a)
            {
                break;
            }
        }
        delay_ms(150U);
        BSP_AS5600_Update();
        as = BSP_AS5600_GetState();
        if ((as->ok == 0U) || (as->mag_ok == 0U))
        {
            end_inject();
            s_res.enc_ok = 0U;
            return BSP_DETECT_ERR_ENCODER;
        }
        deg[axis] = raw_to_deg(as->raw);
    }

    end_inject();

    d01 = ang_diff_deg(deg[1], deg[0]);
    d12 = ang_diff_deg(deg[2], deg[1]);
    expect_mech = 120.0f / (float)MOTOR_POLE_PAIRS;

    /* ratio = Δθ_elec / Δθ_mech */
    if (fabsf(d01) < 0.5f)
    {
        s_res.enc_ok = 0U;
        return BSP_DETECT_ERR_ENCODER;
    }
    ratio = 120.0f / fabsf(d01);
    s_res.enc_ratio = ratio;
    s_res.enc_inverted = (d01 < 0.0f) ? 1U : 0U;
    s_res.enc_offset_deg = deg[0];

    /* 合理性: ratio 应接近极对数; 两段步进符号一致 */
    if ((fabsf(ratio - (float)MOTOR_POLE_PAIRS) > 2.5f) ||
        ((d01 < 0.0f) != (d12 < 0.0f)) ||
        (fabsf(fabsf(d01) - expect_mech) > expect_mech * 0.85f &&
         fabsf(fabsf(d01) - expect_mech) > 8.0f))
    {
        /* 仍保存读数, 但标记不可靠 */
        s_res.enc_ok = 0U;
        return BSP_DETECT_ERR_ENCODER;
    }

    s_res.enc_ok = 1U;
    s_enc_off_rt = s_res.enc_offset_deg;
    s_enc_inv_rt = s_res.enc_inverted;
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
    s_res.enc_ok = 0U;
    s_res.stage = 1U;

    /* mcpwm_foc_dc_cal: 关 PWM 采电流偏置 */
    BSP_MOTOR_DisablePwmIrq();
    BSP_MOTOR_Stop();
    delay_ms(20U);
    BSP_Current_Calibrate();

    /* measure_r_l_imax: 连续注入搜索 (stop_after=false) → 终测 stop */
    i_start = DETECT_I_MIN_A;
    i_last = i_start;
    for (i = i_start; i < DETECT_I_ABS_MAX_A; i *= 1.5f)
    {
        rc = measure_r_inner(i, 5, 0, &r_tmp);
        if (rc != BSP_DETECT_OK)
        {
            end_inject();
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

    rc = measure_r_inner(i_last, 80, 1, &r);
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
    s_res.ld_lq_diff_h = ldq * 1.0e-6f;
    s_res.i_max_a = sqrtf(max_power_loss / r / 1.5f);
    if (s_res.i_max_a > DETECT_I_ABS_MAX_A)
    {
        s_res.i_max_a = DETECT_I_ABS_MAX_A;
    }
    s_res.stage = 3U;

    {
        float i_flux = s_res.i_max_a / 2.5f;
        if (i_flux < DETECT_I_MIN_A)
        {
            i_flux = DETECT_I_MIN_A;
        }
        /* VESC: erpm_per_sec=1800, duty=0.3; 此处用目标 erpm≈3500 */
        rc = BSP_MotorDetect_MeasureFlux(i_flux, 3500.0f, r, s_ls_rt, &flux);
        if (rc == BSP_DETECT_OK)
        {
            s_flux_rt = flux;
            s_res.flux_wb = flux;
        }
        else
        {
            s_res.flux_wb = 0.0f;
            s_res.status = (int8_t)rc;
        }
    }

    s_res.stage = 4U;
    {
        float i_enc = s_res.i_max_a / 3.0f;
        if (i_enc < DETECT_I_MIN_A)
        {
            i_enc = DETECT_I_MIN_A;
        }
        (void)BSP_MotorDetect_MeasureEncoder(i_enc);
    }

    calc_gains(r, s_ls_rt, s_res.flux_wb);
    s_res.stage = 5U;
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
    s_res.ld_lq_diff_h = 0.0f;
    s_res.flux_wb = 0.0f;
    s_res.flux_driven_wb = 0.0f;
    s_res.flux_undriven_wb = 0.0f;
    s_res.flux_ud_samples = 0U;
    s_res.valid = 0U;
    s_res.status = 0;
    s_res.stage = 0U;
    s_res.enc_ok = 0U;
    s_res.enc_offset_deg = 0.0f;
    s_res.enc_ratio = (float)MOTOR_POLE_PAIRS;
    s_res.enc_inverted = 0U;
    s_rs_rt = MOTOR_RS_OHM;
    s_ls_rt = MOTOR_LS_H;
    s_flux_rt = 0.0f;
    s_enc_off_rt = 0.0f;
    s_enc_inv_rt = 0U;
    s_inject_on = 0U;
    BSP_Vbus_Init();
    BSP_BEMF_Init();
}

const BSP_MotorDetect_Result_t *BSP_MotorDetect_GetResult(void)
{
    return &s_res;
}

void BSP_MotorDetect_Apply(void)
{
    if (s_res.valid == 0U)
    {
        return;
    }
    s_rs_rt = s_res.rs_ohm;
    s_ls_rt = s_res.ls_h;
    s_flux_rt = s_res.flux_wb;
    if (s_res.enc_ok != 0U)
    {
        s_enc_off_rt = s_res.enc_offset_deg;
        s_enc_inv_rt = s_res.enc_inverted;
    }
    /* 对齐 VESC: l_current_max = i_max */
    if (s_res.i_max_a > 0.05f)
    {
        float pm = (s_res.i_max_a / 1.5f) * 1000.0f;
        if (pm < 120.0f) { pm = 120.0f; }
        if (pm > 1000.0f) { pm = 1000.0f; }
        BSP_FOC_SetImaxPm((uint16_t)pm);
    }
    if (s_res.enc_inverted != 0U)
    {
        BSP_FOC_SetDirection(-1);
    }
}

float BSP_MotorDetect_GetRs(void) { return s_rs_rt; }
float BSP_MotorDetect_GetLs(void) { return s_ls_rt; }
float BSP_MotorDetect_GetFlux(void) { return s_flux_rt; }
float BSP_MotorDetect_GetEncOffsetDeg(void) { return s_enc_off_rt; }
uint8_t BSP_MotorDetect_GetEncInverted(void) { return s_enc_inv_rt; }
