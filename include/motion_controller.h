
#ifndef MOTION_CONTROLLER_H
#define MOTION_CONTROLLER_H

#include <stdbool.h>
#include <stdint.h>

/* Main controller states */
typedef enum {
    MOTION_STATE_INIT = 0,
    MOTION_STATE_NOT_HOMED,
    MOTION_STATE_IDLE,
    MOTION_STATE_MANUAL,
    MOTION_STATE_AUTO,
    MOTION_STATE_HOMING,
    MOTION_STATE_DECELERATING,
    MOTION_STATE_FAULT
} motion_state_t;

/* Events received by the controller */
typedef enum {
    MOTION_EVENT_INIT_OK = 0,
    MOTION_EVENT_INIT_FAILED,
    MOTION_EVENT_UP_PRESSED,
    MOTION_EVENT_DOWN_PRESSED,
    MOTION_EVENT_BUTTON_RELEASED,
    MOTION_EVENT_MOVE_ABSOLUTE,
    MOTION_EVENT_HOME_REQUEST,
    MOTION_EVENT_STOP,
    MOTION_EVENT_TARGET_REACHED,
    MOTION_EVENT_MOTOR_STOPPED,
    MOTION_EVENT_HOMING_COMPLETE,
    MOTION_EVENT_FAULT,
    MOTION_EVENT_FAULT_RESET
} motion_event_t;

/* Commands issued by the controller */
typedef enum {
    MOTION_ACTION_NONE = 0,
    MOTION_ACTION_MOVE_UP,
    MOTION_ACTION_MOVE_DOWN,
    MOTION_ACTION_MOVE_ABSOLUTE,
    MOTION_ACTION_START_HOMING,
    MOTION_ACTION_DECELERATE,
    MOTION_ACTION_STOPPED,
    MOTION_ACTION_FAULT
} motion_action_t;

typedef struct {
    motion_event_t type;
    int32_t value;
} motion_message_t;

typedef struct {
    motion_action_t type;
    int32_t value;
} motion_command_t;

typedef struct {
    motion_state_t state;
    motion_state_t previous_state;

    bool position_valid;
    int32_t position_steps;
    int32_t target_steps;

    bool moving_up;
} motion_controller_t;

/* Public API */
void motion_controller_init(motion_controller_t *controller);

motion_command_t motion_controller_process(
    motion_controller_t *controller,
    motion_message_t message
);

motion_state_t motion_controller_get_state(
    const motion_controller_t *controller
);

const char *motion_controller_state_name(
    motion_state_t state
);

#endif /* MOTION_CONTROLLER_H */
