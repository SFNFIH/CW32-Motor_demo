# CW32L012 + DengFOC 2208 无刷电机控制

基于 **CW32L012C8**（Cortex-M0+，HSI 96 MHz）的三相无刷电机控制工程。  
控制路径：**互补 SPWM / 中点注入 SVPWM + 开环 V/f**，AS5600 测速 / 测角，相电流采样做 **Imax 上限**。

当前分支 **`feature/current-loop`**：在速度环、绝对位置角度环之上，用双击第三态设定电流上限，并叠加到两个外环。

---

## 功能概览

| 项目 | 说明 |
|------|------|
| 电机 | DengFOC 2208，7 极对，相电阻约 8 Ω，母线约 12 V |
| 驱动波形 | ATIM 中心对齐互补 PWM，约 **15 kHz**，带死区；中点注入近似 SVPWM |
| 速度环 | 电位器 → 约 **15～1350 rpm**（12 V 下可同步上限约 1200 rpm） |
| 角度环 | 电位器 **0～4095** → **0～360°**；反馈用 AS5600 **`cum_raw` 绝对多圈**，转过整圈仍能回位 |
| 电流上限 | 电位器设定 **Imax（约 0.12～1.5 A）**，按比例限制调制电压，**叠加**在速度/角度上 |
| 速度/位置反馈 | AS5600，**硬件 I2C1** |
| 电流反馈 | 片内 OPA + ADC1（PB0 / PB1） |
| 通信 | UART1 JustFloat（VOFA+，**10 通道**）；PyOCD 可读 `g_dbg` |
| 操作 | **单击**启停；**双击**轮换：速度 → 角度 → 电流上限（LED 闪烁） |

> **说明：** 这不是 `Id/Iq` FOC 电流环，而是 V/f 外环 + 电流上限。`BSP_FOC_ToggleDirection()` 仍保留，默认按键 **不再换向**。  
> **12 V：** 反电势大约把同步转速顶在 **1200 rpm**。

---

## 操作说明

1. 接好电机、12 V、AS5600、电位器、电流采样。  
2. 烧录后复位，LED 灭 = 停机。  
3. **单击**启动（LED 常亮）。  
4. **速度环：** 电位器调速，建议从低速慢慢升高。  
5. **双击 → 角度环：** 电位器 0 → 该圈 0°，满量程 → 约 4095 raw。超圈后仍按绝对位置回去。  
6. **再双击 → 电流上限：** LED **闪烁**。电位器只改 **Imax**，刚才的速度/角度外环继续跑。拧小电位器，设定电流下降、力矩变弱。  
7. **再双击**回到速度环（LED 常亮），**Imax 记住**，直到下次再进电流设定。  
8. **单击**停止。

VOFA+ 请把通道数设为 **10**。看 **通道 8（设定电流）** 和 **通道 9（实时电流）** 是否跟着电位器和负载变化。

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

开漏 + 内部上拉，100 kHz（与 `cw32l012_i2c_master_int` 一致）。

### 人机、电流与调试

| 功能 | 引脚 |
|------|------|
| 按键（低有效，内部上拉） | PB10 |
| 电位器 ADC | PA9 → ADC2 CH6 |
| 电流 A | OPA1 → PB0 → ADC1 CH8 |
| 电流 B | OPA2 → PB1 → ADC1 CH9 |
| UART1 TX / RX | PB12 / PB11，115200 |
| LED | PC13（低电平点亮） |
| SWD | SWDIO / SWCLK / 5V / GND |

原理图见 `Doc/`。

---

## 软件架构

```
USER/src/main.c          主循环: 电位器、AS5600、三态模式、JustFloat
BSP/BSP_FOC.c            V/f 速度 + 绝对角度 + Imax 电压缩放
BSP/BSP_Current.c        A/B 相电流 (OPA + ADC1)
BSP/BSP_MOTOR.c          ATIM 互补 PWM
BSP/BSP_AS5600.c         硬件 I2C 磁编（cum_raw 多圈）
BSP/BSP_Button.c         单击 / 双击
BSP/BSP_Potentiometer.c  电位器
BSP/BSP_UART.c           JustFloat
BSP/BSP_DebugSnap.c      PyOCD g_dbg
Libraries/               CW32 标准外设库
```

