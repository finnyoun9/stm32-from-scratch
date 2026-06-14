/*
 * main.c — FreeRTOS 多任务 Demo (STM32F103C8T6)
 *
 * 演示内容（对标深圳嵌入式面试高频考点）：
 *   1. 3 个 FreeRTOS 任务（不同优先级 + 不同周期）
 *   2. 队列（Queue）跨任务通信
 *   3. 软件定时器（Software Timer）
 *   4. 二值信号量（Binary Semaphore）同步 ISR → Task
 *   5. Tick Hook 做调试追踪
 *
 * 硬件需求（当前无需连接，编译验证即可）：
 *   - STM32F103C8T6 (Blue Pill)
 *   - LED on PC13 (板载 LED)
 *   - UART1 (PA9 TX, PA10 RX) @ 115200 8N1
 *
 * 编译:
 *   mkdir build && cd build
 *   cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-none-eabi.cmake
 *   make
 *
 * 烧录 (需要连接 ST-Link):
 *   make flash
 */

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"

/* STM32 寄存器直接访问（裸机风格，证明你懂寄存器） */
#define RCC_BASE        0x40021000UL
#define GPIOC_BASE      0x40011000UL
#define GPIOA_BASE      0x40010800UL
#define USART1_BASE     0x40013800UL

#define RCC_APB2ENR     (*(volatile uint32_t *)(RCC_BASE    + 0x18))
#define GPIOC_CRH       (*(volatile uint32_t *)(GPIOC_BASE  + 0x04))
#define GPIOC_ODR       (*(volatile uint32_t *)(GPIOC_BASE  + 0x0C))
#define GPIOC_BSRR      (*(volatile uint32_t *)(GPIOC_BASE  + 0x10))
#define GPIOA_CRH       (*(volatile uint32_t *)(GPIOA_BASE  + 0x04))

/* 外设时钟使能位 */
#define RCC_APB2ENR_IOPCEN   (1 << 4)
#define RCC_APB2ENR_IOPAEN   (1 << 2)
#define RCC_APB2ENR_USART1EN (1 << 14)

/* ---------- 全局句柄 ---------- */
static QueueHandle_t   xSensorQueue = NULL;    /* 模拟传感器数据队列 */
static SemaphoreHandle_t xButtonSemaphore = NULL;  /* 模拟按键中断信号量 */
static TimerHandle_t   xWatchdogTimer = NULL;  /* 看门狗定时器 */

/* ---------- 声明 ---------- */
static void prvSetupHardware( void );
static void prvUARTSendString( const char *str );
static void prvUARTSendNumber( uint32_t num );

/* ================================================================
 * Task 1: vLEDTask — LED 闪烁 (优先级 1, 周期 500ms)
 *
 * 面试价值：展示任务创建、vTaskDelayUntil（精确周期）、
 *           二值信号量同步（模拟按键中断改变闪烁模式）
 * ================================================================ */
static void vLEDTask( void *pvParameters )
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xBlinkPeriod = pdMS_TO_TICKS( 200 );  /* 正常: 200ms */
    TickType_t xCurrentPeriod = xBlinkPeriod;
    BaseType_t xFastMode = pdFALSE;

    for( ;; )
    {
        /* 检查按键信号量（非阻塞）—— ISR → Task 同步 */
        if( xSemaphoreTake( xButtonSemaphore, 0 ) == pdTRUE )
        {
            xFastMode = !xFastMode;  /* 切换快闪模式 */
            xCurrentPeriod = xFastMode ? pdMS_TO_TICKS( 50 ) : xBlinkPeriod;

            prvUARTSendString( xFastMode
                ? "[LED] Fast blink mode ON\r\n"
                : "[LED] Normal mode\r\n" );
        }

        /* 翻转 PC13 (板载 LED) */
        GPIOC_ODR ^= ( 1 << 13 );

        /* 精确周期延时 */
        vTaskDelayUntil( &xLastWakeTime, xCurrentPeriod );
    }
}

/* ================================================================
 * Task 2: vSensorTask — 模拟传感器采集 (优先级 2, 周期 100ms)
 *
 * 面试价值：展示 Queue 发送、优先级抢占、
 *           volatile 防编译器优化理解
 * ================================================================ */
