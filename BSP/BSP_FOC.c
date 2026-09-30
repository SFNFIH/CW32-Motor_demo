/**
 * @file    BSP_FOC.c
 * @brief   速度闭环 V/f + 角度闭环 (电位器 0..4096 → 0..360°, 绝对多圈可回位)
 *          ToggleDirection / SetDirection API 保留; 模式由 ToggleCtrlMode 切换
 */
#include "BSP_FOC.h"
#include "BSP_MOTOR.h"
#include "BSP_Current.h"
#include "BSP_motor_params.h"
#include "cw32l012_atim.h"
#include "cw32l012_btim.h"
#include "cw32l012_sysctrl.h"

#define SIN_N            256U

#define RPM_MIN_X10      150
#define RPM_MAX_X10      13500

#define FE_MIN_X10       20U
#define FE_MAX_X10       1800U

#define AMP_MIN          950U
#define AMP_MAX          3200U

#define INC_SLEW_UP      3UL
#define INC_SLEW_DN      16UL

#define MAX_LEAD_X10     120
#define MAX_LEAD_LS_X10  50
#define MAX_LEAD_HS_X10  140
#define TRIM_MAX         100
#define TRIM_MAX_LS      40

#define SP_KP_NUM        1
#define SP_KP_DEN        60
#define SP_KI_NUM        1
#define SP_KI_DEN        400
#define SP_I_LIM         50000L
#define ERR_DEAD_X10     80

#define ANG_DEAD_RAW     6       /* 过宽会提前停住: 如目标4095停在4070 */
#define ANG_KP_NUM       5       /* 位置→转速增益 (越大跟得越快) */
#define ANG_KP_DEN       1
#define ANG_RPM_MAX_X10  8000    /* 角度环最大追赶转速 800 rpm */
#define ANG_HOLD_AMP     1100U
#define ANG_REF_SLEW     160     /* 电位器目标缓变步进 (raw/环) */

#define I_LIM_MIN_MA     120
#define I_LIM_MAX_MA     1500

static BSP_FOC_State_t s;
static int8_t s_dir = 1;
static volatile int8_t s_spin_dir = 1; /* IRQ 相位方向: 速度模式跟 s_dir, 角度模式跟误差 */
static uint8_t s_ctrl_mode;   /* SPEED / ANGLE / CURRENT (UI) */
static uint8_t s_motion_mode; /* 外环: SPEED 或 ANGLE (电流设定时保持) */
static uint16_t s_speed_pm;
static int32_t s_rpm_ref_x10;
static int32_t s_rpm_ref_slew;
static uint16_t s_angle_ref_raw;
static int32_t s_ang_base;       /* 绝对目标 = base + pot_raw (进角度模式时锁定) */
static int32_t s_angle_ref_slew; /* 绝对目标缓变 (raw 计数, 可超 ±4096) */
static uint16_t s_imax_pm;
static uint16_t s_imax_slew;
static int32_t s_imax_ma;
static int32_t s_i_filt_ma;

static uint32_t s_phase;
static uint32_t s_phase_inc;
static volatile uint32_t s_phase_inc_tgt;
static volatile uint16_t s_amp;
static volatile uint16_t s_amp_tgt;

