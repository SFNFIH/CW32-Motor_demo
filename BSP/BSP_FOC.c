/**
 * @file    BSP_FOC.c
 * @brief   速度闭环 V/f + AS5600 测速; 降低下限以支持低速
 *          (角度闭环待 V/f 低速验证后再加)
 */
#include "BSP_FOC.h"
#include "BSP_MOTOR.h"
#include "BSP_motor_params.h"
#include "cw32l012_atim.h"
#include "cw32l012_btim.h"
#include "cw32l012_sysctrl.h"

#define SIN_N            256U

#define RPM_MIN_X10      150     /* 15 rpm */
#define RPM_MAX_X10      13500   /* 1350 rpm */

#define FE_MIN_X10       20U     /* 2 Hz 电 ≈ 17 rpm @7pp */
#define FE_MAX_X10       1800U

/* 低速勿顶满: 100~300rpm 过压会齿槽抖 */
#define AMP_MIN          950U
#define AMP_MAX          3200U

#define INC_SLEW_UP      3UL
#define INC_SLEW_DN      16UL

#define MAX_LEAD_X10     120
#define MAX_LEAD_LS_X10  50      /* <400rpm 更紧 */
#define MAX_LEAD_HS_X10  140
#define TRIM_MAX         100
#define TRIM_MAX_LS      40

#define SP_KP_NUM        1
#define SP_KP_DEN        60
#define SP_KI_NUM        1
#define SP_KI_DEN        400
#define SP_I_LIM         50000L
#define ERR_DEAD_X10     80      /* |err|<8rpm 不积分, 减测速噪声拧频 */

static BSP_FOC_State_t s;
static int8_t s_dir = 1;
static uint16_t s_speed_pm;
static int32_t s_rpm_ref_x10;
static int32_t s_rpm_ref_slew;

static uint32_t s_phase;
static uint32_t s_phase_inc;
static volatile uint32_t s_phase_inc_tgt;
static volatile uint16_t s_amp;
static volatile uint16_t s_amp_tgt;

static volatile uint8_t s_enc_ok;
static volatile int32_t s_enc_cum;

static int32_t s_pi_i;
static int32_t s_rpm_filt;
static int32_t s_fe_cmd;
static int32_t s_cum_prev;
static uint32_t s_ms_prev;
static uint8_t s_rpm_ready;
static uint8_t s_slip_cnt;

extern volatile uint32_t g_millis;

static const int16_t s_sin[SIN_N] = {
    0, 804, 1608, 2410, 3212, 4011, 4808, 5602, 6393, 7179, 7962, 8739, 9512, 10278, 11039, 11793,
    12539, 13279, 14010, 14732, 15446, 16151, 16846, 17530, 18204, 18868, 19519, 20159, 20787, 21403, 22005, 22594,
    23170, 23731, 24279, 24811, 25329, 25832, 26319, 26790, 27245, 27683, 28105, 28510, 28898, 29268, 29621, 29956,
    30273, 30571, 30852, 31113, 31356, 31580, 31785, 31971, 32137, 32285, 32412, 32521, 32609, 32678, 32728, 32757,
    32767, 32757, 32728, 32678, 32609, 32521, 32412, 32285, 32137, 31971, 31785, 31580, 31356, 31113, 30852, 30571,
    30273, 29956, 29621, 29268, 28898, 28510, 28105, 27683, 27245, 26790, 26319, 25832, 25329, 24811, 24279, 23731,
    23170, 22594, 22005, 21403, 20787, 20159, 19519, 18868, 18204, 17530, 16846, 16151, 15446, 14732, 14010, 13279,
    12539, 11793, 11039, 10278, 9512, 8739, 7962, 7179, 6393, 5602, 4808, 4011, 3212, 2410, 1608, 804,
    0, -804, -1608, -2410, -3212, -4011, -4808, -5602, -6393, -7179, -7962, -8739, -9512, -10278, -11039, -11793,
    -12539, -13279, -14010, -14732, -15446, -16151, -16846, -17530, -18204, -18868, -19519, -20159, -20787, -21403, -22005, -22594,
    -23170, -23731, -24279, -24811, -25329, -25832, -26319, -26790, -27245, -27683, -28105, -28510, -28898, -29268, -29621, -29956,
    -30273, -30571, -30852, -31113, -31356, -31580, -31785, -31971, -32137, -32285, -32412, -32521, -32609, -32678, -32728, -32757,
    -32767, -32757, -32728, -32678, -32609, -32521, -32412, -32285, -32137, -31971, -31785, -31580, -31356, -31113, -30852, -30571,
    -30273, -29956, -29621, -29268, -28898, -28510, -28105, -27683, -27245, -26790, -26319, -25832, -25329, -24811, -24279, -23731,
    -23170, -22594, -22005, -21403, -20787, -20159, -19519, -18868, -18204, -17530, -16846, -16151, -15446, -14732, -14010, -13279,
    -12539, -11793, -11039, -10278, -9512, -8739, -7962, -7179, -6393, -5602, -4808, -4011, -3212, -2410, -1608, -804
};

