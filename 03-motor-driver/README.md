# STM32F103 Motor Driver — TB6612 PWM Control

Bare-metal STM32F103C8T6 firmware for learning motor driver debugging with a TB6612 dual H-bridge driver and SSD1306 OLED display.

## Hardware

| Component | Model |
|-----------|-------|
| MCU | STM32F103C8T6 (Blue Pill) |
| Motor Driver | TB6612FNG |
| Display | 0.96" SSD1306 I2C OLED |
| Debug Probe | ST-Link V2 |

## Pin Mapping

### TB6612 Motor Driver (Motor B channel)

| TB6612 | STM32 Pin | Function |
|--------|-----------|----------|
| PWMB | PA1 (TIM2_CH2) | PWM speed, 20kHz |
| BIN1 | PA4 | Direction |
| BIN2 | PA5 | Direction |
| STBY | PA6 | Enable (HIGH = on) |
| BO1/BO2 | Motor wires | H-bridge output |

### SSD1306 OLED (I2C remapped)

| OLED | STM32 Pin | Function |
|------|-----------|----------|
| SCL | PB8 | I2C1 SCL (remapped) |
| SDA | PB9 | I2C1 SDA (remapped) |
| VCC | PB7 | Powered by GPIO |
| GND | PB6 | Powered by GPIO |

## State Machine

4-state demo cycle, 3 seconds per state, fixed 50% duty:

```
FORWARD → COAST → REVERSE → BRAKE → (loop)
```

| State | IN1 | IN2 | PWM | Motor Behavior |
|-------|-----|-----|-----|----------------|
| FORWARD | HI | LO | 50% | Spins forward |
| COAST | — | — | 0% | Free-wheeling |
| REVERSE | LO | HI | 50% | Spins backward |
| BRAKE | LO | LO | 100% | Short brake (stops hard) |

OLED displays: state name, countdown, duty cycle, and live AIN1/AIN2 pin levels read from GPIO IDR.

## Build & Flash

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-none-eabi.cmake
make -j$(sysctl -n hw.ncpu)
st-flash --reset write f103-motor-driver.bin 0x08000000
```

Or use VSCode: `Cmd+Shift+B` to build, `F5` to debug with Cortex-Debug + OpenOCD.

## Debugging Notes

This project was built specifically for practicing motor driver debugging:

1. **Register-level code** — no HAL, all peripheral registers written directly. Read `stm32f1xx.h` to understand the TIM, GPIO, and RCC register maps.
2. **OLED live feedback** — pin states are read from `GPIOA->IDR` and displayed in real time, letting you verify software intent matches hardware reality.
3. **H-bridge failure diagnosis** — the original Motor A channel turned out to have a blown reverse FET. Diagnosed by measuring AO1-AO2 voltage: ~4V in forward, 0V in reverse. Switched to Motor B channel as workaround. This is exactly the kind of real-world hardware fault you debug with a multimeter and systematic signal tracing.
4. **Breakpoint practice** — set breakpoints in `motor_set()` to watch direction pin toggles and `TIM2->CCR2` register changes in real time.
