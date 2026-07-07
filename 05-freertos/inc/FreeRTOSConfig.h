/*
 * FreeRTOSConfig.h — STM32F103C8T6 (Cortex-M3)
 *
 * 这是每个 FreeRTOS 项目必需的头文件，配置内核参数。
 * 适配 FreeRTOS V11.1.0
 */
#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "stm32f1xx.h"

/* ── 内核配置 ───────────────────────────────────────────── */
#define configUSE_PREEMPTION            1
#define configCPU_CLOCK_HZ              ( ( unsigned long ) 72000000 )
#define configTICK_RATE_HZ              ( ( TickType_t ) 1000 )
#define configMAX_PRIORITIES            ( 5 )
#define configMINIMAL_STACK_SIZE        ( ( unsigned short ) 128 )
#define configTOTAL_HEAP_SIZE           ( ( size_t ) ( 10 * 1024 ) )
#define configMAX_TASK_NAME_LEN         ( 16 )
#define configUSE_16_BIT_TICKS          0
#define configIDLE_SHOULD_YIELD         1

/* ── 内核功能开关 ────────────────────────────────────────── */
#define configUSE_MUTEXES               1
#define configUSE_RECURSIVE_MUTEXES     1
#define configUSE_COUNTING_SEMAPHORES   1
#define configUSE_TIMERS                1
#define configUSE_TRACE_FACILITY        1

/* ── 软件定时器 ──────────────────────────────────────────── */
#define configTIMER_TASK_PRIORITY       ( configMAX_PRIORITIES - 1 )
#define configTIMER_QUEUE_LENGTH        10
#define configTIMER_TASK_STACK_DEPTH    ( configMINIMAL_STACK_SIZE * 2 )

/* ── 内存管理 ────────────────────────────────────────────── */
#define configSUPPORT_STATIC_ALLOCATION  0
#define configSUPPORT_DYNAMIC_ALLOCATION 1

/* ── Hook 函数 ───────────────────────────────────────────── */
#define configUSE_IDLE_HOOK             0
#define configUSE_TICK_HOOK             1
#define configUSE_MALLOC_FAILED_HOOK    1

/* ── 运行时统计（需要启用才能用 uxTaskGetStackHighWaterMark） ── */
#define configCHECK_FOR_STACK_OVERFLOW  2
#define configQUEUE_REGISTRY_SIZE       5
#define configRECORD_STACK_HIGH_ADDRESS 1
#define INCLUDE_uxTaskGetStackHighWaterMark 1
#define INCLUDE_xTaskDelayUntil            1

/* ── 中断优先级 ──────────────────────────────────────────── */
#define configKERNEL_INTERRUPT_PRIORITY         255
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    191
#define configLIBRARY_KERNEL_INTERRUPT_PRIORITY  15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

/* ── Assert ──────────────────────────────────────────────── */
#define configASSERT( x ) \
    if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); for( ;; ); }

/* ── SysTick ─────────────────────────────────────────────── */
#define xPortSysTickHandler SysTick_Handler

/* ── Hook 声明 ───────────────────────────────────────────── */
extern void vApplicationTickHook( void );
extern void vApplicationMallocFailedHook( void );

#endif /* FREERTOS_CONFIG_H */
