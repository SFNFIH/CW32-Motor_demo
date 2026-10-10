# CW32L012 霍尔六步方波（HALLTIM + ATIM）

基于 **CW32L012C8** 的三相无刷 **有感六步** 控制。  
换相完全在 **HALLTIM 中断** 中完成；主循环只根据电位器更新占空比。

电机：**4006 KV380**（14 极 / 7 极对）。

---

## 功能概览

| 项目 | 说明 |
|------|------|
| 换相 | `HALLTIM` 霍尔边沿中断 → 读 HA/HB/HC → `ATIM` 六步换相 |
| 驱动 | ATIM 三相互补 PWM，20 kHz，带死区；浮空相关断 CH/CHN |
| 占空比 | 主循环电位器 → 约 8%～70% |
| 操作 | **单击**启停，**双击**换向 |
| 通信 | UART1 文本状态（115200） |

---

## 硬件连接

### 电机 PWM（ATIM）

| 信号 | 引脚 |
|------|------|
| UH / UL | PB5 / PA15 |
| VH / VL | PB6 / PB3 |
| WH / WL | PB7 / PB4 |

### 霍尔（HALLTIM）

| 信号 | 引脚 |
|------|------|
| HA | PB13 → HALLTIM_CH1 |
| HB | PB14 → HALLTIM_CH2 |
| HC | PB15 → HALLTIM_CH3 |

### 人机

| 功能 | 引脚 |
|------|------|
| 按键 | PB10 |
| 电位器 | PA9 → ADC2 CH6 |
| UART1 | PB12 TX / PB11 RX |
| LED | PC13 |

---

## 软件架构

```
USER/src/main.c                 主循环: 只改占空比 + 启停/换向请求
USER/src/interrupts_cw32l012.c  BTIM3_HALLTIM → BSP_HallBLDC_IRQHandler
BSP/BSP_HallBLDC.c              HALLTIM 初始化、霍尔表、中断换相
BSP/BSP_MOTOR.c                 ATIM 互补 PWM + CommutateStep 六步
```

六步序号：`0:A+B- 1:A+C- 2:B+C- 3:B+A- 4:C+A- 5:C+B-`  
若转向与预期相反：双击换向，或改 `BSP_HallBLDC.c` 中霍尔→步序表。

---

## 构建

```bash
cmake --preset Debug
cmake --build --preset Debug
```
