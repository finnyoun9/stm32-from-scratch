# 01-blink — Bare-Metal LED Blink (STM32F103C8T6)

> **难度**：入门 | **日期**：2026-05-31 | **不含 HAL / CMSIS 库**

---

## 🎯 目标

用**纯寄存器操作**控制 STM32F103C8T6 (Blue Pill) 板载 LED (PC13) 闪烁。
不依赖任何 HAL 库、标准外设库、或 RTOS。从零配置时钟、GPIO、SysTick。

---

## 🔌 硬件连接

| STM32 引脚 | 连接 |
|-----------|------|
| PC13 | 板载 LED（低电平亮） |
| ST-Link SWDIO | PA13 |
| ST-Link SWCLK | PA14 |

---

## 🧠 做了什么

### 1. 系统时钟初始化
`SystemInit()` — CMSIS 内置，配置 HSE 8MHz → PLL → 72MHz SYSCLK

### 2. GPIO 配置（寄存器级）
```c
// 使能 GPIOC 时钟（APB2 总线）
RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

// 配置 PC13 为推挽输出，2MHz
GPIOC->CRH &= ~((uint32_t)0xF << 20);   // 清除 CNF13[1:0] + MODE13[1:0]
GPIOC->CRH |= ((uint32_t)GPIO_OUT_PP_2MHZ << 20);
```

### 3. SysTick 定时器
- 8MHz HCLK，1ms tick = 8000 - 1 = 7999
- 中断服务函数 `SysTick_Handler()` 递增全局计数器
- `delay_ms()` 阻塞等待

### 4. LED 翻转
使用 `BSRR` 寄存器原子操作（避免 read-modify-write 竞态）：
```c
GPIOC->BSRR = GPIO_BSRR_BR(13);  // 复位 → LED 亮（低电平）
GPIOC->BSRR = GPIO_BSRR_BS(13);  // 置位 → LED 灭
```

---

## 🔨 构建 & 烧录

```bash
# 编译
mkdir build && cd build
cmake ..
make

# 烧录（ST-Link）
st-flash write main.bin 0x08000000

# 或 OpenOCD 烧录
openocd -f openocd.cfg -c "program main.elf verify reset exit"
```

---

## 📚 涉及知识点

- [x] ARM Cortex-M3 寄存器映射
- [x] RCC 时钟树（APB2 外设时钟使能）
- [x] GPIO CRL/CRH 配置寄存器
- [x] BSRR 原子位操作
- [x] SysTick 定时器 + 中断
- [x] 链接脚本（ld/）
- [x] 启动文件（向量表）
- [x] CMake + arm-none-eabi-gcc 工具链

---

## ⏭ 下一步

→ [02-oled-i2c](../02-oled-i2c/) — 用 I2C 驱动 SSD1306 OLED 显示屏，显示实时时钟
