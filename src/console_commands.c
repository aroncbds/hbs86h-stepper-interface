
#include "console_commands.h"
#include "console.h"
#include "motion_limits.h"

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static void print_i32(int32_t value)
{
    char buffer[12];
    char *p = &buffer[sizeof(buffer) - 1U];
    uint32_t magnitude;

    *p = '\0';

    if (value < 0) {
        magnitude = (uint32_t)(-(value + 1)) + 1U;
    } else {
        magnitude = (uint32_t)value;
    }

    do {
        *--p = (char)('0' + (magnitude % 10U));
        magnitude /= 10U;
    } while (magnitude != 0U);

    if (value < 0) {
        *--p = '-';
    }

    console_write(p);
}

static bool parse_i32(const char *text, int32_t *result)
{
    char *end;
    long value;

    if (text == NULL || result == NULL || *text == '\0') {
        return false;
    }

    errno = 0;
    value = strtol(text, &end, 10);

    if (end == text || *end != '\0' || errno == ERANGE ||
        value < INT32_MIN || value > INT32_MAX) {
        return false;
    }

    *result = (int32_t)value;
    return true;
}

static bool mm_to_steps(int32_t mm, int32_t *steps)
{
    if (steps == NULL) {
        return false;
    }

    /* check physical range before multiplication */
    if (mm < MOTION_TRAVEL_MIN_MM ||
        mm > MOTION_TRAVEL_MAX_MM) {
        return false;
    }

    if (MOTION_STEPS_PER_MM <= 0L) {
        return false;
    }

    /* avoid int32 overflow */
    if (mm > INT32_MAX / MOTION_STEPS_PER_MM) {
        return false;
    }

    *steps = mm * MOTION_STEPS_PER_MM;
    return true;
}

static void send_event(
    console_commands_t *console,
    motion_event_t event,
    int32_t value)
{
    motion_message_t message = {
        .type = event,
        .value = value
    };

    motion_response_t response =
        motion_controller_process_ex(
            console->controller,
            message
        );

    console_write("\nEVENT: ");
    console_write(motion_event_name(event));
    console_write("\n");

    switch (response.result) {

    case MOTION_RESULT_ACCEPTED:
        console_write("RESULT: ACCEPTED\n");
        break;

    case MOTION_RESULT_REJECTED:
        console_write("RESULT: REJECTED\n");
        console_write("REASON: ");
        console_write(motion_reason_name(response.reason));
        console_write("\n");
        break;

    case MOTION_RESULT_IGNORED:
        console_write("RESULT: IGNORED\n");
        console_write("REASON: ");
        console_write(motion_reason_name(response.reason));
        console_write("\n");
        break;
    }

    console_write("STATE: ");
    console_write(
        motion_controller_state_name(
            response.previous_state
        )
    );

    console_write(" -> ");

    console_write(
        motion_controller_state_name(
            response.current_state
        )
    );

    console_write("\nACTION: ");
    console_write(
        motion_action_name(response.command.type)
    );

    console_write("\n");
}

static void print_status(const motion_controller_t *controller)
{
    console_write("State: ");
    console_write(motion_controller_state_name(controller->state));

    console_write("\nPosition valid: ");
    console_write(controller->position_valid ? "YES" : "NO");

    console_write("\nPosition (steps): ");
    print_i32(controller->position_steps);

    console_write("\nTarget (steps): ");
    print_i32(controller->target_steps);

    console_write("\n");
}

void console_commands_print_help(void)
{
    console_write(
        "\nCommands:\n"
        "  help          Show commands\n"
        "  status        Show controller state\n"
        "  init          Simulate INIT_OK\n"
        "  home          Request homing\n"
        "  homed         Simulate homing complete\n"
        "  move <steps>  Request absolute move\n"
        "  up            Manual UP\n"
        "  down          Manual DOWN\n"
        "  release       Release manual button\n"
        "  stop          Request deceleration\n"
        "  stopped       Simulate motor stopped\n"
        "  fault         Simulate fault\n"
        "  reset         Reset fault\n"
        "\nSIMULATION ONLY - NO MOTOR OUTPUTS\n"
    );
}

