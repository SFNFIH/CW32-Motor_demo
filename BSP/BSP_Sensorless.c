/**
 * @file    BSP_Sensorless.c
 * @brief   开环强拖 + 反电势过零闭环 (移植自参考 SENSORLESS-SIXSTEP OPENLOOP OK)
 */
#include "BSP_Sensorless.h"
#include "BSP_MOTOR.h"
#include "BSP_BEMF.h"
#include "cw32l012_btim.h"
#include "cw32l012_sysctrl.h"

#define RISING   1U
#define FALLING  2U

#define ST_ZC_NEED       15U   /* 连续过零认为启动成功 */
#define OL_STEP_MS       15U   /* 开环每步等待, 同参考 */
#define DEMAG_DIV        16U   /* StepTime>>4 退磁 */
#define ZC_DELAY_DIV     8U    /* StepTime>>3 过零后延时 */
#define ZC_FAST_THRESH   2000U /* StepTime 小于此则立即换相 */
#define ZC_CONFIRM       2U    /* 连续采样确认过零 */

/* Dir=0: 参考第一行; Dir=1: 第二行 */
static const uint8_t s_rf[2][6] = {
    { RISING, FALLING, RISING, FALLING, RISING, FALLING },
    { FALLING, RISING, FALLING, RISING, FALLING, RISING }
};

/* 浮空相 → g_bemf_sample 下标: C,B,A,C,B,A */
static const uint8_t s_bemf_ch[6] = { 2U, 1U, 0U, 2U, 1U, 0U };

static volatile SensorlessState_t s_state = SLS_ST_IDLE;
static volatile uint8_t  s_step = 0U;
static volatile uint8_t  s_dir  = 0U; /* 0/1 与参考 Dir 一致 */
static volatile uint16_t s_duty = BSP_MOTOR_DUTY_DEFAULT;
static volatile uint16_t s_target = BSP_MOTOR_DUTY_DEFAULT;
static volatile uint16_t s_slew_ms = 0U;
static volatile uint16_t s_bemf = 0U;
static volatile uint16_t s_mid  = 200U;
static volatile uint16_t s_zc_cnt = 0U;
static volatile uint16_t s_miss_cnt = 0U;
static volatile uint16_t s_period = 0U; /* StepTime 快照 */
static volatile uint8_t  s_st_ok = 0U;

/* Sta: 0=idle/open, 1=退磁中, 2=等过零, 3=过零后待换相 */
static volatile uint8_t  s_sta = 0U;
static volatile uint8_t  s_miss_run = 0U;
static volatile uint8_t  s_fflag = 0U;
static volatile uint8_t  s_run_ms = 0U;
static volatile uint8_t  s_run_per = 8U;
static volatile uint8_t  s_zc_armed = 0U;
static volatile uint16_t s_vpeak = 80U;
static volatile uint16_t s_st_count = 0U;

/* 开环状态机 */
static volatile uint16_t s_ol_wait = 0U;
static volatile uint16_t s_ol_steps = 0U;
static volatile uint8_t  s_align_left = 0U;

static void Timers_Init(void)
{
    BTIM_TimeBaseInitTypeDef tb = {0};

    __SYSCTRL_BTIM123_CLK_ENABLE();

    /* BTIM1: 1 ms @ 96 MHz → PSC=96-1, ARR=1000-1 (同参考) */
    tb.BTIM_Mode      = BTIM_MODE_TIMER;
    tb.BTIM_CountMode = BTIM_COUNT_MODE_REPETITIVE;
    tb.BTIM_Prescaler = 95U;
    tb.BTIM_Period    = 999U;
    BTIM_TimeBaseInit(CW_BTIM1, &tb);
    BTIM_ITConfig(CW_BTIM1, BTIM_IT_UPDATE, ENABLE);
    BTIM_Cmd(CW_BTIM1, ENABLE);
    NVIC_SetPriority(BTIM1_IRQn, 2U);
    NVIC_EnableIRQ(BTIM1_IRQn);

    /* BTIM2/3: 96MHz/12 = 8 MHz 计步 (同参考 PSC=12-1) */
    tb.BTIM_Prescaler = 11U;
    tb.BTIM_Period    = 65530U;
    BTIM_TimeBaseInit(CW_BTIM2, &tb);
    BTIM_Cmd(CW_BTIM2, ENABLE);

    /* BTIM3: 退磁 / 过零延时 */
    BTIM_TimeBaseInit(CW_BTIM3, &tb);
    BTIM_ITConfig(CW_BTIM3, BTIM_IT_UPDATE, ENABLE);
    BTIM_Cmd(CW_BTIM3, DISABLE);
    NVIC_SetPriority(BTIM3_HALLTIM_IRQn, 0U);
    NVIC_EnableIRQ(BTIM3_HALLTIM_IRQn);
}

