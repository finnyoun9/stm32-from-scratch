# STM32 From Scratch — 寄存器级裸机编程

> 🔵 STM32F103C8T6 (Blue Pill) 从零开始：不依赖 HAL 库，纯寄存器操作
> 
> 工具链：arm-none-eabi-gcc + CMake + OpenOCD + ST-Link（纯命令行）

---

## 📂 项目

| # | 项目 | 内容 | 知识点 |
|---|------|------|--------|
| 01 | [blink](01-blink/) | GPIO + SysTick 定时器 | 寄存器编程、时钟树 |
| 02 | [oled-i2c](02-oled-i2c/) | SSD1306 OLED I2C 驱动 | I2C 协议、外设驱动 |
| 03 | [motor-driver](03-motor-driver/) | 直流电机驱动 | PWM、H 桥控制 |
| 04 | [bldc](04-bldc/) | BLDC 六步换向 | 无刷电机控制 |
| 05 | [freertos](05-freertos/) | FreeRTOS 多任务调度 | 任务/IPC/中断管理 |

## 🔧 开发环境

```bash
# macOS (已配置)
arm-none-eabi-gcc    # 交叉编译（16.1.0）
cmake                # 构建系统
openocd              # 调试服务器
st-link              # 烧录工具

# 编译 & 烧录
cd 01-blink
cmake -B build && cmake --build build
openocd -f openocd.cfg
```

## 📖 学习笔记

本仓库只放代码。学习笔记和路线规划在 [embedded-notes](https://github.com/finnyoun9/embedded-notes)。

## 🔗 相关仓库

- 📚 [embedded-notes](https://github.com/finnyoun9/embedded-notes)
- 🔧 [hardware-lab](https://github.com/finnyoun9/hardware-lab) — PCB 设计
- 🤖 [robot-vacuum-lab](https://github.com/finnyoun9/robot-vacuum-lab)
