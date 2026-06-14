#include "stm32f1xx.h"
#include "ssd1306.h"

/* ── Pin mapping ────────────────────────────────────────────── */
/* Motor A */
#define AIN1_PIN        4       /* PA4 = BIN1 */
#define AIN2_PIN        5       /* PA5 = BIN2 */
#define PWM_PIN         1       /* PA1 = TIM2_CH2 = PWMB */

/* Standby (both motors) */
#define STBY_PIN        6       /* PA6 */

/* OLED power */
#define OLED_GND_PIN    6       /* PB6 */
#define OLED_VCC_PIN    7       /* PB7 */

/* ── PWM constants ──────────────────────────────────────────── */
/* SystemInit: HSI/2 × PLL×16 = 64MHz, APB1 = 32MHz.
   APB1 prescaler = /2 (≠1), so TIM2_CLK = APB1 × 2 = 64MHz. */
#define TIM2_CLK        64000000UL
#define PWM_FREQ        20000UL     /* 20 kHz, above audible */
#define PWM_ARR         ((TIM2_CLK) / (PWM_FREQ) - 1)   /* 3599 */
#define PWM_MAX         (PWM_ARR + 1)                   /* 3600 */

/* ── Global state ───────────────────────────────────────────── */
static volatile uint32_t g_ticks = 0;
void SysTick_Handler(void) { g_ticks++; }

static void delay_ms(uint32_t ms) {
    uint32_t start = g_ticks;
    while ((g_ticks - start) < ms);
}

/* ── Motor direction ────────────────────────────────────────── */
typedef enum {
    MOTOR_COAST,    /* STBY=0 or both IN low → free spin */
    MOTOR_CW,       /* IN1=H, IN2=L */
    MOTOR_CCW,      /* IN1=L, IN2=H */
    MOTOR_BRAKE     /* IN1=L, IN2=L with STBY=1 → short brake */
} motor_dir_t;

/* ── OLED I2C (same as f103-oled-test) ──────────────────────── */
static void i2c_init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

    /* PB6/7 as GPIO output to power OLED module */
    GPIOB->CRL &= ~((uint32_t)0xF << (OLED_GND_PIN * 4));
    GPIOB->CRL |=  ((uint32_t)GPIO_OUT_PP_2MHZ << (OLED_GND_PIN * 4));
    GPIOB->BSRR = GPIO_BSRR_BR(OLED_GND_PIN);

    GPIOB->CRL &= ~((uint32_t)0xF << (OLED_VCC_PIN * 4));
    GPIOB->CRL |=  ((uint32_t)GPIO_OUT_PP_2MHZ << (OLED_VCC_PIN * 4));
    GPIOB->BSRR = GPIO_BSRR_BS(OLED_VCC_PIN);

    /* PB8/9 = I2C1 remapped (SCL/SDA), AF open-drain 50MHz */
    GPIOB->CRH &= ~((uint32_t)0xFF << 0);
    GPIOB->CRH |=  ((uint32_t)0xFF << 0);

    /* Remap I2C1 to PB8(SCL)/PB9(SDA) */
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    AFIO->MAPR   |= AFIO_MAPR_I2C1_REMAP;

    /* I2C1: 100kHz @ 8MHz peripheral clock */
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    I2C1->CR2   = 8;        /* FREQ = 8 MHz */
    I2C1->CCR   = 40;       /* 100kHz standard mode */
    I2C1->TRISE = 9;
    I2C1->CR1   = I2C_CR1_PE;
}