static void btn_1ms_init(void)
{
    BTIM_TimeBaseInitTypeDef tb = {0};
    __SYSCTRL_BTIM123_CLK_ENABLE();
    tb.BTIM_Mode = BTIM_MODE_TIMER;
    tb.BTIM_CountMode = BTIM_COUNT_MODE_REPETITIVE;
    tb.BTIM_Prescaler = 95U;
    tb.BTIM_Period = 999U;
    BTIM_TimeBaseInit(CW_BTIM1, &tb);
    BTIM_ITConfig(CW_BTIM1, BTIM_IT_UPDATE, ENABLE);
    BTIM_Cmd(CW_BTIM1, ENABLE);
    NVIC_SetPriority(BTIM1_IRQn, 2U);
    NVIC_EnableIRQ(BTIM1_IRQn);
}

static uint32_t fe_to_inc(uint32_t fe_x10)
{
    uint32_t inc = (fe_x10 * 1048576UL) / 9375UL;
    return (inc < 1UL) ? 1UL : inc;
}

static uint16_t fe_to_amp(uint32_t fe_x10)
{
    /* 分段 V/f: 中低速柔和, 高速再过调制
     * fe≈117(100rpm)→~1180, fe≈350(300rpm)→~1550, fe≥1000→满 */
    if (fe_x10 <= FE_MIN_X10)
    {
        return AMP_MIN;
    }
    if (fe_x10 < 120U)
    {
        return (uint16_t)(AMP_MIN + ((1200U - AMP_MIN) * (fe_x10 - FE_MIN_X10)) /
                           (120U - FE_MIN_X10));
    }
    if (fe_x10 < 400U)
    {
        return (uint16_t)(1200U + ((1650U - 1200U) * (fe_x10 - 120U)) / (400U - 120U));
    }
    if (fe_x10 < 1000U)
    {
        return (uint16_t)(1650U + ((AMP_MAX - 1650U) * (fe_x10 - 400U)) / (1000U - 400U));
    }
    return AMP_MAX;
}

static void apply_spwm(uint8_t ia, uint16_t amp)
{
    const int32_t mid = (int32_t)(BSP_MOTOR_PWM_ARR / 2U);
    const int32_t lim_lo = 8;
    const int32_t lim_hi = (int32_t)BSP_MOTOR_PWM_ARR - 8;
    uint8_t ib = (uint8_t)((ia + 85U) & 0xFFU);
    uint8_t ic = (uint8_t)((ia + 171U) & 0xFFU);
    int32_t va = ((int32_t)s_sin[ia] * (int32_t)amp) / 32767;
    int32_t vb = ((int32_t)s_sin[ib] * (int32_t)amp) / 32767;
    int32_t vc = ((int32_t)s_sin[ic] * (int32_t)amp) / 32767;
    int32_t vmax = va, vmin = va, vcom, a, b, c;

    if (vb > vmax) { vmax = vb; }
    if (vc > vmax) { vmax = vc; }
    if (vb < vmin) { vmin = vb; }
    if (vc < vmin) { vmin = vc; }
    vcom = (vmax + vmin) / 2;
    a = mid + va - vcom;
    b = mid + vb - vcom;
    c = mid + vc - vcom;
    if (a < lim_lo) { a = lim_lo; }
    if (b < lim_lo) { b = lim_lo; }
    if (c < lim_lo) { c = lim_lo; }
    if (a > lim_hi) { a = lim_hi; }
    if (b > lim_hi) { b = lim_hi; }
    if (c > lim_hi) { c = lim_hi; }
    BSP_MOTOR_SetPhaseDuty((uint16_t)a, (uint16_t)b, (uint16_t)c);
}

