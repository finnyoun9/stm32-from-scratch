/*
 * FreeRTOSConfig.h — STM32F103C8T6 (Cortex-M3)
 * 
 * 这是每个 FreeRTOS 项目必需的头文件，配置内核参数。
 */
#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "stm32f1xx.h"

/* 时钟配置 */
#define configCPU_CLOCK_HZ              ( ( unsigned long ) 72000000 )  /* SYSCLK = 72 MHz */
#define configTICK_RATE_HZ              ( ( TickType_t ) 1000 )         /* Tick = 1ms */
#define configMAX_PRIORITIES            ( 5 )
#define configMINIMAL_STACK_SIZE        ( ( unsigned short ) 128 )
#define configTOTAL_HEAP_SIZE           ( ( size_t ) ( 10 * 1024 ) )    /* 10KB heap */
#define configMAX_TASK_NAME_LEN         ( 16 )
#define configUSE_TRACE_FACILITY        1
#define configUSE_16_BIT_TICKS          0
#define configIDLE_SHOULD_YIELD         1
#define configUSE_MUTEXES               1
#define configQUEUE_REGISTRY_SIZE       5
#define configCHECK_FOR_STACK_OVERFLOW  2
#define configUSE_RECURSIVE_MUTEXES     1
#define configUSE_COUNTING_SEMAPHORES   1
#define configUSE_TIMERS                1
#define configTIMER_TASK_PRIORITY       ( configMAX_PRIORITIES - 1 )
#define configTIMER_QUEUE_LENGTH        10
#define configTIMER_TASK_STACK_DEPTH    ( configMINIMAL_STACK_SIZE * 2 )

/* 内存分配方案: heap_4.c (最佳通用方案) */
#define configSUPPORT_STATIC_ALLOCATION 0
#define configSUPPORT_DYNAMIC_ALLOCATION 1

/* Hook 函数 */
#define configUSE_IDLE_HOOK             0
#define configUSE_TICK_HOOK             1
#define configUSE_MALLOC_FAILED_HOOK    1

/* 软件定时器 */
#define configUSE_TIMERS                1
#define configTIMER_TASK_PRIORITY       2
#define configTIMER_QUEUE_LENGTH        10
#define configTIMER_TASK_STACK_DEPTH    256

/* 中断优先级 */
#define configKERNEL_INTERRUPT_PRIORITY   255  /* 最低优先级（FreeRTOS 不可屏蔽的中断）*/
#define configMAX_SYSCALL_INTERRUPT_PRIORITY  191  /* 等效于 __NVIC_PRIO_BITS=4 时的 5 */
#define configLIBRARY_KERNEL_INTERRUPT_PRIORITY  15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY  5

/* Assert */
#define configASSERT( x ) if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); for( ;; ); }

/* 用 SysTick 提供 Tick */
#define xPortSysTickHandler SysTick_Handler

/* 可选 trace 宏 */
extern void vApplicationTickHook( void );
extern void vApplicationMallocFailedHook( void );

#endif /* FREERTOS_CONFIG_H */