/* ── GPIO init for motor ────────────────────────────────────── */
static void motor_gpio_init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

    /* PA1 (PWMB) = AF push-pull 50MHz for TIM2_CH2 */
    GPIOA->CRL &= ~((uint32_t)0xF << (PWM_PIN * 4));
    GPIOA->CRL |=  ((uint32_t)GPIO_AF_PP_50MHZ << (PWM_PIN * 4));

    /* PA4..PA6 = GPIO push-pull outputs for direction + STBY */
    for (uint8_t p = 4; p <= 6; p++) {
        GPIOA->CRL &= ~((uint32_t)0xF << (p * 4));
        GPIOA->CRL |=  ((uint32_t)GPIO_OUT_PP_50MHZ << (p * 4));
    }

    /* Start with STBY low (motor off) and all IN pins low */
    GPIOA->BSRR = GPIO_BSRR_BR(STBY_PIN);
    GPIOA->BSRR = GPIO_BSRR_BR(AIN1_PIN) | GPIO_BSRR_BR(AIN2_PIN);
}

/* ── PWM init ───────────────────────────────────────────────── */
static void pwm_init(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    /* Prescaler 0 → counter clock = 72MHz */
    TIM2->PSC  = 0;
    TIM2->ARR  = PWM_ARR;           /* 3599 → 20kHz */
    TIM2->CNT  = 0;

    /* CH2 (PA1) = PWM mode 1, preload enabled */
    TIM2->CCMR1 |= TIM_CCMR1_OC2M_PWM1 | TIM_CCMR1_OC2PE;

    /* Enable CH2 output */
    TIM2->CCER = TIM_CCER_CC2E;

    /* Start with 0% duty */
    TIM2->CCR2 = 0;

    /* Auto-reload preload + enable counter */
    TIM2->CR1 = TIM_CR1_ARPE;
    TIM2->EGR = TIM_EGR_UG;
    TIM2->CR1 |= TIM_CR1_CEN;
}

/* ── Motor set helpers ──────────────────────────────────────── */
static void motor_set(motor_dir_t dir, uint16_t duty) {
    if (duty > PWM_MAX) duty = PWM_MAX;

    switch (dir) {
    case MOTOR_CW:
        GPIOA->BSRR = GPIO_BSRR_BS(AIN1_PIN) | GPIO_BSRR_BR(AIN2_PIN);
        TIM2->CCR2 = duty;
        break;
    case MOTOR_CCW:
        GPIOA->BSRR = GPIO_BSRR_BR(AIN1_PIN) | GPIO_BSRR_BS(AIN2_PIN);
        TIM2->CCR2 = duty;
        break;
    case MOTOR_BRAKE:
        GPIOA->BSRR = GPIO_BSRR_BR(AIN1_PIN) | GPIO_BSRR_BR(AIN2_PIN);
        TIM2->CCR2 = PWM_MAX;   /* Short brake: low-side FETs on */
        break;
    case MOTOR_COAST:
        TIM2->CCR2 = 0;         /* No PWM → floating */
        break;
    }
}

static void motor_enable(uint8_t en) {
    if (en) {
        GPIOA->BSRR = GPIO_BSRR_BS(STBY_PIN);
    } else {
        GPIOA->BSRR = GPIO_BSRR_BR(STBY_PIN);
    }
}

/* ── OLED helpers ───────────────────────────────────────────── */
static void draw_progress_bar(uint8_t x, uint8_t page, uint8_t width,
                              uint16_t duty) {
    uint8_t fill = (uint8_t)((uint32_t)duty * width / PWM_MAX);
    /* Draw border */
    for (uint8_t i = 0; i < width + 2; i++) {
        ssd1306_draw_pixel(x + i, page * 8 + 0, 1);
        ssd1306_draw_pixel(x + i, page * 8 + 8, 1);
    }
    ssd1306_draw_pixel(x,           page * 8 + 1, 1);
    ssd1306_draw_pixel(x,           page * 8 + 7, 1);
    ssd1306_draw_pixel(x + width + 1, page * 8 + 1, 1);
    ssd1306_draw_pixel(x + width + 1, page * 8 + 7, 1);
    /* Fill bar */
    for (uint8_t i = 0; i < fill; i++)
        for (uint8_t r = 1; r < 8; r++)
            ssd1306_draw_pixel(x + 1 + i, page * 8 + r, 1);
}

/* ── Demo state machine ─────────────────────────────────────── */
typedef enum {
    STATE_FWD,
    STATE_COAST,
    STATE_REV,
    STATE_BRAKE,
} demo_state_t;

