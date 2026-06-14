#ifndef STM32F1XX_H
#define STM32F1XX_H

#include <stdint.h>

/* ── Cortex-M3 core peripherals ─────────────────────────────── */
#define SCB_BASE           0xE000E000UL
#define SCB               ((SCB_Type *)SCB_BASE)

#define SYSTICK_BASE       (SCB_BASE + 0x0010UL)
#define SYSTICK           ((SysTick_Type *)SYSTICK_BASE)

#define NVIC_BASE          0xE000E100UL
#define NVIC              ((NVIC_Type *)NVIC_BASE)
#define NVIC_SetPriority   __NVIC_SetPriority
#define NVIC_EnableIRQ     __NVIC_EnableIRQ

/* ── STM32F1 peripheral memory map ──────────────────────────── */
#define PERIPH_BASE        0x40000000UL
#define APB1_PERIPH_BASE   PERIPH_BASE
#define APB2_PERIPH_BASE   (PERIPH_BASE + 0x00010000UL)
#define AHBPERIPH_BASE     (PERIPH_BASE + 0x00018000UL)

#define RCC_BASE           (AHBPERIPH_BASE  + 0x00009000UL)
#define RCC               ((RCC_Type *)RCC_BASE)

#define FLASH_BASE         0x40022000UL
#define FLASH             ((FLASH_Type *)FLASH_BASE)

#define GPIOA_BASE         (APB2_PERIPH_BASE + 0x00000800UL)
#define GPIOB_BASE         (APB2_PERIPH_BASE + 0x00000C00UL)
#define GPIOC_BASE         (APB2_PERIPH_BASE + 0x00001000UL)
#define GPIOA             ((GPIO_Type *)GPIOA_BASE)
#define GPIOB             ((GPIO_Type *)GPIOB_BASE)
#define GPIOC             ((GPIO_Type *)GPIOC_BASE)

/* ── SCB ────────────────────────────────────────────────────── */
typedef struct {
    volatile uint32_t CPUID;
    volatile uint32_t ICSR;
    volatile uint32_t VTOR;
    volatile uint32_t AIRCR;
    volatile uint32_t SCR;
    volatile uint32_t CCR;
    volatile uint32_t SHPR[3];
    volatile uint32_t SHCSR;
    volatile uint32_t CFSR;
    volatile uint32_t HFSR;
    volatile uint32_t DFSR;
    volatile uint32_t MMFAR;
    volatile uint32_t BFAR;
    volatile uint32_t AFSR;
} SCB_Type;

#define SCB_VTOR_TBLOFF_Msk  0x1FFFFF

/* ── SysTick ─────────────────────────────────────────────────── */
typedef struct {
    volatile uint32_t CTRL;
    volatile uint32_t LOAD;
    volatile uint32_t VAL;
    volatile uint32_t CALIB;
} SysTick_Type;

#define SYSTICK_CTRL_ENABLE    (1 << 0)
#define SYSTICK_CTRL_TICKINT   (1 << 1)
#define SYSTICK_CTRL_CLKSOURCE (1 << 2)
#define SYSTICK_CTRL_COUNTFLAG (1 << 16)

/* ── NVIC ────────────────────────────────────────────────────── */
typedef struct {
    volatile uint32_t ISER[8];
    volatile uint32_t RESERVED0[24];
    volatile uint32_t ICER[8];
    volatile uint32_t RESERVED1[24];
    volatile uint32_t ISPR[8];
    volatile uint32_t RESERVED2[24];
    volatile uint32_t ICPR[8];
    volatile uint32_t RESERVED3[24];
    volatile uint32_t IABR[8];
    volatile uint32_t RESERVED4[56];
    volatile uint8_t  IP[240];
} NVIC_Type;

static inline void __NVIC_SetPriority(int32_t IRQn, uint32_t priority)   { if (IRQn >= 0) NVIC->IP[IRQn] = (uint8_t)((priority << 4) & 0xFF); }
static inline void __NVIC_EnableIRQ(int32_t IRQn)                        { if (IRQn >= 0) NVIC->ISER[IRQn >> 5] = (1U << (IRQn & 0x1F)); }

/* ── RCC ─────────────────────────────────────────────────────── */
typedef struct {
    volatile uint32_t CR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    volatile uint32_t APB2RSTR;
    volatile uint32_t APB1RSTR;
    volatile uint32_t AHBENR;
    volatile uint32_t APB2ENR;
    volatile uint32_t APB1ENR;
    volatile uint32_t BDCR;
    volatile uint32_t CSR;
} RCC_Type;

