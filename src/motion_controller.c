#include "motion_controller.h"
#include "motion_limits.h"

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
        
            /*
             * Reject absolute movements outside the configured travel limits.
             *
             * do not change target_steps or state when the target is invalid..
             */
            if (!motion_target_in_range(message.value)) {
                return command(MOTION_ACTION_NONE, 0);
            }
        
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
            /* let the "reached" event update the position to the target value for a more realistic simulation until we get the motor connected */
            controller->position_steps = controller->target_steps;

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


motion_response_t motion_controller_process_ex(
    motion_controller_t *controller,
    motion_message_t message)
{
    motion_response_t response = {
        .result = MOTION_RESULT_REJECTED,
        .reason = MOTION_REASON_INVALID_STATE,
        .previous_state = MOTION_STATE_FAULT,
        .current_state = MOTION_STATE_FAULT,
        .command = { MOTION_ACTION_NONE, 0 }
    };

    if (controller == NULL) {
        response.command.type = MOTION_ACTION_FAULT;
        return response;
    }

    response.previous_state = controller->state;

    /*
     * existing state machine remains the single authority for transitions and commands
     */
    response.command =
        motion_controller_process(controller, message);

    response.current_state = controller->state;

    /*
     * An action or state transition means the event was accepted by the state machine
     */
    if (response.command.type != MOTION_ACTION_NONE ||
        response.previous_state != response.current_state) {

        response.result = MOTION_RESULT_ACCEPTED;
        response.reason = MOTION_REASON_NONE;

    /*
     * Absolute positioning requires a valid reference
     */
    } else if (
        message.type == MOTION_EVENT_MOVE_ABSOLUTE &&
        !controller->position_valid) {

        response.result = MOTION_RESULT_REJECTED;
        response.reason = MOTION_REASON_NOT_HOMED;

    /*
     * Reject absolute targets outside the configured
     * travel limits.
     *
     * This applies only when the state machine would
     * otherwise allow absolute positioning.
     */
    } else if (
        message.type == MOTION_EVENT_MOVE_ABSOLUTE &&
        controller->position_valid &&
        (controller->state == MOTION_STATE_IDLE ||
         controller->state == MOTION_STATE_NOT_HOMED) &&
        !motion_target_in_range(message.value)) {

        response.result = MOTION_RESULT_REJECTED;
        response.reason = MOTION_REASON_OUT_OF_RANGE;

    /*
     * INIT_OK has already been processed
     */
    } else if (
        message.type == MOTION_EVENT_INIT_OK &&
        controller->state == MOTION_STATE_NOT_HOMED) {

        response.result = MOTION_RESULT_IGNORED;
        response.reason = MOTION_REASON_ALREADY_IN_STATE;

    /*
     * Homing is already in progress
     */
    } else if (
        message.type == MOTION_EVENT_HOME_REQUEST &&
        controller->state == MOTION_STATE_HOMING) {

        response.result = MOTION_RESULT_IGNORED;
        response.reason = MOTION_REASON_ALREADY_IN_STATE;
    }

    return response;
}

const char *motion_event_name(motion_event_t event)
{
    switch (event) {
    case MOTION_EVENT_INIT_OK: return "INIT_OK";
    case MOTION_EVENT_INIT_FAILED: return "INIT_FAILED";
    case MOTION_EVENT_UP_PRESSED: return "UP_PRESSED";
    case MOTION_EVENT_DOWN_PRESSED: return "DOWN_PRESSED";
    case MOTION_EVENT_BUTTON_RELEASED: return "BUTTON_RELEASED";
    case MOTION_EVENT_MOVE_ABSOLUTE: return "MOVE_ABSOLUTE";
    case MOTION_EVENT_HOME_REQUEST: return "HOME_REQUEST";
    case MOTION_EVENT_STOP: return "STOP";
    case MOTION_EVENT_TARGET_REACHED: return "TARGET_REACHED";
    case MOTION_EVENT_MOTOR_STOPPED: return "MOTOR_STOPPED";
    case MOTION_EVENT_HOMING_COMPLETE: return "HOMING_COMPLETE";
    case MOTION_EVENT_FAULT: return "FAULT";
    case MOTION_EVENT_FAULT_RESET: return "FAULT_RESET";
    default: return "UNKNOWN_EVENT";
    }
}

const char *motion_action_name(motion_action_t action)
{
    switch (action) {
    case MOTION_ACTION_NONE: return "NONE";
    case MOTION_ACTION_MOVE_UP: return "MOVE_UP";
    case MOTION_ACTION_MOVE_DOWN: return "MOVE_DOWN";
    case MOTION_ACTION_MOVE_ABSOLUTE: return "MOVE_ABSOLUTE";
    case MOTION_ACTION_START_HOMING: return "START_HOMING";
    case MOTION_ACTION_DECELERATE: return "DECELERATE";
    case MOTION_ACTION_STOPPED: return "STOPPED";
    case MOTION_ACTION_FAULT: return "FAULT";
    default: return "UNKNOWN_ACTION";
    }
}

const char *motion_reason_name(motion_reason_t reason)
{
    switch (reason) {
    case MOTION_REASON_NONE:
        return "NONE";

    case MOTION_REASON_INVALID_STATE:
        return "Invalid state";

    case MOTION_REASON_NOT_HOMED:
        return "Position not referenced";

    case MOTION_REASON_ALREADY_IN_STATE:
        return "Already in requested state";

    case MOTION_REASON_OUT_OF_RANGE:
        return "Target outside travel limits";

    default:
        return "Unknown reason";
    }
}