static const char *state_names[] = {
    "FORWARD", "COAST", "REVERSE", "BRAKE"
};

/* ── Entry ──────────────────────────────────────────────────── */
int main(void) {
    SystemInit();

    /* SysTick @ 1ms (64MHz / 64000 = 1kHz) */
    SYSTICK->LOAD = 63999;
    SYSTICK->VAL  = 0;
    SYSTICK->CTRL = SYSTICK_CTRL_ENABLE | SYSTICK_CTRL_TICKINT | SYSTICK_CTRL_CLKSOURCE;

    i2c_init();
    delay_ms(100);
    ssd1306_init();

    motor_gpio_init();
    pwm_init();
    motor_enable(1);

    /* ── Demo loop ──────────────────────────────────────── */
    uint32_t phase_start  = g_ticks;
    demo_state_t state    = STATE_FWD;
    uint16_t duty         = PWM_MAX * 50 / 100;   /* fixed 50% */

    while (1) {
        uint32_t elapsed = g_ticks - phase_start;

        switch (state) {
        case STATE_FWD:
            motor_set(MOTOR_CW, duty);
            if (elapsed >= 3000) {
                state = STATE_COAST;
                phase_start = g_ticks;
            }
            break;

        case STATE_COAST:
            motor_set(MOTOR_COAST, 0);
            if (elapsed >= 3000) {
                state = STATE_REV;
                phase_start = g_ticks;
            }
            break;

        case STATE_REV:
            motor_set(MOTOR_CCW, duty);
            if (elapsed >= 3000) {
                state = STATE_BRAKE;
                phase_start = g_ticks;
            }
            break;

        case STATE_BRAKE:
            motor_set(MOTOR_BRAKE, PWM_MAX);
            if (elapsed >= 3000) {
                state = STATE_FWD;
                phase_start = g_ticks;
            }
            break;
        }

        /* ── Refresh OLED every 100ms ────────────────────── */
        static uint32_t last_display = 0;
        if (g_ticks - last_display >= 100) {
            last_display = g_ticks;

            uint32_t e = g_ticks - phase_start;
            uint16_t remaining = (e < 3000) ? (uint16_t)((3000 - e) / 1000) : 0;

            ssd1306_clear();
            ssd1306_draw_string(0, 0, "Motor B");

            /* State */
            ssd1306_draw_string(0, 1, "St:");
            ssd1306_draw_string(18, 1, state_names[state]);

            /* Countdown */
            char buf[12];
            buf[0] = '0' + (char)remaining;
            buf[1] = 's';
            buf[2] = '\0';
            ssd1306_draw_string(0, 2, "Left:");
            ssd1306_draw_string(30, 2, buf);

            /* Duty */
            uint16_t pct = (uint16_t)((uint32_t)duty * 100 / PWM_MAX);
            buf[0] = (char)('0' + pct / 100);
            buf[1] = (char)('0' + (pct / 10) % 10);
            buf[2] = (char)('0' + pct % 10);
            buf[3] = '%';
            buf[4] = '\0';
            ssd1306_draw_string(0, 3, "Duty:");
            ssd1306_draw_string(30, 3, buf);

            /* Pin status: read PA2 (AIN1) and PA3 (AIN2) from IDR */
            uint8_t ain1 = (GPIOA->IDR >> AIN1_PIN) & 1;
            uint8_t ain2 = (GPIOA->IDR >> AIN2_PIN) & 1;
            ssd1306_draw_string(0, 4, "AIN1:");
            ssd1306_draw_string(30, 4, ain1 ? "HI" : "LO");
            ssd1306_draw_string(0, 5, "AIN2:");
            ssd1306_draw_string(30, 5, ain2 ? "HI" : "LO");

            /* Progress bar for state time */
            draw_progress_bar(0, 6, 126, (uint16_t)(elapsed * PWM_MAX / 3000));

            ssd1306_refresh();
        }
    }
}