static void vSensorTask( void *pvParameters )
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    static uint32_t ulSensorValue = 0;     /* 模拟传感器数据 */
    volatile uint32_t ulNoiseCounter = 0;  /* volatile 防止被优化 */

    for( ;; )
    {
        /* 模拟读取传感器（实际项目换成 ADC） */
        ulSensorValue = ( ulSensorValue + 7 ) % 4096;  /* 0-4095, 模拟 12-bit ADC */
        ulNoiseCounter++;  /* volatile 变量: 编译器不会优化掉 */

        /* 发送到队列（0 超时 = 非阻塞） */
        if( xQueueSend( xSensorQueue, &ulSensorValue, 0 ) != pdPASS )
        {
            /* 队列满 —— 在实际项目中记录 overflow 事件 */
            prvUARTSendString( "[SENSOR] Queue full!\r\n" );
        }

        vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 100 ) );
    }
}

/* ================================================================
 * Task 3: vMonitorTask — 数据监控 + 串口输出 (优先级 3, 周期 1s)
 *
 * 面试价值：展示 Queue 接收、优先级最高 = 数据消费者、
 *           任务状态监控（uxTaskGetStackHighWaterMark）
 * ================================================================ */
static void vMonitorTask( void *pvParameters )
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint32_t ulReceivedValue = 0;
    UBaseType_t uxQueueMessagesWaiting = 0;

    /* 系统启动日志 —— 面试加分：证明你考虑了可观测性 */
    prvUARTSendString( "\r\n========================================\r\n" );
    prvUARTSendString( "  FreeRTOS Multi-Task Demo (STM32F103)\r\n" );
    prvUARTSendString( "  By Finn — 嵌入式工程师转岗项目\r\n" );
    prvUARTSendString( "========================================\r\n\r\n" );

    for( ;; )
    {
        /* 批量消费队列中的数据 */
        uxQueueMessagesWaiting = uxQueueMessagesWaiting( xSensorQueue );
        while( xQueueReceive( xSensorQueue, &ulReceivedValue, 0 ) == pdPASS )
        {
            /* 消费数据 —— 实际项目这里做滤波/融合/上报 */
        }

        /* 串口输出状态 */
        prvUARTSendString( "[MONITOR] " );
        prvUARTSendString( "Heap free: " );
        prvUARTSendNumber( xPortGetFreeHeapSize() );
        prvUARTSendString( " | Task hi-water: " );
        prvUARTSendNumber( uxTaskGetStackHighWaterMark( NULL ) );  /* NULL = 当前任务 */
        prvUARTSendString( " | Queue depth: " );
        prvUARTSendNumber( uxQueueMessagesWaiting );
        prvUARTSendString( " | Last sensor: " );
        prvUARTSendNumber( ulReceivedValue );
        prvUARTSendString( "\r\n" );

        vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 1000 ) );
    }
}

/* ================================================================
 * 软件定时器回调: vWatchdogCallback
 *
 * 面试价值：展示软件定时器使用、看门狗概念理解
 * ================================================================ */
static void vWatchdogCallback( TimerHandle_t xTimer )
{
    static uint32_t ulHeartbeatCount = 0;
    ulHeartbeatCount++;

    /* 每 5 次输出心跳（5s 一次） */
    if( ulHeartbeatCount % 5 == 0 )
    {
        prvUARTSendString( "[WATCHDOG] Heartbeat #" );
        prvUARTSendNumber( ulHeartbeatCount );
        prvUARTSendString( "\r\n" );
    }

    /* 重置定时器，保持运行 */
    xTimerStart( xWatchdogTimer, 0 );
}

/* ================================================================
 * Tick Hook: 每个 Tick 调用一次 (1ms)
 *
 * 面试价值：展示对 RTOS 内核机制的理解
 * ================================================================ */
void vApplicationTickHook( void )
{
    static uint32_t ulTickCount = 0;
    ulTickCount++;

    /* 每 2000 ticks (2s) 触发一次模拟按键中断 */
    if( ulTickCount % 2000 == 0 )
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        /* 从 ISR 给信号量 —— 典型的 ISR→Task 同步模式 */
        xSemaphoreGiveFromISR( xButtonSemaphore, &xHigherPriorityTaskWoken );

        /* 如果唤醒了更高优先级任务，请求上下文切换 */
        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    }
}

/* ================================================================
 * Malloc Failed Hook: 内存分配失败时调用
 * ================================================================ */
