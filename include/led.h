#ifndef LED_H
#define LED_H

#include <stdbool.h>

#ifndef BOARD_HAS_LED
#define BOARD_HAS_LED 0
#endif

void led_init(void);
void led_set(bool on);
void led_toggle(void);

#endif /* LED_H */
