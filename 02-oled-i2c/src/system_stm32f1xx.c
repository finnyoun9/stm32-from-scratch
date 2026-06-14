#include "stm32f1xx.h"

/* HSI = 8 MHz (internal oscillator, default after reset).
   SYSCLK = 8 MHz, HCLK = 8 MHz, APB2 = 8 MHz. */
void SystemInit(void) {
    /* Set vector table offset to start of flash */
    SCB->VTOR = 0x08000000;

    /* Two wait states if we ever switch to higher clock (>48 MHz).
       Default HSI is 8 MHz → zero wait states ok. */
#if 0
    FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY_Msk) | 2;
#endif

    /* HSI is already on by default. Nothing else needed. */
}