/* 与参考 Dir 一致: Dir==0 减步, Dir==1 加步 */
static void StepNext(void)
{
    if (s_dir == 0U)
    {
        if (s_step == 0U)
        {
            s_step = 5U;
        }
        else
        {
            s_step--;
        }
    }
    else
    {
        s_step++;
        if (s_step >= 6U)
        {
            s_step = 0U;
        }
    }
}

static void ApplyDuty(uint16_t duty)
{
    if (duty >= BSP_MOTOR_PWM_ARR)
    {
        duty = (uint16_t)(BSP_MOTOR_PWM_ARR - 1U);
    }
    s_duty = duty;
    BSP_MOTOR_SetDuty(duty);
}

static void DoCommutate(void)
{
    uint16_t st;

    BSP_MOTOR_CommutateStep(s_step);

    st = (uint16_t)BTIM_GetCounter(CW_BTIM2);
    BTIM_SetCounter(CW_BTIM2, 0U);
    s_period = st;

    /* 启动退磁窗 */
    if (st < DEMAG_DIV)
    {
        st = DEMAG_DIV;
    }
    BTIM_SetAutoreload(CW_BTIM3, (uint16_t)(st / DEMAG_DIV));
    BTIM_SetCounter(CW_BTIM3, 0U);
    BTIM_Cmd(CW_BTIM3, ENABLE);
    s_sta = 1U;
    s_zc_armed = 0U;
}

void BSP_Sensorless_Init(void)
{
    s_state = SLS_ST_IDLE;
    s_step = 0U;
    s_dir = 0U;
    s_duty = BSP_MOTOR_DUTY_DEFAULT;
    s_st_ok = 0U;
    s_sta = 0U;
    s_zc_cnt = 0U;
    s_miss_cnt = 0U;
    Timers_Init();
}

void BSP_Sensorless_Start(uint16_t duty)
{
    if (duty < (BSP_MOTOR_PWM_ARR / 5U))
    {
        duty = (uint16_t)(BSP_MOTOR_PWM_ARR / 5U);
    }
    if (duty >= BSP_MOTOR_PWM_ARR)
    {
        duty = (uint16_t)(BSP_MOTOR_PWM_ARR - 1U);
    }

    s_target = duty;
    /* 参考: 启动固定从 QDPwm(20%) 起, 不跟电位器跳变 */
    s_duty = (uint16_t)BSP_MOTOR_DUTY_DEFAULT;
    s_slew_ms = 0U;
    s_st_ok = 0U;
    s_sta = 0U;
    s_fflag = 0U;
    s_zc_armed = 0U;
    s_vpeak = 80U;
    s_st_count = 0U;
    s_zc_cnt = 0U;
    s_miss_cnt = 0U;
    s_ol_steps = 0U;
    s_ol_wait = 0U;
    s_align_left = 10U; /* 对齐保持 ~10 ms */
    s_step = 0U;
    s_state = SLS_ST_ALIGN;

    BSP_MOTOR_SetDuty(s_duty);
    BSP_MOTOR_Start();
    BSP_MOTOR_CommutateStep(s_step);
    BTIM_SetCounter(CW_BTIM2, 0U);
}

void BSP_Sensorless_Stop(void)
{
    s_state = SLS_ST_IDLE;
    s_st_ok = 0U;
    s_sta = 0U;
    BTIM_Cmd(CW_BTIM3, DISABLE);
    BSP_MOTOR_Stop();
}

