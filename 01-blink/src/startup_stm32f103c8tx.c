#include "stm32f1xx.h"

/* Forward declarations */
extern int main(void);
extern uint32_t _estack;
extern uint32_t _sidata, _sdata, _edata;
extern uint32_t _sbss, _ebss;

/* ── Default handlers (weak, user can override) ──────────────── */
void Reset_Handler(void);

__attribute__((weak)) void NMI_Handler(void)           { for(;;); }
__attribute__((weak)) void HardFault_Handler(void)     { for(;;); }
__attribute__((weak)) void MemManage_Handler(void)     { for(;;); }
__attribute__((weak)) void BusFault_Handler(void)      { for(;;); }
__attribute__((weak)) void UsageFault_Handler(void)    { for(;;); }
__attribute__((weak)) void SVC_Handler(void)           { for(;;); }
__attribute__((weak)) void DebugMon_Handler(void)      { for(;;); }
__attribute__((weak)) void PendSV_Handler(void)        { for(;;); }
__attribute__((weak)) void SysTick_Handler(void)       { for(;;); }
__attribute__((weak)) void WWDG_IRQHandler(void)       { for(;;); }
__attribute__((weak)) void PVD_IRQHandler(void)        { for(;;); }
__attribute__((weak)) void TAMPER_IRQHandler(void)     { for(;;); }
__attribute__((weak)) void RTC_IRQHandler(void)        { for(;;); }
__attribute__((weak)) void FLASH_IRQHandler(void)      { for(;;); }
__attribute__((weak)) void RCC_IRQHandler(void)        { for(;;); }
__attribute__((weak)) void EXTI0_IRQHandler(void)      { for(;;); }
__attribute__((weak)) void EXTI1_IRQHandler(void)      { for(;;); }
__attribute__((weak)) void EXTI2_IRQHandler(void)      { for(;;); }
__attribute__((weak)) void EXTI3_IRQHandler(void)      { for(;;); }
__attribute__((weak)) void EXTI4_IRQHandler(void)      { for(;;); }
__attribute__((weak)) void DMA1_Channel1_IRQHandler(void) { for(;;); }

/* ── Vector table ────────────────────────────────────────────── */
__attribute__((section(".isr_vector"), used))
void (* const g_pfnVectors[])(void) = {
    (void (*)(void))&_estack,       /* 0x00: initial SP */
    Reset_Handler,                  /* 0x04: reset */
    NMI_Handler,                    /* 0x08: NMI */
    HardFault_Handler,              /* 0x0C: hard fault */
    MemManage_Handler,              /* 0x10: MPU fault */
    BusFault_Handler,               /* 0x14: bus fault */
    UsageFault_Handler,             /* 0x18: usage fault */
    0, 0, 0, 0,                    /* reserved */
    SVC_Handler,                    /* 0x2C: SVCall */
    DebugMon_Handler,               /* 0x30: debug monitor */
    0,                              /* reserved */
    PendSV_Handler,                 /* 0x38: PendSV */
    SysTick_Handler,                /* 0x3C: SysTick */
    WWDG_IRQHandler,                /* 0x40: WWDG */
    PVD_IRQHandler,                 /* 0x44: PVD */
    TAMPER_IRQHandler,              /* 0x48: Tamper */
    RTC_IRQHandler,                 /* 0x4C: RTC */
    FLASH_IRQHandler,               /* 0x50: Flash */
    RCC_IRQHandler,                 /* 0x54: RCC */
    EXTI0_IRQHandler,              /* 0x58: EXTI0 */
    EXTI1_IRQHandler,              /* 0x5C: EXTI1 */
    EXTI2_IRQHandler,              /* 0x60: EXTI2 */
    EXTI3_IRQHandler,              /* 0x64: EXTI3 */
    EXTI4_IRQHandler,              /* 0x68: EXTI4 */
    DMA1_Channel1_IRQHandler,      /* 0x6C: DMA1 Ch1 */
};

/* ── Reset handler ──────────────────────────────────────────── */
void Reset_Handler(void) {
    /* Copy .data from flash to RAM */
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;
    while (dst < &_edata) {
        *dst++ = *src++;
    }

    /* Zero .bss */
    for (uint32_t *p = &_sbss; p < &_ebss; p++) {
        *p = 0;
    }

    /* Run user code */
    main();

    for (;;);
}
