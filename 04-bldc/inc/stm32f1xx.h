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

#define AFIO_BASE          (APB2_PERIPH_BASE + 0x00000000UL)
#define AFIO              ((AFIO_Type *)AFIO_BASE)

#define TIM1_BASE          (APB2_PERIPH_BASE + 0x00002C00UL)
#define TIM1              ((TIM_Type *)TIM1_BASE)
#define TIM2_BASE          (APB1_PERIPH_BASE + 0x00000000UL)
#define TIM2              ((TIM_Type *)TIM2_BASE)

#define USART1_BASE        (APB2_PERIPH_BASE + 0x00003800UL)
#define USART1            ((USART_Type *)USART1_BASE)

#define I2C1_BASE          (APB1_PERIPH_BASE + 0x00005400UL)
#define I2C1              ((I2C_Type *)I2C1_BASE)

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

#define RCC_AHBENR_DMA1EN  (1 << 0)

#define RCC_APB2ENR_AFIOEN   (1 << 0)
#define RCC_APB2ENR_IOPAEN   (1 << 2)
#define RCC_APB2ENR_IOPBEN   (1 << 3)
#define RCC_APB2ENR_IOPCEN   (1 << 4)
#define RCC_APB2ENR_USART1EN (1 << 14)
#define RCC_APB2ENR_TIM1EN   (1 << 11)

#define RCC_APB1ENR_TIM2EN   (1 << 0)
#define RCC_APB1ENR_I2C1EN   (1 << 21)

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

/* ── AFIO ───────────────────────────────────────────────────── */
typedef struct {
    volatile uint32_t EVCR;
    volatile uint32_t MAPR;
    volatile uint32_t EXTICR[4];
    volatile uint32_t MAPR2;
} AFIO_Type;

#define AFIO_MAPR_I2C1_REMAP  (1 << 1)

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

/* GPIO pin mode: CNF[1:0] + MODE[1:0], each 4 bits in CRL/CRH */
#define GPIO_OUT_PP_2MHZ   0x02
#define GPIO_OUT_PP_10MHZ  0x01
#define GPIO_OUT_PP_50MHZ  0x03
#define GPIO_IN_ANALOG     0x00
#define GPIO_IN_FLOATING   0x04
#define GPIO_IN_PUPD       0x08
#define GPIO_AF_PP_2MHZ    0x0A
#define GPIO_AF_PP_10MHZ   0x09
#define GPIO_AF_PP_50MHZ   0x0B

/* BSRR bits: low 16 bits = set, high 16 bits = reset */
#define GPIO_BSRR_BS(n)   (1U << (n))
#define GPIO_BSRR_BR(n)   (1U << ((n) + 16))

/* ── TIM ─────────────────────────────────────────────────────── */
typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SMCR;
    volatile uint32_t DIER;
    volatile uint32_t SR;
    volatile uint32_t EGR;
    volatile uint32_t CCMR1;
    volatile uint32_t CCMR2;
    volatile uint32_t CCER;
    volatile uint32_t CNT;
    volatile uint32_t PSC;
    volatile uint32_t ARR;
    volatile uint32_t RCR;       /* TIM1 only */
    volatile uint32_t CCR1;
    volatile uint32_t CCR2;
    volatile uint32_t CCR3;
    volatile uint32_t CCR4;
    volatile uint32_t BDTR;      /* TIM1 only */
    volatile uint32_t DCR;
    volatile uint32_t DMAR;
} TIM_Type;

/* TIM_CR1 */
#define TIM_CR1_CEN    (1 << 0)
#define TIM_CR1_ARPE   (1 << 7)

/* TIM_CCMR1 output compare mode */
#define TIM_CCMR1_OC1M_PWM1  (0x6 << 4)
#define TIM_CCMR1_OC1M_PWM2  (0x7 << 4)
#define TIM_CCMR1_OC1PE      (1 << 3)
#define TIM_CCMR1_OC2M_PWM1  (0x6 << 12)
#define TIM_CCMR1_OC2M_PWM2  (0x7 << 12)
#define TIM_CCMR1_OC2PE      (1 << 11)

/* TIM_CCMR2 output compare mode */
#define TIM_CCMR2_OC3M_PWM1  (0x6 << 4)
#define TIM_CCMR2_OC3PE      (1 << 3)
#define TIM_CCMR2_OC4M_PWM1  (0x6 << 12)
#define TIM_CCMR2_OC4PE      (1 << 11)

/* TIM_CCER */
#define TIM_CCER_CC1E   (1 << 0)
#define TIM_CCER_CC1P   (1 << 1)
#define TIM_CCER_CC2E   (1 << 4)
#define TIM_CCER_CC2P   (1 << 5)
#define TIM_CCER_CC3E   (1 << 8)
#define TIM_CCER_CC4E   (1 << 12)

/* TIM_EGR */
#define TIM_EGR_UG      (1 << 0)

/* TIM_SR */
#define TIM_SR_UIF      (1 << 0)

/* ── USART ───────────────────────────────────────────────────── */
typedef struct {
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t BRR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t GTPR;
} USART_Type;

#define USART_SR_TXE    (1 << 7)
#define USART_SR_RXNE   (1 << 5)

#define USART_CR1_UE    (1 << 13)
#define USART_CR1_TE    (1 << 3)
#define USART_CR1_RE    (1 << 2)

/* ── I2C ─────────────────────────────────────────────────────── */
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

#define I2C_CR1_PE      (1 << 0)
#define I2C_CR1_START   (1 << 8)
#define I2C_CR1_STOP    (1 << 9)
#define I2C_CR1_ACK     (1 << 10)

#define I2C_SR1_SB      (1 << 0)
#define I2C_SR1_ADDR    (1 << 1)
#define I2C_SR1_BTF     (1 << 2)
#define I2C_SR1_RXNE    (1 << 6)
#define I2C_SR1_TXE     (1 << 7)

#define I2C_SR2_MSL     (1 << 0)
#define I2C_SR2_BUSY    (1 << 1)
#define I2C_SR2_TRA     (1 << 2)

/* ── Core helpers ────────────────────────────────────────────── */
void SystemInit(void);

#define UNUSED(x)  ((void)(x))

static inline void __enable_irq(void)  { __asm volatile ("cpsie i" : : : "memory"); }
static inline void __disable_irq(void) { __asm volatile ("cpsid i" : : : "memory"); }
static inline void __DSB(void)         { __asm volatile ("dsb 0xF" : : : "memory"); }

#endif /* STM32F1XX_H */