static int32_t iabs32(int32_t x) { return (x < 0) ? -x : x; }

static int32_t rpm_to_fe_x10(int32_t rpm_x10)
{
    int32_t fe = (rpm_x10 * (int32_t)MOTOR_POLE_PAIRS) / 60;
    if (fe < (int32_t)FE_MIN_X10) { fe = (int32_t)FE_MIN_X10; }
    if (fe > (int32_t)FE_MAX_X10) { fe = (int32_t)FE_MAX_X10; }
    return fe;
}

void BSP_FOC_Init(void)
{
    s.mode = 0U; s.running = 0U; s.isr_cnt = 0U;
    s_enc_ok = 0U; s_pi_i = 0; s_rpm_ready = 0U;
    BSP_MOTOR_Init();
    btn_1ms_init();
}

void BSP_FOC_Start(void)
{
    s_phase = 0U;
    s_phase_inc = fe_to_inc(FE_MIN_X10);
    s_phase_inc_tgt = s_phase_inc;
    s_amp = AMP_MIN;
    s_amp_tgt = AMP_MIN;
    s_pi_i = 0;
    s_rpm_filt = 0;
    s_fe_cmd = (int32_t)FE_MIN_X10;
    s_rpm_ref_slew = RPM_MIN_X10;
    s_rpm_ready = 0U;
    s_slip_cnt = 0U;
    s.mode = 2U;
    s.running = 1U;
    s.isr_cnt = 0U;
    BSP_MOTOR_Start();
    BSP_MOTOR_EnablePwmIrq();
}

void BSP_FOC_Stop(void)
{
    s.running = 0U; s.mode = 0U;
    BSP_MOTOR_DisablePwmIrq();
    BSP_MOTOR_Stop();
}

void BSP_FOC_SetSpeed(uint16_t speed_permille)
{
    uint32_t sp = speed_permille;
    if (sp > 1000U) { sp = 1000U; }
    s_speed_pm = (uint16_t)sp;
    s_rpm_ref_x10 = RPM_MIN_X10 +
        ((int32_t)(RPM_MAX_X10 - RPM_MIN_X10) * (int32_t)sp) / 1000;
}

void BSP_FOC_SetDirection(int8_t dir) { s_dir = (dir >= 0) ? 1 : -1; }
void BSP_FOC_ToggleDirection(void) { s_dir = (int8_t)(-s_dir); }

void BSP_FOC_OnEncoder(uint16_t raw, uint8_t ok)
{
    (void)raw;
    s_enc_ok = ok;
}
void BSP_FOC_OnEncoderRpm(int32_t rpm_x10) { (void)rpm_x10; }
void BSP_FOC_OnEncoderCum(int32_t cum_raw) { s_enc_cum = cum_raw; }

