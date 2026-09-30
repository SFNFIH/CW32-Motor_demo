# CW32L012 + DengFOC 2208 无刷电机控制

基于 **CW32L012C8**（Cortex-M0+，HSI 96 MHz）的三相无刷电机控制工程。  
控制路径：**互补 SPWM / 中点注入 SVPWM + 开环 V/f**，AS5600 测速 / 测角，相电流采样做 **Imax 上限**，并移植了 **VESC 风格电机自整定（R / L / Flux）**。

当前在 `feature/current-loop` 之上增加 `BSP_MotorDetect` / `BSP_HFI`：算法对齐 [vedderb/bldc](https://github.com/vedderb/bldc) 的 `mcpwm_foc_measure_resistance` / **六矢量 HFI** `measure_inductance` / `measure_flux_linkage_openloop` / `detect_apply_all_foc`。

---

## 功能概览

| 项目 | 说明 |
|------|------|
| 电机 | DengFOC 2208，7 极对，相电阻约 8 Ω，母线约 12 V |
| 驱动波形 | ATIM 中心对齐互补 PWM，约 **15 kHz**，带死区；中点注入近似 SVPWM |
| 速度环 | 电位器 → 约 **15～1350 rpm**（12 V 下可同步上限约 1200 rpm） |
| 角度环 | 电位器 **0～4095** → **0～360°**；反馈用 AS5600 **`cum_raw` 绝对多圈** |
| 电流上限 | 电位器设定 **Imax（约 0.12～1.5 A）**，按比例限制调制电压 |
| 自整定 | 测 **Rs / Ls / λ**，并按 VESC 公式算电流环 **kp/ki** 与 observer gain |
| 速度/位置反馈 | AS5600，**硬件 I2C1** |
| 电流反馈 | 片内 OPA + ADC1（PB0 / PB1） |
| 母线电压 | PA8 → ADC2 CH5（自整定用） |
| 通信 | UART1 JustFloat（VOFA+，**10 通道**）；PyOCD 可读 `g_dbg` |
| 操作 | **单击**启停；**双击**轮换速度/角度/Imax；**停机+电位器最低时双击** → 自整定 |

> **说明：** 这不是 `Id/Iq` FOC 电流环，而是 V/f 外环 + 电流上限。自整定得到的 kp/ki 供后续电流环使用。  
> **12 V：** 反电势大约把同步转速顶在 **1200 rpm**。

---

## 操作说明

1. 接好电机、12 V、AS5600、电位器、电流采样。  
2. 烧录后复位，LED 灭 = 停机。  
3. **单击**启动（LED 常亮）。  
4. **速度环：** 电位器调速，建议从低速慢慢升高。  
5. **双击 → 角度环**；**再双击 → 电流上限**（LED 闪烁）；**再双击**回速度环。  
6. **单击**停止。  
7. **自整定：** 停机后把电位器拧到**最低**，再**双击**；或 PyOCD 写 `g_dbg.cmd = 4`。电机会注入电流并短时旋转。成功后 LED 慢闪；VOFA 停机时 ch8=`Rs(Ω)`、ch9=`Ls(µH)`。

VOFA+ 通道数设为 **10**。

---

## 电机自整定（VESC 移植）

源码：`BSP/BSP_MotorDetect.c`，对照 `vedderb/bldc`：

| VESC | 本工程 | 做法 |
|------|--------|------|
| `mcpwm_foc_measure_resistance` | `BSP_MotorDetect_MeasureR` | 锁轴 DC 注入 A/B（C 中点），`R = Van/Ia` |
| `mcpwm_foc_measure_inductance*` | `BSP_HFI` 六矢量 HFI + FFT bin0/bin2 → L / (Lq−Ld) |
| `conf_general_measure_flux_linkage_openloop` | `BSP_MotorDetect_MeasureFlux` | 开环 V/f + AS5600，`λ=(V−IR)/ωe−IL` |
| `measure_r_l_imax` / `detect_apply_all_foc` | `BSP_MotorDetect_RunAll` | 功耗爬升电流 → R → L → Flux → kp/ki |
| `conf_general_calc_apply_foc_cc_kp_ki_gain` | 结果 `kp/ki` | `bw=1/(1000µs)`（detect_apply），`kp=L·bw`，`ki=R·bw` |

电感结果乘 **0.9**（与 VESC 一致）。HFI 同时给出 **Lq−Ld**。  
默认 `max_power_loss = 5 W`，电流硬限约 **1.6 A**。整定中请保证电机可自由转动。  
`Apply` 写入 `i_max` → FOC Imax；`enc_inverted` 只作编码器 polarity 记录，不翻转开环转向。

自整定中的 `hypot` / `sqrt` / `div` / `atan2` 走片上 **CORDIC + EAU**（`BSP_MathHw`，对照官方例程）。

### 已对齐 / 刻意未移植

| 项目 | 状态 |
|------|------|
| DC 电流偏置校准 | ✅ RunAll 开头 `BSP_Current_Calibrate` |
| 相电阻 R | ✅ DC 注入 + **死区电压补偿**；搜索段保持注入 |
| 电感 L | ✅ **六矢量 HFI**（VESC SIX_VECTOR + FFT），得 L 与 Lq−Ld |
| 磁链 λ（驱动） | ✅ 开环 V/f + AS5600 |
| 磁链 λ（无驱 coasting） | ✅ 关 PWM 后读 BEMF（PA0/1/2，与电流同序无冲突），`λ=\|Vαβ\|/ωe`；样本>60 优先无驱 |
| 编码器 offset/ratio/invert | ✅ 三轴 DC 锁相（简化 `encoder_detect`） |
| Hall 表检测 | ❌ 无 Hall |
| kp/ki（tc=1000µs） | ✅ 与 `detect_apply_all_foc` 一致 |
| 应用 i_max 电流限 | ✅ `Apply` → `BSP_FOC_SetImaxPm` |
| 电机温度补偿 | ❌ 无温度传感器 |
| EEPROM / CAN 多机 | ❌ 不适用 |
| 旧版 BLDC `detect_motor_param` | ❌ 梯形波路径，非 FOC 自整定 |

---

## 硬件连接

### 电机 PWM（ATIM）

| 信号 | 引脚 |
|------|------|
| UH / UL | PB5 / PA15 |
| VH / VL | PB6 / PB3 |
| WH / WL | PB7 / PB4 |

### AS5600（I2C1）

| 信号 | 引脚 |
|------|------|
| SDA | PC14 |
| SCL | PC15 |
| 地址 | `0x36` |

### 人机、电流与调试

| 功能 | 引脚 |
|------|------|
| 按键（低有效，内部上拉） | PB10 |
| 电位器 ADC | PA9 → ADC2 CH6 |
| 母线电压 | PA8 → ADC2 CH5 |
| 电流 A | OPA1 → PB0 → ADC1 CH8 |
| 电流 B | OPA2 → PB1 → ADC1 CH9 |
| 相电压 BEMF | MCU_EA/EB/EC → PA0/1/2 → ADC1 CH0/1/2（与电流独立，同序扫描） |
| UART1 TX / RX | PB12 / PB11，115200 |
| LED | PC13（低电平点亮） |
| SWD | SWDIO / SWCLK / 5V / GND |

原理图见 `Doc/`。

---

## 软件架构

```
USER/src/main.c          主循环: 三态模式、自整定触发、JustFloat
BSP/BSP_FOC.c            V/f 速度 + 绝对角度 + Imax 电压缩放
BSP/BSP_MotorDetect.c    VESC 风格 R/L/Flux(驱动+无驱) 自整定
BSP/BSP_HFI.c            六矢量 HFI 电感
BSP/BSP_BEMF.c           相电压 PA0/1/2 (与电流同 ADC1 序列, 无冲突)
BSP/BSP_MathHw.c         CORDIC(hypot/atan2/cos/sin) + EAU(div/sqrt)
BSP/BSP_Current.c        A/B 相电流 (OPA + ADC1 CH8/9)
BSP/BSP_Vbus.c           母线电压
BSP/BSP_MOTOR.c          ATIM 互补 PWM
BSP/BSP_AS5600.c         硬件 I2C 磁编（cum_raw 多圈）
BSP/BSP_Button.c         单击 / 双击
BSP/BSP_Potentiometer.c  电位器
BSP/BSP_UART.c           JustFloat
BSP/BSP_DebugSnap.c      PyOCD g_dbg (cmd=4 触发整定)
Libraries/               CW32 标准外设库
```

### PyOCD 调试命令（`g_dbg.cmd`）

| 值 | 含义 |
|----|------|
| 1 | 启动 |
| 2 | 停止 |
| 3 | 切换控制模式 |
| 4 | 电机自整定 |

停机且已整定时，`g_dbg.bemf`≈Rs(mΩ)，`mid`≈Ls(µH)，`duty`≈Flux(µWb)。

---

## 环境依赖

- `arm-none-eabi-gcc`（可用 STM32CubeCLT）
- CMake ≥ 3.22
- Python3 + **pyOCD**
- CMSIS Pack：`WHXY/CW32L012_DFP`

```bash
python3 scripts/install_cw32_pack.py
cmake --preset Debug
cmake --build build/Debug -j
python3 flash_cw32.py build/Debug/cw32l012_blank.elf
```

---

## 调试与遥测

### VOFA+ JustFloat（10 通道）

| 序号 | 运行时 | 停机且已整定 |
|------|--------|--------------|
| 0 | mode | mode |
| 1 | id 遥测 | **enc_ratio**（≈极对数） |
| 2 | iq 遥测 | **enc_inverted** |
| 3 | theta | **enc_offset_deg** |
| 5 | rpm | **flux (mWb)** |
| 6 | 电位器归一等 | **kp** |
| 8 | Imax (A) | **Rs (Ω)** |
| 9 | Imeas (A) | **Ls (µH)** |

---

## License

本仓库以学习与个人开发为目的；CW32 库文件请遵循原厂许可。  
VESC/bldc 算法思路来自 Benjamin Vedder 开源固件，移植时保留对应注释便于对照。
