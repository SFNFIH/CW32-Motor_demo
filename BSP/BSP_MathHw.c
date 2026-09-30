/**
 * @file    BSP_MathHw.c
 * @brief   CORDIC + EAU 硬件数学封装 (对照官方例程用法)
 */
#include "BSP_MathHw.h"
#include "cw32l012_cordic.h"
#include "cw32l012_eau.h"
#include "cw32l012.h"

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define MATH_Q31_MAX   (0.999999f)
#define MATH_EPS       (1.0e-12f)

static void cordic_wait_eoc(void)
{
    uint32_t guard = 100000UL;
    while ((CORDIC_GetStatus().eoc == 0U) && (guard > 0U))
    {
        guard--;
    }
}

static void eau_wait_idle(void)
{
    uint32_t guard = 100000UL;
    while (((uint32_t)EAU_GetStatus() & (uint32_t)EAU_STATUS_BUSY) != 0U && (guard > 0U))
    {
        guard--;
    }
}

static int32_t clamp_q31(float v)
{
    if (v > MATH_Q31_MAX) { v = MATH_Q31_MAX; }
    if (v < -MATH_Q31_MAX) { v = -MATH_Q31_MAX; }
    return float_to_q1_31(v);
}

static int32_t clamp_q15(float v)
{
    if (v > MATH_Q31_MAX) { v = MATH_Q31_MAX; }
    if (v < -MATH_Q31_MAX) { v = -MATH_Q31_MAX; }
    return float_to_q1_15(v);
}

void BSP_MathHw_Init(void)
{
    cordic_init_t init = {
        .func = CORDIC_FUNC_SQRT,
        .scale = 0,
        .format = CORDIC_FORMAT_Q1_31,
        .iter = CORDIC_ITER_32,
        .comp = 1,
        .ie = 0,
        .dmaeoc = 0,
        .dmaidle = 0
    };
    CORDIC_Init(&init);
    EAU_Init();
}

uint32_t BSP_MathHw_ISqrt(uint32_t n)
{
    EAU_SetMode(EAU_MODE_SQRT);
    EAU_StartOperation(n, 0U);
    eau_wait_idle();
    return EAU_GetQuotient();
}

float BSP_MathHw_Sqrt(float x)
{
    uint32_t n;
    uint32_t r;

    if (x <= 0.0f)
    {
        return 0.0f;
    }
    /* Q20 定点: sqrt(x) = sqrt(x·2^20) / 2^10 */
    if (x > 2000.0f)
    {
        /* 过大时先缩一半指数, 避免 32-bit 溢出 */
        float s = BSP_MathHw_Sqrt(x * 0.25f);
        return s * 2.0f;
    }
    n = (uint32_t)(x * 1048576.0f + 0.5f);
    if (n == 0U)
    {
        return 0.0f;
    }
    r = BSP_MathHw_ISqrt(n);
    return (float)r * (1.0f / 1024.0f);
}

float BSP_MathHw_Hypot(float x, float y)
{
    cordic_init_t init;
    float ax;
    float ay;
    float m;
    float xn;
    float yn;
    float r;
    int32_t res;

    ax = (x >= 0.0f) ? x : -x;
    ay = (y >= 0.0f) ? y : -y;
    m = (ax > ay) ? ax : ay;
    if (m < MATH_EPS)
    {
        return 0.0f;
    }

    /* 归一化到 |·|≤0.5, 保证 Q1.31 不饱和 */
    xn = (x / m) * 0.5f;
    yn = (y / m) * 0.5f;

    init.func = CORDIC_FUNC_HYPOT;
    init.scale = 0;
    init.format = CORDIC_FORMAT_Q1_31;
    init.iter = CORDIC_ITER_32;
    init.comp = 1;
    init.ie = 0;
    init.dmaeoc = 0;
    init.dmaidle = 0;
    CORDIC_Init(&init);

    CW_CORDIC->X = (uint32_t)clamp_q31(xn);
    CW_CORDIC->Y = (uint32_t)clamp_q31(yn); /* 写 Y 启动 */
    cordic_wait_eoc();
    res = (int32_t)CW_CORDIC->X;
    r = q1_31_to_float(res);
    return r * m * 2.0f;
}

