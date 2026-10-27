
#ifndef MOTION_LIMITS_H
#define MOTION_LIMITS_H

#include <stdint.h>

/* Physical travel limits, in millimetres. */
#define MOTION_TRAVEL_MIN_MM 0L
#define MOTION_TRAVEL_MAX_MM 370L

/*
 * Example only!
 * Must match the HBS86H pulse/revolution setting.
 */
#define MOTION_PULSES_PER_REV 1600L
#define MOTION_SCREW_PITCH_MM 5L

#define MOTION_STEPS_PER_MM \
    (MOTION_PULSES_PER_REV / MOTION_SCREW_PITCH_MM)

#define MOTION_MIN_STEPS \
    (MOTION_TRAVEL_MIN_MM * MOTION_STEPS_PER_MM)

#define MOTION_MAX_STEPS \
    (MOTION_TRAVEL_MAX_MM * MOTION_STEPS_PER_MM)

static inline uint8_t motion_target_in_range(
    int32_t target_steps)
{
    return target_steps >= MOTION_MIN_STEPS &&
           target_steps <= MOTION_MAX_STEPS;
}

#endif
