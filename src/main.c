
#include "FreeRTOS.h"
#include "task.h"

#include "console.h"
#include "console_commands.h"
#include "led.h"
#include "motion_controller.h"

#include <stdint.h>

#if defined(__AVR_3_BYTE_PC__)
#define LOWTEXT __attribute__((section(".lowtext")))
#else
#define LOWTEXT
#endif

#define HEARTBEAT_STACK_BYTES 320U
#define HEARTBEAT_PRIORITY    (tskIDLE_PRIORITY + 1U)

#define CONSOLE_STACK_BYTES   512U
#define CONSOLE_PRIORITY      (tskIDLE_PRIORITY + 2U)

static motion_controller_t controller;
static console_commands_t commands;

static void LOWTEXT heartbeat_task(void *parameters)
{
    TickType_t last_wake;

    (void)parameters;
    last_wake = xTaskGetTickCount();

    for (;;) {
        led_toggle();

        vTaskDelayUntil(
            &last_wake,
            pdMS_TO_TICKS(500)
        );
    }
}

static void LOWTEXT console_task(void *parameters)
{
    char character;

    (void)parameters;

    motion_controller_init(&controller);
    console_commands_init(&commands, &controller);

    console_write("\nMotion controller console\n");

    /*
     * Simulate successful hardware initialization.
     * No physical outputs are enabled.
     */
    motion_message_t init_message = {
        .type = MOTION_EVENT_INIT_OK,
        .value = 0
    };

    (void)motion_controller_process(
        &controller, init_message
    );

    console_commands_print_help();
    console_write("\n> ");

    for (;;) {
        while (console_read_char(&character)) {
            console_commands_process_char(
                &commands, character
            );

            if (character == '\r' ||
                character == '\n') {
                console_write("> ");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

int main(void)
{
    led_init();
    console_init();

    console_write(
        "stepper-interface " BOARD_NAME "\n"
    );

    configASSERT(
        xTaskCreate(
            heartbeat_task,
            "beat",
            HEARTBEAT_STACK_BYTES,
            NULL,
            HEARTBEAT_PRIORITY,
            NULL
        ) == pdPASS
    );

    configASSERT(
        xTaskCreate(
            console_task,
            "console",
            CONSOLE_STACK_BYTES,
            NULL,
            CONSOLE_PRIORITY,
            NULL
        ) == pdPASS
    );

    vTaskStartScheduler();

    vAssertCalled(__FILE__, __LINE__);
    return 0;
}