#define RCC_CR_HSION      (1 <<  0)
#define RCC_CR_HSIRDY     (1 <<  1)
#define RCC_CR_HSEON      (1 << 16)
#define RCC_CR_HSERDY     (1 << 17)
#define RCC_CR_PLLON      (1 << 24)
#define RCC_CR_PLLRDY     (1 << 25)

#define RCC_CFGR_SW_HSI   0x00
#define RCC_CFGR_SW_HSE   0x01
#define RCC_CFGR_SW_PLL   0x02
#define RCC_CFGR_SWS_HSI  0x00
#define RCC_CFGR_SWS_HSE  0x04
#define RCC_CFGR_SWS_PLL  0x08

#define RCC_APB2ENR_IOPCEN  (1 << 4)
#define RCC_APB2ENR_IOPBEN  (1 << 3)
#define RCC_APB2ENR_IOPAEN  (1 << 2)

#define RCC_APB1ENR_I2C1EN  (1 << 21)

/* ── FLASH ──────────────────────────────────────────────────── */
typedef struct {
    volatile uint32_t ACR;
    volatile uint32_t KEYR;
    volatile uint32_t OPTKEYR;
    volatile uint32_t SR;
    volatile uint32_t CR;
    volatile uint32_t AR;
    volatile uint32_t RESERVED;
    volatile uint32_t OBR;
    volatile uint32_t WRPR;
} FLASH_Type;

#define FLASH_ACR_LATENCY_Msk 0x07

/* ── I2C ─────────────────────────────────────────────────────── */
#define I2C1_BASE          (APB1_PERIPH_BASE  + 0x00005400UL)
#define I2C1              ((I2C_Type *)I2C1_BASE)

typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t OAR1;
    volatile uint32_t OAR2;
    volatile uint32_t DR;
    volatile uint32_t SR1;
    volatile uint32_t SR2;
    volatile uint32_t CCR;
    volatile uint32_t TRISE;
} I2C_Type;

/* I2C CR1 */
#define I2C_CR1_PE      (1 << 0)
#define I2C_CR1_START   (1 << 8)
#define I2C_CR1_STOP    (1 << 9)
#define I2C_CR1_ACK     (1 << 10)

/* I2C SR1 */
#define I2C_SR1_SB       (1 << 0)
#define I2C_SR1_ADDR     (1 << 1)
#define I2C_SR1_BTF      (1 << 2)
#define I2C_SR1_RXNE     (1 << 6)
#define I2C_SR1_TXE      (1 << 7)

/* I2C SR2 */
#define I2C_SR2_MSL      (1 << 0)
#define I2C_SR2_BUSY     (1 << 1)
#define I2C_SR2_TRA      (1 << 2)

/* ── GPIO ────────────────────────────────────────────────────── */
typedef struct {
    volatile uint32_t CRL;
    volatile uint32_t CRH;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t BRR;
    volatile uint32_t LCKR;
} GPIO_Type;

/* GPIO pin mode: CNF[1:0] + MODE[1:0] */
#define GPIO_OUT_PP_2MHZ   0x02  /* Output push-pull, 2 MHz */
#define GPIO_OUT_PP_10MHZ  0x01  /* Output push-pull, 10 MHz */
#define GPIO_OUT_PP_50MHZ  0x03  /* Output push-pull, 50 MHz */
#define GPIO_IN_ANALOG     0x00
#define GPIO_IN_FLOATING   0x04
#define GPIO_IN_PUPD       0x08

/* Macros to pack per-pin CNF+MODE into CRL/CRH */
#define PIN_MODE_OUT(pin, speed)  ((speed) << ((pin) * 4))

/* BSRR bits: low 16 bits = set, high 16 bits = reset */
#define GPIO_BSRR_BS(n)   (1U << (n))
#define GPIO_BSRR_BR(n)   (1U << ((n) + 16))

/* ── Core helpers ────────────────────────────────────────────── */
void SystemInit(void);

#define UNUSED(x)  ((void)(x))

static inline void __enable_irq(void)  { __asm volatile ("cpsie i" : : : "memory"); }
static inline void __disable_irq(void) { __asm volatile ("cpsid i" : : : "memory"); }

/* Data synchronization barrier */
static inline void __DSB(void)         { __asm volatile ("dsb 0xF" : : : "memory"); }

#endif /* STM32F1XX_H */
