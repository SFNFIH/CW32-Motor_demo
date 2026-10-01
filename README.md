# CW32L012 + SguanFOC 无感 FOC（DengFOC 2208）

基于 **CW32L012C8**（Cortex-M0+，HSI 96 MHz）的三相无刷电机控制工程。  
当前主路径：**[SguanFOC v3.1.0](https://github.com/Sguan-ZhouQing/SguanFOC_Library) 无感滑模观测（IF → SMO）**，电位器给定转速，按键启停 / 换向。

库源码位于 `Middlewares/SguanFOC/`（MIT，来自 Sguan-ZhouQing/SguanFOC_Library）。

---

## 功能概览

| 项目 | 说明 |
|------|------|
| 电机 | DengFOC 2208，7 极对，相电阻约 8 Ω，母线约 12 V |
| 控制库 | SguanFOC v3.1.0，`Define_Run_Mode = 8`（`MODE_Sensorless_SMO`） |
| 驱动波形 | ATIM 中心对齐互补 PWM，**10 kHz**，SVPWM |
| 电流采样 | 下桥臂 A/B 分流 + 片内 OPA + ADC1，ISR 内采样 |
| 速度给定 | 电位器 → 约 **0～100 rad/s**（约 0～955 rpm 机械） |
| 速度反馈 | **无感**：IF 强拖启动 → SMO + PLL |
| 通信 | UART1 JustFloat（VOFA+ / Sguan 上位机）；串口指令 `MOTOR=1?` / `Speed=50.0?` |
| 操作 | **单击**启停，**双击**换向 |

> 旧版 AS5600 V/f 速度闭环代码仍保留在 `BSP/BSP_FOC*.c`、`BSP_AS5600*.c` 等，但当前 `main` 未编入。

---

## 硬件连接

### 电机 PWM（ATIM）

| 信号 | 引脚 |
|------|------|
| UH / UL | PB5 / PA15 |
| VH / VL | PB6 / PB3 |
| WH / WL | PB7 / PB4 |

### 相电流（ADC1）

| 信号 | 引脚 |
|------|------|
| OPA1 IN+/IN− / OUT（A 相） | PA6 / PA7 / PB0 → ADC1 CH8 |
| OPA2 IN+/IN− / OUT（B 相） | PA4 / PA5 / PB1 → ADC1 CH9 |
| 分流电阻 | 10 mΩ，外接反相增益约 10 |

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
USER/src/main.c                 主循环: SguanFOC_main_Loop、电位器、按键
USER/src/interrupts_cw32l012.c  ATIM→电流采样+High_Loop；BTIM1→Low_Loop
Middlewares/SguanFOC/           SguanFOC v3.1.0 无感库 + UserData_* 适配
BSP/BSP_MOTOR.c                 ATIM 互补 PWM
BSP/BSP_Current.c               OPA + ADC1 电流原始值
BSP/BSP_Button.c / Potentiometer / UART / led
Libraries/                      CW32 标准外设库
```

### SguanFOC 三环任务

1. `SguanFOC_High_Loop()` — PWM 中断（10 kHz）：电流、SMO、电流环、SVPWM  
2. `SguanFOC_Low_Loop()` — 1 ms：状态机 / 保护  
3. `SguanFOC_main_Loop()` — 主循环：首次初始化、启动校准、JustFloat 发送  

配置入口：`Middlewares/SguanFOC/UserData_Config.h`（模式）、`UserData_Motor.h`（电机/采样）、`UserData_Parameter.h`（PI / 无感切换阈值）。

---

## 调参提示（无感）

| 参数 | 位置 | 说明 |
|------|------|------|
| `Target_IF_Iq` | `UserData_Motor.h` | IF 强拖电流，默认 0.45 A |
| `Sensorless_*` | `UserData_Parameter.h` | IF→SMO 机械角速度门槛（rad/s） |
| 电流环 Kp/Ki | `UserData_Parameter.h` | 按 Rs/Ls 粗调，默认按 8 Ω / 4.25 mH |
| `Current_Dir0/1` | `UserData_Motor.h` | 反相运放为 −1；若电流极性反了再改 |
| `Motor_Dir` | `UserData_Motor.h` | 相序反了改为 −1 |

串口（115200）：`MOTOR=1?` 启动，`MOTOR=0?` 停止，`Speed=60.0?` 给定机械 rad/s。

---

## 环境依赖

- `arm-none-eabi-gcc`
- CMake ≥ 3.22 + Ninja
- Python3 + **pyOCD**（烧录/调试）
- CMSIS Pack：`WHXY/CW32L012_DFP`

```bash
cmake --preset Debug
cmake --build --preset Debug
python3 flash_cw32.py   # 或按 README 原有 pyocd 流程
```

当前 Debug 镜像约占用 Flash ~52%、RAM ~45%（8 KB SRAM / 64 KB Flash）。

---

## 许可

- 本工程应用层：随仓库原许可
- `Middlewares/SguanFOC/`：MIT（见该目录 `LICENSE`），版权属 Sguan / ZhouQing