void BSP_FOC_SpeedLoop(void)
{
    int32_t rpm_meas, rpm_ref, err, trim, fe_ff, fe_tgt;
    uint32_t now;

    if (s.running == 0U) { return; }

    {
        int32_t up = 40, dn = 100;
        if (s_rpm_ref_slew > 10000) { up = 14; }
        else if (s_rpm_ref_slew > 6000) { up = 24; }
        else if (s_rpm_ref_slew < 2000) { up = 20; dn = 40; }
        if (s_rpm_ref_slew < s_rpm_ref_x10) {
            s_rpm_ref_slew += up;
            if (s_rpm_ref_slew > s_rpm_ref_x10) { s_rpm_ref_slew = s_rpm_ref_x10; }
        } else if (s_rpm_ref_slew > s_rpm_ref_x10) {
            s_rpm_ref_slew -= dn;
            if (s_rpm_ref_slew < s_rpm_ref_x10) { s_rpm_ref_slew = s_rpm_ref_x10; }
        }
    }
    rpm_ref = s_rpm_ref_slew;

    now = g_millis;
    if (s_rpm_ready == 0U) {
        s_cum_prev = s_enc_cum; s_ms_prev = now; s_rpm_ready = 1U; rpm_meas = 0;
    } else {
        uint32_t dt = now - s_ms_prev;
        uint32_t need = (s_rpm_filt < 4000) ? 100U : 40U;
        if (dt >= need) {
            int32_t dc = s_enc_cum - s_cum_prev;
            int32_t inst = (dc * 600000L) / ((int32_t)dt * 4096L);
            if (inst > 25000L) { inst = 25000L; }
            if (inst < -25000L) { inst = -25000L; }
            /* 低速更重滤波, 减轻 PI 跟着噪声抖 */
            if (iabs32(inst) < 4000) {
                s_rpm_filt = (s_rpm_filt * 5 + iabs32(inst)) / 6;
            } else {
                s_rpm_filt = (s_rpm_filt * 2 + iabs32(inst)) / 3;
            }
            s_cum_prev = s_enc_cum; s_ms_prev = now;
        }
        rpm_meas = s_rpm_filt;
    }

    fe_ff = rpm_to_fe_x10(rpm_ref);

    if (s_enc_ok == 0U) {
        s_pi_i = 0; fe_tgt = fe_ff; s.mode = 1U;
    } else if (rpm_meas < 200) {
        s_pi_i = 0; s.mode = 1U;
        if (s_fe_cmd > 400 && rpm_meas < 80) {
            s_fe_cmd = (int32_t)FE_MIN_X10 + 20;
            s_rpm_ref_slew = RPM_MIN_X10;
            fe_tgt = s_fe_cmd; s.mode = 4U;
        } else {
            fe_tgt = s_fe_cmd + 3;
            if (fe_tgt > fe_ff) { fe_tgt = fe_ff; }
            if (fe_tgt < (int32_t)FE_MIN_X10) { fe_tgt = (int32_t)FE_MIN_X10; }
        }
    } else {
        int32_t trim_lim = (rpm_ref < 4000) ? TRIM_MAX_LS : TRIM_MAX;
        err = rpm_ref - rpm_meas;
        /* 死区: 小误差不累计, 前馈为主 */
        if (iabs32(err) < ERR_DEAD_X10) {
            err = 0;
            s_pi_i = (s_pi_i * 7) / 8;
        } else {
            s_pi_i += err;
        }
        if (s_pi_i > SP_I_LIM) { s_pi_i = SP_I_LIM; }
        if (s_pi_i < -SP_I_LIM) { s_pi_i = -SP_I_LIM; }
        trim = (err * SP_KP_NUM) / SP_KP_DEN + (s_pi_i * SP_KI_NUM) / SP_KI_DEN;
        if (trim > trim_lim) { trim = trim_lim; }
        if (trim < -trim_lim) { trim = -trim_lim; }
        fe_tgt = fe_ff + trim;
        s.mode = 2U;
        {
            int32_t fe_meas = rpm_to_fe_x10(rpm_meas);
            int32_t lead;
            if (rpm_meas < 4000) { lead = MAX_LEAD_LS_X10; }
            else if (rpm_meas > 9000) { lead = MAX_LEAD_HS_X10; }
            else { lead = MAX_LEAD_X10; }
            int32_t fe_cap = fe_meas + lead;
            if (fe_tgt > fe_cap) {
                fe_tgt = fe_cap;
                if (err > 0) {
                    s_pi_i -= err;
                    if ((rpm_meas < ((rpm_ref * 88) / 100)) &&
                        (s_rpm_ref_slew > (rpm_meas + 1000))) {
                        s_rpm_ref_slew = rpm_meas + 1000;
                    }
                }
            }
        }
        if ((rpm_ref > 2500) && (rpm_meas < (rpm_ref * 70) / 100)) {
            if (s_slip_cnt < 50U) { s_slip_cnt++; }
            if (s_slip_cnt >= 10U) {
                fe_tgt = rpm_to_fe_x10(rpm_meas) + 60;
                s_fe_cmd = fe_tgt; s_pi_i /= 4;
                if (s_rpm_ref_slew > (rpm_meas + 600)) {
                    s_rpm_ref_slew = rpm_meas + 600;
                }
                s.mode = 4U;
            }
        } else {
            s_slip_cnt = 0U;
        }
    }

    if (fe_tgt < (int32_t)FE_MIN_X10) { fe_tgt = (int32_t)FE_MIN_X10; }
    if (fe_tgt > (int32_t)FE_MAX_X10) { fe_tgt = (int32_t)FE_MAX_X10; }

    {
        int32_t up = 2, dn = 8;
        if (s_fe_cmd > 1200) { up = 2; }
        else if (s_fe_cmd < 400) { up = 1; dn = 4; } /* 中低速 fe 更平滑 */
        if (s_fe_cmd < fe_tgt) {
            s_fe_cmd += up;
            if (s_fe_cmd > fe_tgt) { s_fe_cmd = fe_tgt; }
        } else if (s_fe_cmd > fe_tgt) {
            s_fe_cmd -= dn;
            if (s_fe_cmd < fe_tgt) { s_fe_cmd = fe_tgt; }
        }
    }

    s_phase_inc_tgt = fe_to_inc((uint32_t)s_fe_cmd);
    s_amp_tgt = fe_to_amp((uint32_t)s_fe_cmd);
    s.rpm = (float)rpm_meas * 0.1f;
    s.iq_a = (float)rpm_ref * 0.1f;
    s.id_a = (float)s_fe_cmd;
    s.omega_e = (float)s_speed_pm;
}

