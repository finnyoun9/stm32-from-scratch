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
 *   platformio run
 * 烧录 (需要连接 ST-Link):
 *   platformio run --target upload
 */

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"

/* ── 全局句柄 ──────────────────────────────────────────── */
static QueueHandle_t   xSensorQueue = NULL;
static SemaphoreHandle_t xButtonSemaphore = NULL;
static TimerHandle_t   xWatchdogTimer = NULL;

/* ── 声明 ──────────────────────────────────────────────── */
static void prvSetupHardware( void );
static void prvUARTSendString( const char *str );
static void prvUARTSendNumber( uint32_t num );

/* ================================================================
 * Task 1: vLEDTask — LED 闪烁 (优先级 1, 周期 200ms)
 *
 * 面试价值：展示任务创建、vTaskDelayUntil（精确周期）、
 *           二值信号量同步（模拟按键中断改变闪烁模式）
 * ================================================================ */
static void vLEDTask( void *pvParameters )
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xBlinkPeriod = pdMS_TO_TICKS( 200 );
    TickType_t xCurrentPeriod = xBlinkPeriod;
    BaseType_t xFastMode = pdFALSE;

    for( ;; )
    {
        if( xSemaphoreTake( xButtonSemaphore, 0 ) == pdTRUE )
        {
            xFastMode = !xFastMode;
            xCurrentPeriod = xFastMode ? pdMS_TO_TICKS( 50 ) : xBlinkPeriod;
            prvUARTSendString( xFastMode
                ? "[LED] Fast blink mode ON\r\n"
                : "[LED] Normal mode\r\n" );
        }

        GPIOB->ODR ^= ( 1 << 13 );  /* PC13 = 板载 LED */
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
    static uint32_t ulSensorValue = 0;
    volatile uint32_t ulNoiseCounter = 0;

    for( ;; )
    {
        ulSensorValue = ( ulSensorValue + 7 ) % 4096;
        ulNoiseCounter++;

        if( xQueueSend( xSensorQueue, &ulSensorValue, 0 ) != pdPASS )
        {
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
    UBaseType_t uxQueueDepth = 0;

    prvUARTSendString( "\r\n========================================\r\n" );
    prvUARTSendString( "  FreeRTOS Multi-Task Demo (STM32F103)\r\n" );
    prvUARTSendString( "  By Finn — embedded engineer portfolio\r\n" );
    prvUARTSendString( "========================================\r\n\r\n" );

    for( ;; )
    {
        uxQueueDepth = uxQueueMessagesWaiting( xSensorQueue );
        while( xQueueReceive( xSensorQueue, &ulReceivedValue, 0 ) == pdPASS )
        {
        }

        prvUARTSendString( "[MONITOR] " );
        prvUARTSendString( "Heap free: " );
        prvUARTSendNumber( xPortGetFreeHeapSize() );
        prvUARTSendString( " | Task hi-water: " );
        prvUARTSendNumber( uxTaskGetStackHighWaterMark( NULL ) );
        prvUARTSendString( " | Queue depth: " );
        prvUARTSendNumber( uxQueueDepth );
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

    if( ulHeartbeatCount % 5 == 0 )
    {
        prvUARTSendString( "[WATCHDOG] Heartbeat #" );
        prvUARTSendNumber( ulHeartbeatCount );
        prvUARTSendString( "\r\n" );
    }

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

    if( ulTickCount % 2000 == 0 )
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR( xButtonSemaphore, &xHigherPriorityTaskWoken );
        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    }
}

/* ================================================================
 * Malloc Failed Hook
 * ================================================================ */
void vApplicationMallocFailedHook( void )
{
    prvUARTSendString( "[FATAL] Malloc failed!\r\n" );
    taskDISABLE_INTERRUPTS();
    for( ;; );
}

/* ================================================================
 * Stack Overflow Hook (configCHECK_FOR_STACK_OVERFLOW > 0 必需)
 * ================================================================ */
void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
    prvUARTSendString( "[FATAL] Stack overflow in task: " );
    prvUARTSendString( pcTaskName );
    prvUARTSendString( "\r\n" );
    taskDISABLE_INTERRUPTS();
    for( ;; );
}

/* ================================================================
 * 硬件初始化（使用 CMSIS 寄存器定义，不用自己 #define）
 * ================================================================ */
static void prvSetupHardware( void )
{
    /* 使能 GPIO 时钟 */
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN | RCC_APB2ENR_IOPAEN;

    /* PC13: 推挽输出, 50MHz (板载 LED) */
    GPIOC->CRH &= ~( 0xF << 20 );
    GPIOC->CRH |=  ( 0x3 << 20 );

    /* PA9 (TX): 复用推挽输出, 50MHz */
    GPIOA->CRH &= ~( 0xF << 4 );
    GPIOA->CRH |=  ( 0xB << 4 );

    /* USART1 初始化: 115200 8N1 @72MHz */
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    USART1->BRR = 0x271;
    USART1->CR1 |= USART_CR1_TE;
    USART1->CR1 |= USART_CR1_UE;
}

/* ================================================================
 * 阻塞式 UART 发送
 * ================================================================ */
static void prvUARTSendChar( char c )
{
    while( ( USART1->SR & USART_SR_TXE ) == 0 );
    USART1->DR = c;
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
 * main()
 * ================================================================ */
int main( void )
{
    prvSetupHardware();
    prvUARTSendString( "Booting FreeRTOS...\r\n" );

    xSensorQueue = xQueueCreate( 10, sizeof( uint32_t ) );
    configASSERT( xSensorQueue != NULL );

    xButtonSemaphore = xSemaphoreCreateBinary();
    configASSERT( xButtonSemaphore != NULL );

    xWatchdogTimer = xTimerCreate(
        "Watchdog",
        pdMS_TO_TICKS( 1000 ),
        pdTRUE,
        ( void * ) 0,
        vWatchdogCallback
    );
    configASSERT( xWatchdogTimer != NULL );
    xTimerStart( xWatchdogTimer, 0 );

    xTaskCreate( vLEDTask,     "LED",     configMINIMAL_STACK_SIZE, NULL, 1, NULL );
    xTaskCreate( vSensorTask,  "Sensor",  configMINIMAL_STACK_SIZE, NULL, 2, NULL );
    xTaskCreate( vMonitorTask, "Monitor", configMINIMAL_STACK_SIZE * 2, NULL, 3, NULL );

    prvUARTSendString( "Tasks created. Starting scheduler...\r\n" );
    vTaskStartScheduler();

    for( ;; );
    return 0;
}
