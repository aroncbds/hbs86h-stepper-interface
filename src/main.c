#include "FreeRTOS.h"
#include "task.h"

#include "console.h"
#include "led.h"

#include <stdint.h>

/*
 * Step pulses for the HBS86H belong on a 16-bit timer ISR (Timer1, 3, 4 or
 * 5 on the Mega). The RTOS tick is only the housekeeping clock.
 *
 * On the ATmega2560 the port stores a 16-bit start address for each task and
 * forces the third program-counter byte to 0, so task functions stay in the
 * low 128 KiB via .lowtext.
 */
#if defined(__AVR_3_BYTE_PC__)
#define LOWTEXT __attribute__((section(".lowtext")))
#else
#define LOWTEXT
#endif

#define HEARTBEAT_STACK_BYTES 320U
#define HEARTBEAT_PRIORITY    (tskIDLE_PRIORITY + 1U)

static void LOWTEXT heartbeat_task(void *parameters);

static void LOWTEXT heartbeat_task(void *parameters)
{
    TickType_t last_wake;
    uint16_t beat = 0U;

    (void)parameters;
    last_wake = xTaskGetTickCount();

    for (;;) {
        led_toggle();
        beat++;
        console_write("heartbeat ");
        console_write_u16(beat);
        console_write(" stack ");
        console_write_u16((uint16_t)uxTaskGetStackHighWaterMark2(NULL));
        console_write("\n");
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(500));
    }
}

int main(void)
{
    led_init();
    console_init();
    console_write("stepper-interface " BOARD_NAME "\n");

    if (xTaskCreate(heartbeat_task,
                    "beat",
                    HEARTBEAT_STACK_BYTES,
                    NULL,
                    HEARTBEAT_PRIORITY,
                    NULL) != pdPASS) {
        vAssertCalled(__FILE__, __LINE__);
    }

    vTaskStartScheduler();

    /* The scheduler only returns if the idle task could not be created. */
    vAssertCalled(__FILE__, __LINE__);
    return 0;
}