float BSP_MathHw_Atan2(float y, float x)
{
    cordic_init_t init;
    float ax;
    float ay;
    float m;
    float xn;
    float yn;
    int32_t z;
    float pi_units;

    ax = (x >= 0.0f) ? x : -x;
    ay = (y >= 0.0f) ? y : -y;
    m = (ax > ay) ? ax : ay;
    if (m < MATH_EPS)
    {
        return 0.0f;
    }
    xn = x / m;
    yn = y / m;

    init.func = CORDIC_FUNC_ATAN2;
    init.scale = 0;
    init.format = CORDIC_FORMAT_Q1_15;
    init.iter = CORDIC_ITER_32;
    init.comp = 1;
    init.ie = 0;
    init.dmaeoc = 0;
    init.dmaidle = 0;
    CORDIC_Init(&init);

    CW_CORDIC->X = (uint32_t)clamp_q15(xn);
    CW_CORDIC->Y = (uint32_t)clamp_q15(yn);
    cordic_wait_eoc();
    z = (int32_t)CW_CORDIC->Z;
    /* 例程: 结果以 π 为单位, Q1.15 */
    pi_units = q1_15_to_float((int32_t)(int16_t)(z & 0xFFFF));
    return pi_units * (float)M_PI;
}

void BSP_MathHw_CosSin(float angle_rad, float *c, float *s)
{
    cordic_init_t init;
    float pi_units;
    int32_t z;
    int32_t xr;
    int32_t yr;

    /* 折到 (-1,1] ·π */
    pi_units = angle_rad / (float)M_PI;
    while (pi_units > 1.0f) { pi_units -= 2.0f; }
    while (pi_units <= -1.0f) { pi_units += 2.0f; }
    z = clamp_q15(pi_units);

    init.scale = 0;
    init.format = CORDIC_FORMAT_Q1_15;
    init.iter = CORDIC_ITER_20;
    init.comp = 1;
    init.ie = 0;
    init.dmaeoc = 0;
    init.dmaidle = 0;

    if (c != 0)
    {
        init.func = CORDIC_FUNC_COS;
        CORDIC_Init(&init);
        CW_CORDIC->Z = (uint32_t)z;
        cordic_wait_eoc();
        xr = (int32_t)CW_CORDIC->X;
        *c = q1_15_to_float((int32_t)(int16_t)(xr & 0xFFFF));
    }
    if (s != 0)
    {
        init.func = CORDIC_FUNC_SIN;
        CORDIC_Init(&init);
        CW_CORDIC->Z = (uint32_t)z;
        cordic_wait_eoc();
        yr = (int32_t)CW_CORDIC->X; /* SIN 结果在 X, 与 COS 同寄存器约定 */
        *s = q1_15_to_float((int32_t)(int16_t)(yr & 0xFFFF));
    }
}

float BSP_MathHw_Div(float num, float den)
{
    float aden;
    float anum;
    float n;
    float d;
    float sn;
    float sd;
    int32_t ni;
    int32_t di;
    int32_t q;

    aden = (den >= 0.0f) ? den : -den;
    if (aden < MATH_EPS)
    {
        return 0.0f;
    }

    /*
     * EAU 32-bit 有符号除: 先把 |num|,|den| 收到安全区再 Q12/Q4 定点。
     * 溢出或除零则回退软浮点, 避免 HFI (f·di) 抖动污染 invL。
     */
    anum = (num >= 0.0f) ? num : -num;
    n = num;
    d = den;
    sn = 1.0f;
    sd = 1.0f;
    while (anum > 200.0f)
    {
        n *= 0.5f;
        anum *= 0.5f;
        sn *= 0.5f;
    }
    while ((anum > MATH_EPS) && (anum < 0.02f))
    {
        n *= 2.0f;
        anum *= 2.0f;
        sn *= 2.0f;
    }
    while (aden > 200.0f)
    {
        d *= 0.5f;
        aden *= 0.5f;
        sd *= 0.5f;
    }
    while ((aden > MATH_EPS) && (aden < 0.02f))
    {
        d *= 2.0f;
        aden *= 2.0f;
        sd *= 2.0f;
    }

    ni = (int32_t)(n * 4096.0f); /* Q12 */
    di = (int32_t)(d * 16.0f);   /* Q4  → 商为 Q8 */
    if (di == 0)
    {
        return num / den;
    }

    EAU_SetMode(EAU_MODE_SIGNED_DIV);
    EAU_StartOperation((uint32_t)ni, (uint32_t)di);
    eau_wait_idle();
    if (((uint32_t)EAU_GetStatus() &
         ((uint32_t)EAU_STATUS_DIV_ZERO | (uint32_t)EAU_STATUS_OVERFLOW)) != 0U)
    {
        return num / den;
    }
    q = (int32_t)EAU_GetQuotient();
    /* (n/d) = q/256; 再补偿 sn/sd: true = (n/sn)/(d/sd) = (n/d)*(sd/sn) */
    return ((float)q * (1.0f / 256.0f)) * (sd / sn);
}
