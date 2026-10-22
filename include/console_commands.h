
#ifndef CONSOLE_COMMANDS_H
#define CONSOLE_COMMANDS_H

#include "motion_controller.h"

#include <stdint.h>

#define CONSOLE_COMMAND_MAX_LENGTH 48U

typedef struct {
    motion_controller_t *controller;
    char line[CONSOLE_COMMAND_MAX_LENGTH];
    uint8_t length;
} console_commands_t;

void console_commands_init(
    console_commands_t *console,
    motion_controller_t *controller
);

void console_commands_process_char(
    console_commands_t *console,
    char character
);

void console_commands_print_help(void);

#endif