void console_commands_init(
    console_commands_t *console,
    motion_controller_t *controller)
{
    console->controller = controller;
    console->length = 0U;
    console->line[0] = '\0';
}


static void execute_command(console_commands_t *console)
{
    char *line = console->line;
    int32_t value;

    if (strcmp(line, "help") == 0) {
        console_commands_print_help();

    } else if (strcmp(line, "status") == 0) {
        print_status(console->controller);

    } else if (strcmp(line, "init") == 0) {
        send_event(console, MOTION_EVENT_INIT_OK, 0);

    } else if (strcmp(line, "home") == 0) {
        send_event(console, MOTION_EVENT_HOME_REQUEST, 0);

    } else if (strcmp(line, "homed") == 0) {
        send_event(console, MOTION_EVENT_HOMING_COMPLETE, 0);

    } else if (strncmp(line, "move ", 5U) == 0) {
        int32_t steps;
    
        if (!parse_i32(line + 5, &value)) {
            console_write("Invalid millimetre value\n");
    
        } else if (!mm_to_steps(value, &steps)) {
            console_write("Position outside 0-370 mm or invalid scaling\n");
    
        } else {
            send_event(console, MOTION_EVENT_MOVE_ABSOLUTE, steps);
        }
    
    } else if (strncmp(line, "movesteps ", 10U) == 0) {
        if (parse_i32(line + 10, &value)) {
            send_event(console, MOTION_EVENT_MOVE_ABSOLUTE, value);
        } else {
            console_write("Invalid step position\n");
        }
    } else if (strcmp(line, "up") == 0) {
        send_event(console, MOTION_EVENT_UP_PRESSED, 0);

    } else if (strcmp(line, "down") == 0) {
        send_event(console, MOTION_EVENT_DOWN_PRESSED, 0);

    } else if (strcmp(line, "release") == 0) {
        send_event(console, MOTION_EVENT_BUTTON_RELEASED, 0);

    } else if (strcmp(line, "stop") == 0) {
        send_event(console, MOTION_EVENT_STOP, 0);

    } else if (strcmp(line, "reached") == 0) {
        send_event(console, MOTION_EVENT_TARGET_REACHED, 0);

    } else if (strcmp(line, "stopped") == 0) {
        send_event(console, MOTION_EVENT_MOTOR_STOPPED, 0);

    } else if (strcmp(line, "fault") == 0) {
        send_event(console, MOTION_EVENT_FAULT, 0);

    } else if (strcmp(line, "reset") == 0) {
        send_event(console, MOTION_EVENT_FAULT_RESET, 0);

    } else if (*line != '\0') {
        console_write("Unknown command. Type help\n");
    }
}



void console_commands_process_char(
    console_commands_t *console,
    char character)
{
    if (character == '\r' || character == '\n') {

        /* Echo Enter as a newline. */
        console_write("\n");

        if (console->length != 0U) {
            console->line[console->length] = '\0';

            execute_command(console);

            console->length = 0U;
            console->line[0] = '\0';
        }

        //console_write("> ");
        return;
    }

    /* Backspace (ASCII 8) or Delete (ASCII 127). */
    if (character == '\b' || character == 127) {
        if (console->length != 0U) {
            --console->length;
            console->line[console->length] = '\0';

            /* Move left, erase character, move left again. */
            console_write("\b \b");
        }
        return;
    }

    /* Ignore non-printable ASCII characters. */
    if (character < 32 || character > 126) {
        return;
    }

    /* Protect command buffer against overflow. */
    if (console->length >= CONSOLE_COMMAND_MAX_LENGTH - 1U) {
        console->length = 0U;
        console->line[0] = '\0';

        console_write("\nCommand too long\n");
        return;
    }

    /* Append character to the existing command buffer. */
    console->line[console->length++] = character;
    console->line[console->length] = '\0';

    /* Echo the accepted character. */
    char echo[2] = { character, '\0' };
    console_write(echo);
}
