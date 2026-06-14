#include "stm32f1xx.h"
#include "ssd1306.h"

/* ── BLDC control pin mapping ────────────────────────────────── */
/* All open-drain: ODR=0 pulls to GND (active), ODR=1 floats (inactive) */
#define REV_PIN     4   /* PA4 = Reverse */
#define BRK_PIN     5   /* PA5 = Brake */
#define STOP_PIN    6   /* PA6 = Stop */

/* OLED power */
#define OLED_GND_PIN  6   /* PB6 */
#define OLED_VCC_PIN  7   /* PB7 */

#define GPIO_OUT_OD_50MHZ  0x05   /* CNF=01 open-drain, MODE=11 50MHz */

/* ── Global state ───────────────────────────────────────────── */
static volatile uint32_t g_ticks = 0;
void SysTick_Handler(void) { g_ticks++; }

static void delay_ms(uint32_t ms) {
    uint32_t start = g_ticks;
    while ((g_ticks - start) < ms);
}

/* ── OLED I2C ───────────────────────────────────────────────── */
static void i2c_init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

    GPIOB->CRL &= ~((uint32_t)0xF << (OLED_GND_PIN * 4));
    GPIOB->CRL |=  ((uint32_t)GPIO_OUT_PP_2MHZ << (OLED_GND_PIN * 4));
    GPIOB->BSRR = GPIO_BSRR_BR(OLED_GND_PIN);

    GPIOB->CRL &= ~((uint32_t)0xF << (OLED_VCC_PIN * 4));
    GPIOB->CRL |=  ((uint32_t)GPIO_OUT_PP_2MHZ << (OLED_VCC_PIN * 4));
    GPIOB->BSRR = GPIO_BSRR_BS(OLED_VCC_PIN);

    GPIOB->CRH &= ~((uint32_t)0xFF << 0);
    GPIOB->CRH |=  ((uint32_t)0xFF << 0);

    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    AFIO->MAPR   |= AFIO_MAPR_I2C1_REMAP;

    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    I2C1->CR2   = 8;
    I2C1->CCR   = 40;
    I2C1->TRISE = 9;
    I2C1->CR1   = I2C_CR1_PE;
}

/* ── BLDC control helpers ───────────────────────────────────── */
static void bldc_init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

    /* PA4, PA5, PA6 = open-drain output for active-low control */
    for (uint8_t p = REV_PIN; p <= STOP_PIN; p++) {
        GPIOA->CRL &= ~((uint32_t)0xF << (p * 4));
        GPIOA->CRL |=  ((uint32_t)GPIO_OUT_OD_50MHZ << (p * 4));
    }

    /* All pins float → not active → motor runs forward */
    GPIOA->BSRR = GPIO_BSRR_BS(REV_PIN) | GPIO_BSRR_BS(BRK_PIN) | GPIO_BSRR_BS(STOP_PIN);
}

static void bldc_reverse(uint8_t on) {
    if (on) GPIOA->BRR = (1U << REV_PIN);
    else    GPIOA->BSRR = GPIO_BSRR_BS(REV_PIN);
}

static void bldc_brake(uint8_t on) {
    if (on) GPIOA->BRR = (1U << BRK_PIN);
    else    GPIOA->BSRR = GPIO_BSRR_BS(BRK_PIN);
}

static void bldc_stop(uint8_t on) {
    if (on) GPIOA->BRR = (1U << STOP_PIN);
    else    GPIOA->BSRR = GPIO_BSRR_BS(STOP_PIN);
}

/* ── Display helpers ────────────────────────────────────────── */
static void draw_progress_bar(uint8_t x, uint8_t page, uint8_t w, uint16_t val, uint16_t max) {
    uint8_t fill = (uint8_t)((uint32_t)val * w / max);
    for (uint8_t i = 0; i < w + 2; i++) {
        ssd1306_draw_pixel(x + i, page * 8 + 0, 1);
        ssd1306_draw_pixel(x + i, page * 8 + 8, 1);
    }
    ssd1306_draw_pixel(x, page * 8 + 1, 1);
    ssd1306_draw_pixel(x, page * 8 + 7, 1);
    ssd1306_draw_pixel(x + w + 1, page * 8 + 1, 1);
    ssd1306_draw_pixel(x + w + 1, page * 8 + 7, 1);
    for (uint8_t i = 0; i < fill; i++)
        for (uint8_t r = 1; r < 8; r++)
            ssd1306_draw_pixel(x + 1 + i, page * 8 + r, 1);
}

/* ── State machine ──────────────────────────────────────────── */
typedef enum { ST_FWD, ST_BRAKE, ST_REV, ST_STOP } state_t;
static const char *state_names[] = {"FORWARD", "BRAKE", "REVERSE", "STOP"};

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

    bldc_init();

    /* ── Demo loop ──────────────────────────────────────── */
    uint32_t phase_start = g_ticks;
    state_t state = ST_FWD;

    while (1) {
        uint32_t elapsed = g_ticks - phase_start;

        switch (state) {
        case ST_FWD:
            bldc_reverse(0); bldc_brake(0); bldc_stop(0);
            if (elapsed >= 3000) { state = ST_BRAKE; phase_start = g_ticks; }
            break;
        case ST_BRAKE:
            bldc_reverse(0); bldc_brake(1); bldc_stop(0);
            if (elapsed >= 1500) { state = ST_REV; phase_start = g_ticks; }
            break;
        case ST_REV:
            bldc_reverse(1); bldc_brake(0); bldc_stop(0);
            if (elapsed >= 3000) { state = ST_STOP; phase_start = g_ticks; }
            break;
        case ST_STOP:
            bldc_reverse(0); bldc_brake(0); bldc_stop(1);
            if (elapsed >= 2000) { state = ST_FWD; phase_start = g_ticks; }
            break;
        }

        /* ── OLED refresh every 100ms ──────────────────── */
        static uint32_t last_display = 0;
        if (g_ticks - last_display >= 100) {
            last_display = g_ticks;

            uint32_t timeouts[] = {3000, 1500, 3000, 2000};
            uint32_t to = timeouts[state];
            uint16_t remaining = (elapsed < to) ? (uint16_t)((to - elapsed) / 1000) : 0;

            /* Read actual pin states via IDR */
            uint8_t rev  = (GPIOA->IDR >> REV_PIN)  & 1;
            uint8_t brk  = (GPIOA->IDR >> BRK_PIN)  & 1;
            uint8_t stop = (GPIOA->IDR >> STOP_PIN) & 1;

            ssd1306_clear();
            ssd1306_draw_string(0, 0, "BLDC DRIVER");

            char buf[14];
            ssd1306_draw_string(0, 1, "St:");
            ssd1306_draw_string(18, 1, state_names[state]);

            buf[0] = '0' + (char)remaining;
            buf[1] = 's';
            buf[2] = '\0';
            ssd1306_draw_string(0, 2, "Left:");
            ssd1306_draw_string(30, 2, buf);

            ssd1306_draw_string(0, 3, "REV:");
            ssd1306_draw_string(30, 3, rev  ? "HI" : "LO");
            ssd1306_draw_string(0, 4, "BRK:");
            ssd1306_draw_string(30, 4, brk  ? "HI" : "LO");
            ssd1306_draw_string(0, 5, "STOP:");
            ssd1306_draw_string(30, 5, stop ? "HI" : "LO");

            draw_progress_bar(0, 6, 126, (uint16_t)(elapsed * 1000 / to), 1000);

            ssd1306_refresh();
        }
    }
}
