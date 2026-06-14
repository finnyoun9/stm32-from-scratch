# 02-oled-i2c — Bare-Metal I2C OLED Clock (SSD1306)

> **难度**：中级 | **日期**：2026-06-01 | **不含 HAL / CMSIS 库**

---

## 🎯 目标

用**纯寄存器操作**通过 I2C1 驱动 SSD1306 OLED (128x64)，显示编译时间并从该时间点开始走时。
完全自主编写 I2C 和 SSD1306 驱动，不依赖任何现成库。

---

## 🔌 硬件连接

| STM32 引脚 | SSD1306 OLED | 备注 |
|-----------|-------------|------|
| PB8 | SCL | I2C1 重映射后 |
| PB9 | SDA | I2C1 重映射后 |
| PB6 | GND | GPIO 输出低电平供电 |
| PB7 | VCC | GPIO 输出高电平供电 |

> 💡 直接用 GPIO 给 OLED 供电，省去面包板电源线。

---

## 🧠 做了什么

### 1. I2C 初始化（纯寄存器）
```c
// 使能 GPIOB + AFIO + I2C1 时钟
// 配置 PB8/PB9 为复用开漏输出
// 重映射 I2C1 到 PB8/PB9（默认在 PB6/PB7）
AFIO->MAPR |= AFIO_MAPR_I2C1_REMAP;

// I2C 时序计算（8MHz PCLK1 → 100kHz SCL）
I2C1->CR2 = 8;    // FREQ = 8MHz
I2C1->CCR = 40;   // CCR = 8MHz / (2 * 100kHz) = 40
I2C1->TRISE = 9;  // 1000ns / 125ns + 1 = 9
I2C1->CR1 = I2C_CR1_PE;  // 使能 I2C
```

### 2. SSD1306 驱动（自写）
- 完整 I2C 读写封装（起始/停止/地址/数据）
- SSD1306 初始化序列（升压、对比度、寻址模式）
- 大字体渲染（16x16 像素字符，用于 HH:MM:SS）
- 小字体渲染（6x8 像素字符，用于日期和星期）
- 逐像素绘制 API

### 3. 实时时钟
- 从编译时间 `__DATE__` / `__TIME__` 宏解析初始时刻
- SysTick 1ms 中断驱动秒级累计
- 日期进位（月/年边界 + 闰年判断）
- Zeller 公式计算星期

### 4. 显示布局
```
┌──────────────────────────────┐
│  HH:MM:SS        (大字体)     │
│──────────────────────────────│
│  YYYY-MM-DD      (小字体)     │
│                              │
│  Mon              (小字体)     │
└──────────────────────────────┘
```

---

## 🔨 构建 & 烧录

```bash
mkdir build && cd build
cmake ..
make

# 烧录
st-flash write main.bin 0x08000000
```

---

## 📚 涉及知识点

- [x] I2C 协议（寄存器级）— START/STOP/ACK/NACK
- [x] STM32 I2C1 外设配置（CR1/CR2/CCR/TRISE）
- [x] GPIO 复用功能 + AFIO 重映射
- [x] AFIO_MAPR 寄存器
- [x] SSD1306 命令/数据协议
- [x] OLED 像素寻址（页 + 列）
- [x] 自定义点阵字体渲染
- [x] 实时时钟算法（日期进位 + 闰年）
- [x] Zeller 公式（星期计算）
- [x] 字符解析（C 字符串处理）

---

## 📸 效果

[待拍照添加]

---

## ⏭ 下一步

→ 03-vl53l0x-tof — 连接 VL53L0X ToF 传感器，裸机 I2C 读取距离数据