static volatile uint8_t s_enc_ok;
static volatile uint16_t s_enc_raw;
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
    if (fe_x10 <= FE_MIN_X10) { return AMP_MIN; }
    if (fe_x10 < 120U) {
        return (uint16_t)(AMP_MIN + ((1200U - AMP_MIN) * (fe_x10 - FE_MIN_X10)) /
                           (120U - FE_MIN_X10));
    }
    if (fe_x10 < 400U) {
        return (uint16_t)(1200U + ((1650U - 1200U) * (fe_x10 - 120U)) / (400U - 120U));
    }
    if (fe_x10 < 1000U) {
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

static uint8_t motion_is_angle(void)
{
    return (s_motion_mode == BSP_FOC_CTRL_ANGLE) ? 1U : 0U;
}

static void imax_from_pm(uint16_t pm)
{
    if (pm > 1000U) { pm = 1000U; }
    s_imax_pm = pm;
    s_imax_ma = I_LIM_MIN_MA +
        ((I_LIM_MAX_MA - I_LIM_MIN_MA) * (int32_t)pm) / 1000;
}

static void sample_current(void)
{
    float ia;
    float ib;
    int32_t ima;
    int32_t imb;
    int32_t imeas;

    if (BSP_Current_Read(&ia, &ib) != 0U) {
        ima = (int32_t)(ia * 1000.0f);
        imb = (int32_t)(ib * 1000.0f);
        if (ima < 0) { ima = -ima; }
        if (imb < 0) { imb = -imb; }
        imeas = ima + imb;
        s_i_filt_ma = (s_i_filt_ma * 3 + imeas) / 4;
    }
    s.i_meas_a = (float)s_i_filt_ma * 0.001f;
    s.i_lim_a = (float)s_imax_ma * 0.001f;
}

void BSP_FOC_PollCurrent(void)
{
    sample_current();
}

static void apply_current_loop(void)
{
    uint32_t pm;
    uint32_t amp_lim;
    uint16_t amp_floor;

    /* Imax 缓变, 避免电流设定时电位器噪声把电压拧抖 */
    if (s_imax_slew + 40U < s_imax_pm) {
        s_imax_slew = (uint16_t)(s_imax_slew + 40U);
    } else if (s_imax_slew > s_imax_pm + 40U) {
        s_imax_slew = (uint16_t)(s_imax_slew - 40U);
    } else {
        s_imax_slew = s_imax_pm;
    }

    sample_current();

    /* 到位停频: 绝不能再注入 FE_MIN, 否则角环/保持会发抖 */
    if ((s_fe_cmd <= 0) && (s_phase_inc_tgt == 0U)) {
        s_amp_tgt = ANG_HOLD_AMP;
        return;
    }

    /* 只缩电压(力矩), 不改频率, 避免 V/f 失配失步抖动 */
    pm = s_imax_slew;
    if (pm < 120U) { pm = 120U; }
    amp_lim = ((uint32_t)s_amp_tgt * pm) / 1000U;
    amp_floor = motion_is_angle() ? 900U : 400U;
    if (amp_lim < (uint32_t)amp_floor) { amp_lim = amp_floor; }
    s_amp_tgt = (uint16_t)amp_lim;
}

static int32_t rpm_to_fe_x10(int32_t rpm_x10)
{
    int32_t fe = (rpm_x10 * (int32_t)MOTOR_POLE_PAIRS) / 60;
    if (fe < (int32_t)FE_MIN_X10) { fe = (int32_t)FE_MIN_X10; }
    if (fe > (int32_t)FE_MAX_X10) { fe = (int32_t)FE_MAX_X10; }
    return fe;
}

/* 将 x 四舍五入到最近的 4096 倍数 (一圈) */
static int32_t round_mul_4096(int32_t x)
{
    if (x >= 0) {
        return ((x + 2048) / 4096) * 4096;
    }
    return -(((-x + 2048) / 4096) * 4096);
}

/* pot=0 → 该圈绝对 0; pot=4095 → 近 360°; 超圈后仍按 cum 回位 */
static void angle_lock_abs(void)
{
    s_ang_base = round_mul_4096(s_enc_cum - (int32_t)s_angle_ref_raw);
    s_angle_ref_slew = s_enc_cum;
}

static void slew_fe_amp(int32_t fe_tgt)
{
    int32_t up = 2, dn = 8;
    if (motion_is_angle()) {
        /* 角度环加快升频, 缩短到位时间 */
        up = 8;
        dn = 12;
    } else if (s_fe_cmd > 1200) {
        up = 2;
    } else if (s_fe_cmd < 400) {
        up = 1;
        dn = 4;
    }
    if (s_fe_cmd < fe_tgt) {
        s_fe_cmd += up;
        if (s_fe_cmd > fe_tgt) { s_fe_cmd = fe_tgt; }
    } else if (s_fe_cmd > fe_tgt) {
        s_fe_cmd -= dn;
        if (s_fe_cmd < fe_tgt) { s_fe_cmd = fe_tgt; }
    }
    s_phase_inc_tgt = fe_to_inc((uint32_t)s_fe_cmd);
    s_amp_tgt = fe_to_amp((uint32_t)s_fe_cmd);
    apply_current_loop();
}

static void update_rpm_meas(void)
{
    uint32_t now = g_millis;
    if (s_rpm_ready == 0U) {
        s_cum_prev = s_enc_cum;
        s_ms_prev = now;
        s_rpm_ready = 1U;
        s_rpm_filt = 0;
        return;
    }
    {
        uint32_t dt = now - s_ms_prev;
        uint32_t need = (s_rpm_filt < 4000) ? 100U : 40U;
        if (dt >= need) {
            int32_t dc = s_enc_cum - s_cum_prev;
            int32_t inst = (dc * 600000L) / ((int32_t)dt * 4096L);
            if (inst > 25000L) { inst = 25000L; }
            if (inst < -25000L) { inst = -25000L; }
            if (iabs32(inst) < 4000) {
                s_rpm_filt = (s_rpm_filt * 5 + iabs32(inst)) / 6;
            } else {
                s_rpm_filt = (s_rpm_filt * 2 + iabs32(inst)) / 3;
            }
            s_cum_prev = s_enc_cum;
            s_ms_prev = now;
        }
    }
}

void BSP_FOC_Init(void)
{
    s.mode = 0U;
    s.running = 0U;
    s.isr_cnt = 0U;
    s_enc_ok = 0U;
    s_pi_i = 0;
    s_rpm_ready = 0U;
    s_ctrl_mode = BSP_FOC_CTRL_SPEED;
    s_motion_mode = BSP_FOC_CTRL_SPEED;
    s_angle_ref_raw = 0U;
    s_ang_base = 0;
    s_angle_ref_slew = 0;
    s_spin_dir = 1;
    imax_from_pm(1000U);
    s_imax_slew = 1000U;
    s_i_filt_ma = 0;
    s.i_meas_a = 0.0f;
    s.i_lim_a = (float)I_LIM_MAX_MA * 0.001f;
    BSP_MOTOR_Init();
    BSP_Current_Init();
    BSP_Current_Calibrate();
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
    s_spin_dir = s_dir;
    if ((s_enc_ok != 0U) && motion_is_angle()) {
        angle_lock_abs();
    }
    s.mode = motion_is_angle() ? 3U : 2U;
    s.running = 1U;
    s.isr_cnt = 0U;
    BSP_MOTOR_Start();
    BSP_MOTOR_EnablePwmIrq();
}

void BSP_FOC_Stop(void)
{
    s.running = 0U;
    s.mode = 0U;
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

void BSP_FOC_SetAngleRaw(uint16_t raw_0_4095)
{
    if (raw_0_4095 > 4095U) { raw_0_4095 = 4095U; }
    s_angle_ref_raw = raw_0_4095;
}

void BSP_FOC_SetImaxPm(uint16_t permille)
{
    imax_from_pm(permille);
    s.i_lim_a = (float)s_imax_ma * 0.001f;
}

uint16_t BSP_FOC_GetImaxPm(void)
{
    return s_imax_pm;
}

void BSP_FOC_SetCtrlMode(uint8_t mode)
{
    if (mode == BSP_FOC_CTRL_CURRENT) {
        s_ctrl_mode = BSP_FOC_CTRL_CURRENT;
    } else if (mode == BSP_FOC_CTRL_ANGLE) {
        s_ctrl_mode = BSP_FOC_CTRL_ANGLE;
        s_motion_mode = BSP_FOC_CTRL_ANGLE;
        s_pi_i = 0;
        s_slip_cnt = 0U;
        if ((s.running != 0U) && (s_enc_ok != 0U)) {
            angle_lock_abs();
        }
    } else {
        s_ctrl_mode = BSP_FOC_CTRL_SPEED;
        s_motion_mode = BSP_FOC_CTRL_SPEED;
        s_pi_i = 0;
        s_slip_cnt = 0U;
    }
    if (s.running != 0U) {
        s.mode = motion_is_angle() ? 3U : 2U;
    }
}

uint8_t BSP_FOC_GetCtrlMode(void)
{
    return s_ctrl_mode;
}

uint8_t BSP_FOC_GetMotionMode(void)
{
    return s_motion_mode;
}

void BSP_FOC_ToggleCtrlMode(void)
{
    if (s_ctrl_mode == BSP_FOC_CTRL_SPEED) {
        BSP_FOC_SetCtrlMode(BSP_FOC_CTRL_ANGLE);
    } else if (s_ctrl_mode == BSP_FOC_CTRL_ANGLE) {
        BSP_FOC_SetCtrlMode(BSP_FOC_CTRL_CURRENT);
    } else {
        BSP_FOC_SetCtrlMode(BSP_FOC_CTRL_SPEED);
    }
}

void BSP_FOC_SetDirection(int8_t dir)
{
    s_dir = (dir >= 0) ? 1 : -1;
    if (s_motion_mode == BSP_FOC_CTRL_SPEED) {
        s_spin_dir = s_dir;
    }
}

void BSP_FOC_ToggleDirection(void)
{
    BSP_FOC_SetDirection((int8_t)(-s_dir));
}

uint16_t BSP_FOC_GetAmp(void)
{
    return s_amp;
}

void BSP_FOC_OnEncoder(uint16_t raw, uint8_t ok)
{
    s_enc_raw = raw;
    s_enc_ok = ok;
}

void BSP_FOC_OnEncoderRpm(int32_t rpm_x10) { (void)rpm_x10; }
void BSP_FOC_OnEncoderCum(int32_t cum_raw) { s_enc_cum = cum_raw; }

static void run_speed_vf(int32_t rpm_ref_abs, int32_t rpm_meas)
{
    int32_t fe_ff = rpm_to_fe_x10(rpm_ref_abs);
    int32_t fe_tgt;
    int32_t err;
    int32_t trim;

    if (s_enc_ok == 0U) {
        s_pi_i = 0;
        fe_tgt = fe_ff;
        s.mode = 1U;
    } else if (rpm_meas < 200) {
        s_pi_i = 0;
        s.mode = 1U;
        if ((s_fe_cmd > 400) && (rpm_meas < 80)) {
            s_fe_cmd = (int32_t)FE_MIN_X10 + 20;
            if (s_motion_mode == BSP_FOC_CTRL_SPEED) {
                s_rpm_ref_slew = RPM_MIN_X10;
            }
            fe_tgt = s_fe_cmd;
            s.mode = 4U;
        } else {
            int32_t step = motion_is_angle() ? 12 : 3;
            fe_tgt = s_fe_cmd + step;
            if (fe_tgt > fe_ff) { fe_tgt = fe_ff; }
            if (fe_tgt < (int32_t)FE_MIN_X10) { fe_tgt = (int32_t)FE_MIN_X10; }
        }
    } else {
        int32_t trim_lim = (rpm_ref_abs < 4000) ? TRIM_MAX_LS : TRIM_MAX;
        err = rpm_ref_abs - rpm_meas;
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
        s.mode = motion_is_angle() ? 3U : 2U;
        {
            int32_t fe_meas = rpm_to_fe_x10(rpm_meas);
            int32_t lead = MAX_LEAD_X10;
            int32_t fe_cap;
            if (rpm_meas < 4000) { lead = MAX_LEAD_LS_X10; }
            else if (rpm_meas > 9000) { lead = MAX_LEAD_HS_X10; }
            fe_cap = fe_meas + lead;
            if (fe_tgt > fe_cap) {
                fe_tgt = fe_cap;
                if (err > 0) { s_pi_i -= err; }
            }
        }
        if ((rpm_ref_abs > 2500) && (rpm_meas < (rpm_ref_abs * 70) / 100)) {
            if (s_slip_cnt < 50U) { s_slip_cnt++; }
            if (s_slip_cnt >= 10U) {
                fe_tgt = rpm_to_fe_x10(rpm_meas) + 60;
                s_fe_cmd = fe_tgt;
                s_pi_i /= 4;
                s.mode = 4U;
            }
        } else {
            s_slip_cnt = 0U;
        }
    }

    if (fe_tgt < (int32_t)FE_MIN_X10) { fe_tgt = (int32_t)FE_MIN_X10; }
    if (fe_tgt > (int32_t)FE_MAX_X10) { fe_tgt = (int32_t)FE_MAX_X10; }
    slew_fe_amp(fe_tgt);
}

static void loop_speed(void)
{
    int32_t rpm_meas;
    int32_t rpm_ref;

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
    update_rpm_meas();
    rpm_meas = s_rpm_filt;
    s_spin_dir = s_dir;
    run_speed_vf(rpm_ref, rpm_meas);

    s.rpm = (float)rpm_meas * 0.1f;
    s.iq_a = (float)rpm_ref * 0.1f;
    s.id_a = (float)s_fe_cmd;
    s.omega_e = (float)s_speed_pm;
}

static void loop_angle(void)
{
    int32_t err;
    int32_t rpm_cmd;
    int32_t rpm_meas;
    int32_t dref;
    int32_t ref_tgt;

    /* 绝对目标 = 进模式时锁定的圈基址 + 电位器 0..4095 */
    ref_tgt = s_ang_base + (int32_t)s_angle_ref_raw;
    dref = ref_tgt - s_angle_ref_slew;
    if (dref > ANG_REF_SLEW) { dref = ANG_REF_SLEW; }
    if (dref < -ANG_REF_SLEW) { dref = -ANG_REF_SLEW; }
    s_angle_ref_slew += dref;

    update_rpm_meas();
    rpm_meas = s_rpm_filt;

    if (s_enc_ok == 0U) {
        s_spin_dir = s_dir;
        run_speed_vf(RPM_MIN_X10, rpm_meas);
        s.mode = 1U;
        s.iq_a = (float)s_angle_ref_raw * (360.0f / 4096.0f);
        s.omega_e = 0.0f;
        s.id_a = (float)s_amp_tgt;
        s.rpm = (float)rpm_meas * 0.1f;
        return;
    }

    /* 绝对误差 (不取模): 转过 ±360° 仍能回到目标 */
    err = s_angle_ref_slew - s_enc_cum;

    if (iabs32(err) <= ANG_DEAD_RAW) {
        /* 到位: 停频, 小幅保持 */
        s_pi_i = 0;
        s_fe_cmd = 0;
        s_phase_inc_tgt = 0U;
        s_amp_tgt = ANG_HOLD_AMP;
        apply_current_loop();
        s_spin_dir = s_dir;
        s.mode = 3U;
    } else {
        rpm_cmd = (err * ANG_KP_NUM) / ANG_KP_DEN;
        if (rpm_cmd > ANG_RPM_MAX_X10) { rpm_cmd = ANG_RPM_MAX_X10; }
        if (rpm_cmd < -ANG_RPM_MAX_X10) { rpm_cmd = -ANG_RPM_MAX_X10; }
        /* 本板: 相位正转时 AS5600 cum 递减, 故误差符号取反驱动 */
        if (rpm_cmd > 0) {
            s_spin_dir = -1;
            run_speed_vf(rpm_cmd, rpm_meas);
        } else {
            s_spin_dir = 1;
            run_speed_vf(-rpm_cmd, rpm_meas);
        }
        s.mode = 3U;
    }

    s.rpm = (float)rpm_meas * 0.1f;
    /* 相对当前圈: pot0→0°, pot4095→~360°; 超圈时 meas 可超出 */
    s.iq_a = (float)(s_angle_ref_slew - s_ang_base) * (360.0f / 4096.0f);
    s.omega_e = (float)(s_enc_cum - s_ang_base) * (360.0f / 4096.0f);
    s.id_a = (float)s_amp_tgt;
    s.theta = (float)(s_enc_cum - s_ang_base) * (6.2831853f / 4096.0f);
}

void BSP_FOC_SpeedLoop(void)
{
    if (s.running == 0U) { return; }
    if (motion_is_angle()) {
        loop_angle();
    } else {
        loop_speed();
    }
    if (s_ctrl_mode == BSP_FOC_CTRL_CURRENT) {
        s.id_a = s.i_meas_a;
        s.iq_a = s.i_lim_a;
    }
}

void BSP_FOC_PwmIrq(void)
{
    uint8_t idx;
    uint16_t amp;
    uint32_t inc_tgt;

    if (ATIM_GetITStatus(ATIM_STATE_UIF) == RESET) { return; }
    ATIM_ClearITPendingBit(ATIM_STATE_UIF);
    s.isr_cnt++;

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
        uint32_t step = motion_is_angle() ? 12UL : INC_SLEW_UP;
        s_phase_inc += (d > step) ? step : d;
    } else if (s_phase_inc > inc_tgt) {
        uint32_t d = s_phase_inc - inc_tgt;
        uint32_t step = motion_is_angle() ? 24UL : INC_SLEW_DN;
        s_phase_inc -= (d > step) ? step : d;
    }

    if (s_spin_dir >= 0) { s_phase += s_phase_inc; }
    else { s_phase -= s_phase_inc; }

    idx = (uint8_t)(s_phase >> 16);
    apply_spwm(idx, amp);
    if ((!motion_is_angle()) && ((s.isr_cnt & 127U) == 0U)) {
        s.theta = (float)idx * (6.2831853f / 256.0f);
    }
}

const BSP_FOC_State_t *BSP_FOC_GetState(void) { return &s; }
