# CW32L012 + DengFOC 2208 无刷速度闭环

基于 **CW32L012C8**（Cortex-M0+，HSI 96 MHz）的三相无刷电机控制工程。  
当前主路径：**互补 SPWM / SVPWM 开环 V/f + AS5600 测速闭环**，电位器给定转速，按键启停 / 换向。

---

## 功能概览

| 项目 | 说明 |
|------|------|
| 电机 | DengFOC 2208，7 极对，相电阻约 8 Ω，母线约 12 V |
| 驱动波形 | ATIM 中心对齐互补 PWM，约 **15 kHz**，带死区；中点注入近似 SVPWM |
| 速度给定 | 电位器 → 约 **15～1350 rpm**（实际可同步上限约 1200 rpm @12 V） |
| 速度反馈 | AS5600 磁编码器，**硬件 I2C1** |
| 控制 | `fe` 前馈 + PI 微调；中低速压 V/f、限超前，减轻 100～300 rpm 抖动 |
| 通信 | UART1 JustFloat（VOFA+）；PyOCD 可读 `g_dbg` 快照 |
| 操作 | **单击**启停，**双击**换向 |

> **说明：** 12 V 下反电势会把开环同步速度顶在约 **1200 rpm** 附近；再高需要弱磁或升压。电流环 / 角度 FOC 相关文件仍在仓库中，但当前 `main` 未启用。

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

与参考例程 `cw32l012_i2c_master_int` 一致：开漏 + 内部上拉，100 kHz。

### 人机与调试

| 功能 | 引脚 |
|------|------|
| 按键（低有效，内部上拉） | PB10 |
| 电位器 ADC | PA9 → ADC2 CH6 |
| UART1 TX / RX | PB12 / PB11，115200 |
| LED | PC13（低电平点亮） |
| SWD | SWDIO / SWCLK / 5V / GND |

原理图见 `Doc/` 目录。

---

## 软件架构

```
USER/src/main.c          主循环: 电位器、AS5600、速度环、遥测
BSP/BSP_FOC.c            V/f 速度闭环 + SPWM 中断
BSP/BSP_MOTOR.c          ATIM 互补 PWM
BSP/BSP_AS5600.c         硬件 I2C 磁编
BSP/BSP_Button.c         单击 / 双击
BSP/BSP_Potentiometer.c  电位器
BSP/BSP_UART.c           JustFloat
BSP/BSP_DebugSnap.c      PyOCD 调试快照 g_dbg
Libraries/               CW32 标准外设库
```

### 控制要点（`BSP_FOC.c`）

1. 电位器映射目标转速，斜坡限速。  
2. AS5600 累计角 + `g_millis` 测机械转速。  
3. 目标转速 → 电频率 `fe` 前馈；PI 只做小范围修正。  
4. 中低速降低调制幅度，限制相对实测的 `fe` 超前，避免拧飞失步。  
5. PWM 中断里推进相位并输出三相占空比；启停只由主循环按键回调处理。

---

## 环境依赖

- `arm-none-eabi-gcc`（可用 STM32CubeCLT 自带工具链）
- CMake ≥ 3.22
- Python3 + **pyOCD**（建议独立 venv）
- CMSIS Pack：`WHXY/CW32L012_DFP`（不在公网索引，需本地安装）

Pack 默认路径：

```text
~/.local/share/cmsis-pack-manager/WHXY/CW32L012_DFP/1.0.2.pack
```

安装 / 更新：

```bash
python3 scripts/install_cw32_pack.py
# 或指定本地 pack：
python3 scripts/install_cw32_pack.py /path/to/WHXY.CW32L012_DFP.1.0.2.pack
```

---

## 编译

```bash
cmake --preset Debug
cmake --build build/Debug -j
```

产物：

- `build/Debug/cw32l012_blank.elf`
- 同目录 `.hex` / `.bin`

---

## 烧录

关闭占用调试口的串口软件后：

```bash
python3 flash_cw32.py build/Debug/cw32l012_blank.elf
```

或：

```bash
/home/tony/DAPLink/third_party/DAPLink/venv/bin/python flash_cw32.py
```

`pyocd.yml` 已配置目标 `cw32l012c8` 与 pack 路径。连接方式：`under-reset`。

---

## 使用说明

1. 接好电机、12 V 母线、AS5600、电位器。  
2. 烧录后复位，LED 未亮表示停机。  
3. **单击按键**启动；拧电位器调速（建议从低速慢慢升高）。  
4. 再 **单击** 停止；**双击** 切换旋转方向。  
5. 约 **40 rpm** 起可较稳跟随；**100～300 rpm** 已针对抖动做了 V/f 与 PI 柔化；高于约 **1120～1200 rpm** 会接近 12 V 反电势上限，继续拧电位器不会明显再升速，但应保持同步而不失步崩溃。

---

## 调试与遥测

### VOFA+ JustFloat

UART1 周期发送 8 个 float，例如：

| 序号 | 含义 |
|------|------|
| 0 | mode（0 停 / 1 开环爬升 / 2 闭环 / 4 失步恢复） |
| 1 | `fe_x10` 指令 |
| 2 | 目标转速（rpm） |
| 3 | 电角度（rad，抽样） |
| 4 | AS5600 raw |
| 5 | 实测转速（rpm） |
| 6 | 电位器归一化 0～1000 |
| 7 | 电位器 ADC |

### PyOCD 快照

全局 `g_dbg`（见 `BSP_DebugSnap.h`）含 `as_ok`、`as_raw`、`as_cum`、mode 等。  
可用 `scripts/pyocd_loop_debug.py` 或自写脚本读写；`g_force_duty` 非 0 时可强制速度给定（便于自动化测速）。

注意：用调试器 **halt** 测转速会干扰时序；应用 `as_cum` 在运行中做墙钟差分更可靠。

---

## 已知限制

- 当前为 **开环 V/f + 编码器测速**，不是电流环 FOC；极低速顺滑度、抗负载能力有限。  
- 母线约 12 V 时机械转速上限约 **1.2 krpm** 量级。  
- 仓库内仍有 `BSP_Sensorless` / `BSP_BEMF` / `BSP_Current` 等历史模块，默认未接入 `main`。  
- I2C 与电机同板时请保证地线良好；AS5600 需磁铁对准、`STATUS.MD` 有效。

---

## 目录结构

| 路径 | 说明 |
|------|------|
| `BSP/` | 板级驱动与控制 |
| `USER/` | `main`、中断、`SystemInit` 覆盖 |
| `Libraries/` | CW32 外设库 |
| `Doc/` | 原理图等资料 |
| `cmake/` | `arm-none-eabi` 工具链 |
| `scripts/` | pack 安装、调试辅助 |
| `flash_cw32.py` | pyOCD 烧录入口 |
| `cw32l012_flash.ld` | 链接脚本 |
| `startup_cw32l012x8.s` | 启动文件 |

---

## License

本仓库以学习与个人开发为目的；CW32 库文件请遵循原厂许可。
