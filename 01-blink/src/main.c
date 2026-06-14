#include "stm32f1xx.h"

/* LED on Blue Pill: PC13, active-low */
#define LED_PIN    13
#define LED_PORT   GPIOC

/* SysTick-based delay */
static volatile uint32_t g_ticks = 0;

void SysTick_Handler(void) {
    g_ticks++;
}

static void delay_ms(uint32_t ms) {
    uint32_t start = g_ticks;
    while ((g_ticks - start) < ms);
}

/* ── Entry point ────────────────────────────────────────────── */
int main(void) {
    /* System init */
    SystemInit();

    /* Enable GPIOC clock */
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

    /* Configure PC13 as push-pull output, 2 MHz */
    GPIOC->CRH &= ~((uint32_t)0xF << 20);
    GPIOC->CRH |= ((uint32_t)GPIO_OUT_PP_2MHZ << 20);

    /* SysTick: 1 ms tick @ 8 MHz HCLK */
    SYSTICK->LOAD = 7999;   /* 8000 - 1 */
    SYSTICK->VAL  = 0;
    SYSTICK->CTRL = SYSTICK_CTRL_ENABLE |
                    SYSTICK_CTRL_TICKINT |
                    SYSTICK_CTRL_CLKSOURCE;

    while (1) {
        /* LED on (active low) */
        GPIOC->BSRR = GPIO_BSRR_BR(LED_PIN);
        delay_ms(500);

        /* LED off */
        GPIOC->BSRR = GPIO_BSRR_BS(LED_PIN);
        delay_ms(500);
    }
}