void vApplicationMallocFailedHook( void )
{
    /* 实际项目中发送错误日志、进入安全模式 */
    prvUARTSendString( "[FATAL] Malloc failed!\r\n" );
    taskDISABLE_INTERRUPTS();
    for( ;; );
}

/* ================================================================
 * 硬件初始化（裸机风格 —— 证明你会操作寄存器）
 * ================================================================ */
static void prvSetupHardware( void )
{
    /* 使能 GPIO 时钟 */
    RCC_APB2ENR |= RCC_APB2ENR_IOPCEN | RCC_APB2ENR_IOPAEN;

    /* PC13: 推挽输出, 50MHz (板载 LED) */
    GPIOC_CRH &= ~( 0xF << 20 );
    GPIOC_CRH |=  ( 0x3 << 20 );

    /* PA9 (TX): 复用推挽输出, 50MHz */
    GPIOA_CRH &= ~( 0xF << 4 );
    GPIOA_CRH |=  ( 0xB << 4 );  /* 0b1011 = 50MHz AF push-pull */

    /* USART1 初始化（简化版, 115200 8N1 @72MHz） */
    RCC_APB2ENR |= RCC_APB2ENR_USART1EN;

    /* USART1 BRR: 72000000 / 115200 = 625 = 0x271 */
    *(volatile uint32_t *)(USART1_BASE + 0x08) = 0x271;  /* BRR */
    *(volatile uint32_t *)(USART1_BASE + 0x0C) |= (1 << 3);  /* TE (Transmit Enable) */
    *(volatile uint32_t *)(USART1_BASE + 0x0C) |= (1 << 13); /* UE (USART Enable) */
}

/* ================================================================
 * 阻塞式 UART 发送（简化版 —— 不用中断，面试展示基础能力）
 * ================================================================ */
static void prvUARTSendChar( char c )
{
    /* 等待发送数据寄存器空 */
    while( ( *(volatile uint32_t *)(USART1_BASE + 0x00) & (1 << 7) ) == 0 );
    *(volatile uint32_t *)(USART1_BASE + 0x04) = c;
}

static void prvUARTSendString( const char *str )
{
    while( *str )
    {
        prvUARTSendChar( *str++ );
    }
}

static void prvUARTSendNumber( uint32_t num )
{
    char buf[12];
    int i = 10;
    buf[11] = '\0';

    if( num == 0 )
    {
        prvUARTSendChar( '0' );
        return;
    }

    while( num > 0 && i >= 0 )
    {
        buf[i--] = '0' + ( num % 10 );
        num /= 10;
    }
    prvUARTSendString( &buf[i + 1] );
}

/* ================================================================
 * main() — 创建所有任务和内核对象
 * ================================================================ */
int main( void )
{
    prvSetupHardware();

    prvUARTSendString( "Booting FreeRTOS...\r\n" );

    /* 创建队列: 容纳 10 个 uint32_t */
    xSensorQueue = xQueueCreate( 10, sizeof( uint32_t ) );
    configASSERT( xSensorQueue != NULL );

    /* 创建二值信号量（初始为空） */
    xButtonSemaphore = xSemaphoreCreateBinary();
    configASSERT( xButtonSemaphore != NULL );

    /* 创建看门狗定时器: 1s 周期, 自动重载 */
    xWatchdogTimer = xTimerCreate(
        "Watchdog",
        pdMS_TO_TICKS( 1000 ),
        pdTRUE,   /* 自动重载 */
        ( void * ) 0,
        vWatchdogCallback
    );
    configASSERT( xWatchdogTimer != NULL );
    xTimerStart( xWatchdogTimer, 0 );

    /* 创建任务 */
    xTaskCreate( vLEDTask,     "LED",     configMINIMAL_STACK_SIZE, NULL, 1, NULL );
    xTaskCreate( vSensorTask,  "Sensor",  configMINIMAL_STACK_SIZE, NULL, 2, NULL );
    xTaskCreate( vMonitorTask, "Monitor", configMINIMAL_STACK_SIZE * 2, NULL, 3, NULL );

    prvUARTSendString( "Tasks created. Starting scheduler...\r\n" );

    /* 启动调度器 —— 永不返回 */
    vTaskStartScheduler();

    /* 理论上不会到这里 */
    for( ;; );
    return 0;
}