void BSP_Sensorless_SetDuty(uint16_t duty)
{
    if (duty < (BSP_MOTOR_PWM_ARR / 5U))
    {
        duty = (uint16_t)(BSP_MOTOR_PWM_ARR / 5U);
    }
    if (duty >= BSP_MOTOR_PWM_ARR)
    {
        duty = (uint16_t)(BSP_MOTOR_PWM_ARR - 1U);
    }
    /* 只记目标. 升速在换相/+30 或 RUN 每 30ms +1% 里慢慢跟上 (参考 MotorRunOPEN) */
    s_target = duty;
    if ((s_state != SLS_ST_IDLE) && (s_state != SLS_ST_FAULT) && (s_duty > s_target))
    {
        ApplyDuty(s_target);
    }
}

void BSP_Sensorless_SetDirection(int8_t dir)
{
    s_dir = (dir >= 0) ? 0U : 1U;
}

int8_t BSP_Sensorless_GetDirection(void)
{
    return (s_dir == 0U) ? 1 : -1;
}

void BSP_Sensorless_ToggleDirection(void)
{
    s_dir = (s_dir == 0U) ? 1U : 0U;
}

void BSP_Sensorless_Btim3IRQ(void)
{
    if (BTIM_GetITStatus(CW_BTIM3, BTIM_IT_UPDATE) == RESET)
    {
        return;
    }
    BTIM_ClearITPendingBit(CW_BTIM3, BTIM_IT_UPDATE);

    if (s_sta == 1U)
    {
        /* 退磁结束 → 开始过零检测 */
        s_sta = 2U;
        BTIM_Cmd(CW_BTIM3, DISABLE);
    }
    else if ((s_sta == 3U) && (s_st_ok == 1U))
    {
        /* 过零延时到 → 换相 */
        BTIM_Cmd(CW_BTIM3, DISABLE);
        StepNext();
        DoCommutate();
    }
}

void BSP_Sensorless_OnAdcSample(void)
{
    static uint8_t s_cou = 0U;
    uint16_t bemf;
    uint16_t thre;
    uint8_t  edge;
    uint8_t  hx = 0U;

    if ((s_state == SLS_ST_IDLE) || (s_state == SLS_ST_FAULT))
    {
        return;
    }
    if (s_sta != 2U)
    {
        return;
    }

    bemf = g_bemf_sample[s_bemf_ch[s_step]];
    s_bemf = bemf;

    /* 换相毛刺过去再判. 8MHz 计步, 16000 ≈ 2ms */
    if ((uint16_t)BTIM_GetCounter(CW_BTIM2) < 16000U)
    {
        return;
    }

    /* 用三相峰值的一半做稳定中点, 不跟单步采样跳 */
    {
        uint16_t mx = g_bemf_sample[0];
        if (g_bemf_sample[1] > mx) { mx = g_bemf_sample[1]; }
        if (g_bemf_sample[2] > mx) { mx = g_bemf_sample[2]; }
        if (mx > s_vpeak)
        {
            s_vpeak = mx;
        }
        else if (s_vpeak > 8U)
        {
            s_vpeak = (uint16_t)(s_vpeak - (s_vpeak >> 6));
        }
        thre = (uint16_t)(s_vpeak / 2U);
    }
    if (thre < 40U)
    {
        thre = 40U;
    }
    s_mid = thre;

    edge = s_rf[s_dir ^ 1U][s_step];

    /* 必须先看到过零前的一侧, 否则换相后立刻“过零”, 步距在 2ms/15ms 间乱跳 */
    if (s_zc_armed == 0U)
    {
        s_cou = 0U;
        if (edge == FALLING)
        {
            if (bemf > (uint16_t)(thre + 15U))
            {
                s_zc_armed = 1U;
            }
        }
        else if (bemf + 15U < thre)
        {
            s_zc_armed = 1U;
        }
        return;
    }

    if (edge == FALLING)
    {
        if (bemf < thre)
        {
            s_cou++;
            if (s_cou >= ZC_CONFIRM)
            {
                s_cou = 0U;
                s_sta = 3U;
                s_st_count++;
                s_fflag = 1U;
                hx = 1U;
                if (s_zc_cnt < 0xFFFFU)
                {
                    s_zc_cnt++;
                }
            }
        }
        else
        {
            s_cou = 0U;
        }
    }
    else
    {
        if (bemf > thre)
        {
            s_cou++;
            if (s_cou >= ZC_CONFIRM)
            {
                s_cou = 0U;
                s_sta = 3U;
                s_st_count++;
                s_fflag = 1U;
                hx = 1U;
                if (s_zc_cnt < 0xFFFFU)
                {
                    s_zc_cnt++;
                }
            }
        }
        else
        {
            s_cou = 0U;
        }
    }

    if ((s_st_count >= ST_ZC_NEED) && (s_st_ok == 0U))
    {
        s_st_ok = 1U;
        s_state = SLS_ST_RUN;
        s_run_ms = 0U;
        s_run_per = 8U;
    }
    (void)hx;
}

