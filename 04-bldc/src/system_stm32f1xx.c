#include "stm32f1xx.h"

/* HSI 8MHz → PLL ×16 = 64MHz SYSCLK (no external crystal needed).
   AHB = 64MHz, APB2 = 64MHz, APB1 = 32MHz.
   TIM2 clock = APB1 × 2 = 64MHz (doubled because APB1 prescaler > 1). */
void SystemInit(void) {
    SCB->VTOR = 0x08000000;

    /* Two wait states required for > 48MHz */
    FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY_Msk) | 2;

    /* PLL source = HSI/2 (4MHz, default), PLLMUL = ×16 → 64MHz.
       PPRE1 = /2 (APB1 = 32MHz) */
    RCC->CFGR = (0xF << 18)   /* PLLMUL = ×16 */
              | (0x4 << 8);   /* PPRE1 = /2 */

    /* Enable PLL and wait */
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));

    /* Switch system clock to PLL */
    RCC->CFGR = (RCC->CFGR & ~0x3) | RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & 0xC) != RCC_CFGR_SWS_PLL);
}