void BSP_FOC_PwmIrq(void)
{
    uint8_t idx; uint16_t amp; uint32_t inc_tgt;

    if (ATIM_GetITStatus(ATIM_STATE_UIF) == RESET) { return; }
    ATIM_ClearITPendingBit(ATIM_STATE_UIF);
    s.isr_cnt++;

    /* 启停只走 main 单击回调, 避免按下时 IRQ 先停、松开又被单击重新启动 */
    if (s.running == 0U) { return; }

    amp = s_amp;
    if (amp < s_amp_tgt) {
        amp = (uint16_t)(amp + 2U);
        if (amp > s_amp_tgt) { amp = s_amp_tgt; }
        s_amp = amp;
    } else if (amp > s_amp_tgt) {
        amp--;
        s_amp = amp;
    }

    inc_tgt = s_phase_inc_tgt;
    if (s_phase_inc < inc_tgt) {
        uint32_t d = inc_tgt - s_phase_inc;
        s_phase_inc += (d > INC_SLEW_UP) ? INC_SLEW_UP : d;
    } else if (s_phase_inc > inc_tgt) {
        uint32_t d = s_phase_inc - inc_tgt;
        s_phase_inc -= (d > INC_SLEW_DN) ? INC_SLEW_DN : d;
    }

    if (s_dir >= 0) { s_phase += s_phase_inc; }
    else { s_phase -= s_phase_inc; }

    idx = (uint8_t)(s_phase >> 16);
    apply_spwm(idx, amp);
    if ((s.isr_cnt & 127U) == 0U) {
        s.theta = (float)idx * (6.2831853f / 256.0f);
    }
}

const BSP_FOC_State_t *BSP_FOC_GetState(void) { return &s; }