void BSP_Sensorless_Tick1ms(void)
{
    if (s_state == SLS_ST_ALIGN)
    {
        if (s_align_left > 0U)
        {
            s_align_left--;
        }
        if (s_align_left == 0U)
        {
            s_state = SLS_ST_RAMP;
            s_ol_wait = 0U;
            s_ol_steps = 0U;
            s_fflag = 0U;
            s_st_count = 0U;
            /* 进入开环第一步 */
            StepNext();
            DoCommutate();
            s_ol_wait = 0U;
        }
        return;
    }

    if (s_state == SLS_ST_RUN)
    {
        s_run_ms++;
        if (s_run_ms >= s_run_per)
        {
            uint32_t pct = ((uint32_t)s_duty * 100U) / BSP_MOTOR_PWM_ARR;
            uint32_t per;

            if (pct < 20U)
            {
                pct = 20U;
            }
            if (pct > 60U)
            {
                pct = 60U;
            }
            /* 占空比决定均匀步距: 20%→10ms, 60%→4ms. 不再被假过零越拖越慢 */
            per = 10U - ((pct - 20U) * 6U) / 40U;
            if (per < 4U)
            {
                per = 4U;
            }
            s_run_per = (uint8_t)per;
            s_run_ms = 0U;
            s_fflag = 0U;
            StepNext();
            DoCommutate();
            s_period = s_run_per;
        }

        s_slew_ms++;
        if (s_slew_ms >= 30U)
        {
            uint16_t step = (uint16_t)(BSP_MOTOR_PWM_ARR / 100U);
            s_slew_ms = 0U;
            if (s_duty + step < s_target)
            {
                ApplyDuty((uint16_t)(s_duty + step));
            }
            else if (s_duty != s_target)
            {
                ApplyDuty(s_target);
            }
        }
        return;
    }

    if (s_state != SLS_ST_RAMP)
    {
        return;
    }

    /* 参考 Sensorless_START: 最多等 15ms, 见到过零且已过 2ms 则提前换相 */
    s_ol_wait++;
    if (s_ol_wait < OL_STEP_MS)
    {
        if ((s_fflag == 0U) || (s_ol_wait < 4U))
        {
            return;
        }
    }

    if (s_st_ok != 0U)
    {
        return;
    }

    if (s_fflag == 0U)
    {
        if (s_miss_run < 3U)
        {
            s_miss_run++;
        }
        else
        {
            s_st_count = 0U;
        }
    }
    else
    {
        s_miss_run = 0U;
    }
    s_fflag = 0U;

    StepNext();
    DoCommutate();
    s_ol_steps++;
    s_ol_wait = 0U;

    /* 参考每步 OutPwmValue += 30, 不超过电位器目标 */
    if (s_duty < s_target)
    {
        uint16_t d = (uint16_t)(s_duty + 30U);
        if (d > s_target)
        {
            d = s_target;
        }
        ApplyDuty(d);
    }
}

SensorlessState_t BSP_Sensorless_GetState(void) { return s_state; }
uint8_t  BSP_Sensorless_GetStep(void)   { return s_step; }
uint16_t BSP_Sensorless_GetBemf(void)   { return s_bemf; }
uint16_t BSP_Sensorless_GetMid(void)    { return s_mid; }
uint16_t BSP_Sensorless_GetPeriod(void) { return s_period; }
uint16_t BSP_Sensorless_GetMissCnt(void){ return s_miss_cnt; }
uint16_t BSP_Sensorless_GetZcCnt(void)  { return s_zc_cnt; }
