# FreeRTOS 多任务实战项目 — STM32F103C8T6

> **面试亮点**: 深圳嵌入式岗位 FreeRTOS 必考技能实战  
> **硬件**: STM32F103C8T6 (Blue Pill), ST-Link V2  
> **编译**: `pio run`  
> **烧录**: `pio run --target upload`  
> **串口**: `pio device monitor`

## 项目结构

```
freertos-demo/
├── platformio.ini          # PlatformIO 构建配置
├── README.md               # 本文件
├── inc/
│   └── FreeRTOSConfig.h    # FreeRTOS 内核配置
└── src/
    └── main.c              # 主程序（3 任务 + 队列 + 定时器 + 信号量）
```

## 演示的 FreeRTOS 特性

| 特性 | 代码位置 | 面试考频 |
|------|----------|---------|
| **任务创建** (xTaskCreate) | 3 个不同优先级的任务 | ⭐⭐⭐⭐⭐ |
| **队列** (Queue) | Sensor → Monitor 数据传递 | ⭐⭐⭐⭐ |
| **二值信号量** (Semaphore) | Tick ISR → LED Task 同步 | ⭐⭐⭐⭐⭐ |
| **软件定时器** (Timer) | 看门狗心跳 | ⭐⭐⭐ |
| **精确周期** (vTaskDelayUntil) | LED/Sensor 任务 | ⭐⭐⭐⭐ |
| **栈溢出检测** (Stack High Water Mark) | Monitor 任务 | ⭐⭐⭐ |
| **Tick Hook** | 模拟按键中断 | ⭐⭐⭐ |
| **Malloc Failed Hook** | 内存失败保护 | ⭐⭐ |

## 任务优先级设计

```
优先级 3 (最高): Monitor  — 数据消费者（1s 周期，串口输出状态）
优先级 2:        Sensor   — 数据生产者（100ms 周期，模拟 ADC 采集）
优先级 1 (最低): LED      — 输出控制（200ms 正常 / 50ms 快闪）
```

这是嵌入式面试的经典模式：**高优先级消费、低优先级生产**。

## 串口输出示例

```
========================================
  FreeRTOS Multi-Task Demo (STM32F103)
  By Finn — 嵌入式工程师转岗项目
========================================

[MONITOR] Heap free: 7234 | Task hi-water: 98 | Queue depth: 9 | Last sensor: 4089
[WATCHDOG] Heartbeat #5
[MONITOR] Heap free: 7132 | Task hi-water: 98 | Queue depth: 10 | Last sensor: 4095
[LED] Fast blink mode ON           ← Tick Hook 触发，模拟按键
```

## 下一步

1. 连接 STM32F103C8T6 + ST-Link，运行 `pio run --target upload`
2. 打开串口 `pio device monitor` 看输出
3. 修改计划：把 vSensorTask 里的模拟数据换成实际 VL53L1X ToF 读数
4. 增加实际按键中断（EXTI）替代 Tick Hook 模拟
