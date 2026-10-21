#include "FreeRTOS.h"
#include "task.h"

#include "led.h"

#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <stdint.h>

static void fault_halt(void)
{
    volatile uint32_t i;

    cli();
    wdt_disable();
    led_init();

    for (;;) {
        led_toggle();
        for (i = 0U; i < 200000U; i++) {
        }
    }
}

void vAssertCalled(const char *pcFile, int lLine)
{
    (void)pcFile;
    (void)lLine;
    fault_halt();
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    fault_halt();
}

void vApplicationMallocFailedHook(void)
{
    fault_halt();
}
