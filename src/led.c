#include "led.h"

#include <avr/io.h>
#include <stdint.h>

#if BOARD_HAS_LED
#ifndef BOARD_LED_DDR
#error BOARD_LED_DDR is not set
#endif
#ifndef BOARD_LED_PORT
#error BOARD_LED_PORT is not set
#endif
#ifndef BOARD_LED_PIN
#error BOARD_LED_PIN is not set
#endif
#ifndef BOARD_LED_BIT
#error BOARD_LED_BIT is not set
#endif
#endif

void led_init(void)
{
#if BOARD_HAS_LED
    BOARD_LED_DDR |= (uint8_t)_BV(BOARD_LED_BIT);
    BOARD_LED_PORT &= (uint8_t)~_BV(BOARD_LED_BIT);
#endif
}

void led_set(bool on)
{
#if BOARD_HAS_LED
    if (on) {
        BOARD_LED_PORT |= (uint8_t)_BV(BOARD_LED_BIT);
    } else {
        BOARD_LED_PORT &= (uint8_t)~_BV(BOARD_LED_BIT);
    }
#else
    (void)on;
#endif
}

void led_toggle(void)
{
#if BOARD_HAS_LED
    /* Writing a 1 to the PIN register toggles the matching PORT bit. */
    BOARD_LED_PIN |= (uint8_t)_BV(BOARD_LED_BIT);
#endif
}
