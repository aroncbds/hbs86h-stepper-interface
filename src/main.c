
#include "FreeRTOS.h"
#include "task.h"

#include "console.h"
#include "console_commands.h"
#include "led.h"
#include "motion_controller.h"

#include "twi.h"
#include "lcd_hd44780.h"
#include "lcd_ui.h"

#include <stdint.h>

#if defined(__AVR_3_BYTE_PC__)
#define LOWTEXT __attribute__((section(".lowtext")))
#else
#define LOWTEXT
#endif

#define HEARTBEAT_STACK_BYTES 320U
#define HEARTBEAT_PRIORITY (tskIDLE_PRIORITY + 1U)

/*
 * Slightly increased for LCD UI local buffers.
 * Stack sizes are bytes on our AVR FreeRTOS port.
 */
#define CONSOLE_STACK_BYTES 640U
#define CONSOLE_PRIORITY (tskIDLE_PRIORITY + 2U)

#define LCD_UPDATE_MS 200U

static motion_controller_t controller;
static console_commands_t commands;

static uint8_t lcd_available = 0U;

/*
 * Convert controller state to LCD snapshot.
 *
 * No additional controller instance is created.
 */

 static void lcd_update_controller(void)
 {
    lcd_ui_snapshot_t snapshot;
 
    snapshot.state = motion_controller_state_name(controller.state);
    snapshot.position_steps = controller.position_steps;
    snapshot.target_steps = controller.target_steps;
    snapshot.position_valid = controller.position_valid ? 1U : 0U;
    snapshot.target_unit = commands.target_unit;
 
     if (controller.state == MOTION_STATE_FAULT) {
         snapshot.status = "FAULT";
 
     } else if (controller.state == MOTION_STATE_HOMING) {
         snapshot.status = "HOMING";
 
     } else if (controller.state == MOTION_STATE_DECELERATING) {
         snapshot.status = "STOPPING";
 
     } else if (controller.state == MOTION_STATE_NOT_HOMED) {
         snapshot.status = "HOME NEEDED";
 
     } else if (controller.state == MOTION_STATE_IDLE) {
         snapshot.status = "READY";
 
     } else if (controller.state == MOTION_STATE_MANUAL) {
         snapshot.status = "MANUAL";
 
     } else if (controller.state == MOTION_STATE_AUTO) {
         snapshot.status = "AUTO";
 
     } else {
         snapshot.status = "INITIALIZING";
     }
 
     (void)lcd_ui_update(&snapshot);
 }
 

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
    TickType_t last_lcd_update;

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
        &controller,
        init_message
    );

    console_commands_print_help();
    console_write("\n> ");

    /*
     * First LCD update immediately after
     * the controller has been initialized.
     */
    if (lcd_available) {
        lcd_ui_init();
        lcd_update_controller();
    }

    last_lcd_update = xTaskGetTickCount();

    for (;;) {

        /*
         * Process serial commands.
         * These modify our shared controller.
         */
        while (console_read_char(&character)) {

            console_commands_process_char(
                &commands,
                character
            );

            if (character == '\r' ||
                character == '\n') {
                console_write("> ");
            }
        }

        /*
         * Update LCD every 200 ms.
         * Only changed characters are transmitted.
         */
        if (lcd_available) {

            TickType_t now = xTaskGetTickCount();

            if ((TickType_t)(now - last_lcd_update) >=
                pdMS_TO_TICKS(LCD_UPDATE_MS)) {

                last_lcd_update = now;
                lcd_update_controller();
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

int main(void)
{
    led_init();
    console_init();

    twi_init();

    twi_result_t lcd_result = lcd_init();

    if (lcd_result == TWI_OK) {

        lcd_available = 1U;
        console_write("LCD initialized\n");

    } else {

        lcd_available = 0U;

        console_write("LCD initialization failed\n");
        console_write("TWI error: ");
        console_write_u16((uint16_t)lcd_result);
        console_write("\n");
    }

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
