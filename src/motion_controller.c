
#include "motion_controller.h"

#include <stddef.h>

static motion_command_t command(
    motion_action_t type,
    int32_t value)
{
    motion_command_t result = { type, value };
    return result;
}

static void set_state(
    motion_controller_t *controller,
    motion_state_t next)
{
    controller->previous_state = controller->state;
    controller->state = next;
}

void motion_controller_init(motion_controller_t *controller)
{
    if (controller == NULL) {
        return;
    }

    controller->state = MOTION_STATE_INIT;
    controller->previous_state = MOTION_STATE_INIT;

    controller->position_valid = false;
    controller->position_steps = 0;
    controller->target_steps = 0;
    controller->moving_up = false;
}

motion_command_t motion_controller_process(
    motion_controller_t *controller,
    motion_message_t message)
{
    if (controller == NULL) {
        return command(MOTION_ACTION_FAULT, 0);
    }

    /* Faults have priority over ordinary commands. */
    if (message.type == MOTION_EVENT_FAULT ||
        message.type == MOTION_EVENT_INIT_FAILED) {
        controller->position_valid = false;
        set_state(controller, MOTION_STATE_FAULT);
        return command(MOTION_ACTION_FAULT, 0);
    }

    switch (controller->state) {

    case MOTION_STATE_INIT:
        if (message.type == MOTION_EVENT_INIT_OK) {
            set_state(controller, MOTION_STATE_NOT_HOMED);
        }
        break;

    case MOTION_STATE_NOT_HOMED:
    case MOTION_STATE_IDLE:

        if (message.type == MOTION_EVENT_HOME_REQUEST) {
            set_state(controller, MOTION_STATE_HOMING);
            return command(MOTION_ACTION_START_HOMING, 0);
        }

        if (message.type == MOTION_EVENT_UP_PRESSED) {
            controller->moving_up = true;
            set_state(controller, MOTION_STATE_MANUAL);
            return command(MOTION_ACTION_MOVE_UP, 0);
        }

        if (message.type == MOTION_EVENT_DOWN_PRESSED) {
            controller->moving_up = false;
            set_state(controller, MOTION_STATE_MANUAL);
            return command(MOTION_ACTION_MOVE_DOWN, 0);
        }

        if (message.type == MOTION_EVENT_MOVE_ABSOLUTE &&
            controller->position_valid) {

            controller->target_steps = message.value;
            set_state(controller, MOTION_STATE_AUTO);

            return command(
                MOTION_ACTION_MOVE_ABSOLUTE,
                message.value
            );
        }
        break;

    case MOTION_STATE_MANUAL:

        if (message.type == MOTION_EVENT_BUTTON_RELEASED ||
            message.type == MOTION_EVENT_STOP) {

            set_state(controller, MOTION_STATE_DECELERATING);
            return command(MOTION_ACTION_DECELERATE, 0);
        }
        break;

    case MOTION_STATE_AUTO:

        if (message.type == MOTION_EVENT_STOP) {
            set_state(controller, MOTION_STATE_DECELERATING);
            return command(MOTION_ACTION_DECELERATE, 0);
        }

        if (message.type == MOTION_EVENT_TARGET_REACHED) {
            set_state(controller, MOTION_STATE_DECELERATING);
            return command(MOTION_ACTION_DECELERATE, 0);
        }
        break;

    case MOTION_STATE_HOMING:

        if (message.type == MOTION_EVENT_STOP) {
            controller->position_valid = false;
            set_state(controller, MOTION_STATE_DECELERATING);
            return command(MOTION_ACTION_DECELERATE, 0);
        }

        if (message.type == MOTION_EVENT_HOMING_COMPLETE) {
            controller->position_valid = true;
            set_state(controller, MOTION_STATE_IDLE);
            return command(MOTION_ACTION_STOPPED, 0);
        }
        break;

    case MOTION_STATE_DECELERATING:

        if (message.type == MOTION_EVENT_MOTOR_STOPPED) {
            set_state(
                controller,
                controller->position_valid
                    ? MOTION_STATE_IDLE
                    : MOTION_STATE_NOT_HOMED
            );

            return command(MOTION_ACTION_STOPPED, 0);
        }
        break;

    case MOTION_STATE_FAULT:

        if (message.type == MOTION_EVENT_FAULT_RESET) {
            set_state(controller, MOTION_STATE_NOT_HOMED);
        }
        break;

    default:
        set_state(controller, MOTION_STATE_FAULT);
        return command(MOTION_ACTION_FAULT, 0);
    }

    return command(MOTION_ACTION_NONE, 0);
}

motion_state_t motion_controller_get_state(
    const motion_controller_t *controller)
{
    return controller != NULL
        ? controller->state
        : MOTION_STATE_FAULT;
}

const char *motion_controller_state_name(
    motion_state_t state)
{
    switch (state) {
    case MOTION_STATE_INIT:         return "INIT";
    case MOTION_STATE_NOT_HOMED:    return "NOT_HOMED";
    case MOTION_STATE_IDLE:         return "IDLE";
    case MOTION_STATE_MANUAL:       return "MANUAL";
    case MOTION_STATE_AUTO:         return "AUTO";
    case MOTION_STATE_HOMING:       return "HOMING";
    case MOTION_STATE_DECELERATING: return "DECELERATING";
    case MOTION_STATE_FAULT:        return "FAULT";
    default:                        return "UNKNOWN";
    }
}
