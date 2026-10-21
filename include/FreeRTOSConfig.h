#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#ifndef F_CPU
#error F_CPU must be defined on the compiler command line
#endif

/*
 * Tick source.
 * 1: Timer0 CTC. Leaves the watchdog off so a fault cannot lock an old
 *    Arduino Mega bootloader out of USB upload. Timer1, Timer3, Timer4 and
 *    Timer5 stay free for step generation. The port prescales Timer0 by
 *    1024, so 100 Hz is the rate that divides cleanly on a 16 MHz crystal
 *    (actual rate about 100.2 Hz).
 * 0: Enhanced watchdog interrupt at about 16 ms (WDTO_15MS). Frees Timer0
 *    as well. Use this only when a Dragon is available to recover the chip,
 *    because a stuck system can arm the watchdog reset and wedge the stock
 *    Mega bootloader.
 */
#define configAVR_USE_TIMER0_TICK               1

#if ( configAVR_USE_TIMER0_TICK == 1 )
#define configTICK_RATE_HZ                      100
#else
#define configTICK_RATE_HZ                      62
#endif

#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configUSE_TICKLESS_IDLE                 0
#define configCPU_CLOCK_HZ                      ( ( unsigned long ) F_CPU )
#define configMAX_PRIORITIES                    4
/*
 * This port's StackType_t is uint8_t, so stack sizes are bytes, not 16-bit
 * or 32-bit words. Every task stack also has to hold the tick ISR.
 */
#define configMINIMAL_STACK_SIZE                256
#define configMAX_TASK_NAME_LEN                 8
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_TASK_NOTIFICATIONS           1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES  1
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             0
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_QUEUE_SETS                    0
#define configQUEUE_REGISTRY_SIZE               0
#define configUSE_TIME_SLICING                  1
#define configUSE_NEWLIB_REENTRANT              0
#define configENABLE_BACKWARD_COMPATIBILITY     0
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 0
#define configSTACK_DEPTH_TYPE                  uint16_t
#define configUSE_MINI_LIST_ITEM                1

#define configSUPPORT_STATIC_ALLOCATION         0
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#ifndef configTOTAL_HEAP_SIZE
#define configTOTAL_HEAP_SIZE                   4096
#endif
#define configAPPLICATION_ALLOCATED_HEAP        0

#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configCHECK_FOR_STACK_OVERFLOW          2
#define configUSE_MALLOC_FAILED_HOOK            1
#define configUSE_DAEMON_TASK_STARTUP_HOOK      0

#define configGENERATE_RUN_TIME_STATS           0
#define configUSE_TRACE_FACILITY                0
#define configUSE_STATS_FORMATTING_FUNCTIONS    0

#define configUSE_CO_ROUTINES                   0
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               ( configMAX_PRIORITIES - 1 )
#define configTIMER_QUEUE_LENGTH                8
#define configTIMER_TASK_STACK_DEPTH            384

#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_xTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_uxTaskGetStackHighWaterMark     0
#define INCLUDE_uxTaskGetStackHighWaterMark2    1
#define INCLUDE_xTaskGetIdleTaskHandle          0
#define INCLUDE_eTaskGetState                   0
#define INCLUDE_xTimerPendFunctionCall          0
#define INCLUDE_xTaskAbortDelay                 0
#define INCLUDE_xTaskGetHandle                  0
#define INCLUDE_xTaskResumeFromISR              1

#define configASSERT( x ) \
    do { if( ( x ) == 0 ) { vAssertCalled( __FILE__, __LINE__ ); } } while( 0 )

void vAssertCalled( const char * pcFile, int lLine );

#endif /* FREERTOS_CONFIG_H */
