
#ifndef MOTION_LIMITS_H
#define MOTION_LIMITS_H

#include <stdint.h>

/*
 * PALLET CUTTER - TABLE GEOMETRY
 *
 * All absolute positions represent the distance between the table surface and the knife edge.
 *
 * Values might need to be adjusted as the machine evolves...
 */

/* Physical travel limits, in millimetres */
#define MOTION_TRAVEL_MIN_MM      120L
#define MOTION_TRAVEL_MAX_MM      370L

/*
 * Motor and mechanical configuration.
 *
 * IMPORTANT:
 * 1600 pulses/rev is an example.
 * Must match the actual HBS86H configuration.
 */
#define MOTION_PULSES_PER_REV     1600L
#define MOTION_SCREW_PITCH_MM     5L

/* Derived values - do not edit manually */
#define MOTION_STEPS_PER_MM \
    (MOTION_PULSES_PER_REV / MOTION_SCREW_PITCH_MM)

#define MOTION_MIN_STEPS \
    (MOTION_TRAVEL_MIN_MM * MOTION_STEPS_PER_MM)

#define MOTION_MAX_STEPS \
    (MOTION_TRAVEL_MAX_MM * MOTION_STEPS_PER_MM)

/*
 * Validate an absolute target position.
 */
static inline uint8_t motion_target_in_range(
    int32_t target_steps)
{
    return (target_steps >= MOTION_MIN_STEPS) &&
           (target_steps <= MOTION_MAX_STEPS);
}

#define MOTION_HOME_POSITION_MM  MOTION_TRAVEL_MAX_MM

#define MOTION_HOME_STEPS \
    (MOTION_HOME_POSITION_MM * MOTION_STEPS_PER_MM)

#endif /* MOTION_LIMITS_H */