### 速度环

电位器 → 目标转速（斜坡）→ `fe` 前馈 + 小 PI；中低速压幅、限超前，减轻失步。

### 角度环

进模式时锁定圈基址，使电位器 0 对准该圈 0°。绝对误差 `目标 − cum` 不取模。到位后 **停频 + 保持电压**（不再注入最低转速，避免发抖）。

### 电流上限（内环叠加）

- 第三态：电位器 → `Imax`，LED 闪烁。  
- 对速度/角度 **只缩小调制电压、不改频率**；角度到位时保持停频。  
- `Imax` 缓变，减轻电位器噪声。  
- 尚无 Park/`Iq` 闭环，通道 9 为相电流绝对值之和的滤波，便于对照设定值。

---

## 环境依赖

- `arm-none-eabi-gcc`（可用 STM32CubeCLT）
- CMake ≥ 3.22
- Python3 + **pyOCD**
- CMSIS Pack：`WHXY/CW32L012_DFP`

```text
~/.local/share/cmsis-pack-manager/WHXY/CW32L012_DFP/1.0.2.pack
```

```bash
python3 scripts/install_cw32_pack.py
python3 scripts/install_cw32_pack.py /path/to/WHXY.CW32L012_DFP.1.0.2.pack
```

---

## 编译与烧录

```bash
cmake --preset Debug
cmake --build build/Debug -j
python3 flash_cw32.py build/Debug/cw32l012_blank.elf
```

产物：`build/Debug/cw32l012_blank.elf`（同目录 `.hex` / `.bin`）。  
`pyocd.yml`：目标 `cw32l012c8`，`under-reset`。烧录前请关掉占用串口的软件。

---

## VOFA+ JustFloat

UART1 发送 **10** 个 float + 帧尾 `00 00 80 7F`。

| 通道 | 速度环 | 角度环 | 电流设定（LED 闪） |
|------|--------|--------|-------------------|
| 0 | mode（0 停 / 1 爬升 / 2 速度 / 3 角度 / 4 失步恢复） | 同左 | 同左 |
| 1 | `fe_x10` | 调制幅度 | 同外环遗留字段 |
| 2 | 目标转速 (rpm) | 目标角 (°) | 同外环遗留字段 |
| 3 | 电角度抽样 | 累计角 (rad) | 同外环 |
| 4 | AS5600 raw | 同左 | 同左 |
| 5 | 实测转速 (rpm) | 同左 | 同左 |
| 6 | 电位器 0～1000 | 实测角 (°) | Imax 千分比 |
| 7 | 电位器 ADC | 同左 | 同左 |
| **8** | **设定电流 Imax (A)** | 同左 | 拧电位器变化 |
| **9** | **实时电流 (A)** | 同左 | 同左 |

建议在 VOFA 把通道 8、9 命名为 `Iset`、`Imeas`。

### PyOCD

`g_dbg`（`BSP_DebugSnap.h`）：`as_ok`、`as_raw`、`as_cum`、mode 等。  
`g_force_duty` 非 0 时强制速度给定（角度/电流映射见 `main.c`）。不要用 halt 测转速。

---

## 已知限制

- 开环 V/f + 编码器 + **电流上限**，不是 FOC `Id/Iq` 环。  
- 12 V 机械转速大约 **1.2 krpm** 封顶。  
- 角度到位有死区（约 6 raw）。  
- 电流上限有电压下限，避免锁角时电压过低发抖，因此 Imax 拧到很小也可能仍有一点力矩。  
- `BSP_Sensorless` / `BSP_BEMF` 等历史模块默认未接入。

---

## 目录结构

| 路径 | 说明 |
|------|------|
| `BSP/` | 板级驱动与控制 |
| `USER/` | `main`、中断、`SystemInit` |
| `Libraries/` | CW32 外设库 |
| `Doc/` | 原理图 |
| `cmake/` | 工具链 |
| `scripts/` | pack 安装、调试 |
| `flash_cw32.py` | pyOCD 烧录 |
| `cw32l012_flash.ld` | 链接脚本 |
| `startup_cw32l012x8.s` | 启动文件 |

---

## License

学习与个人开发用途；CW32 库文件遵循原厂许可。
