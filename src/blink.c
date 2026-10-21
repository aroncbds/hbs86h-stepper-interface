#include "led.h"

#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>

/*
 * Toolchain smoke test. The board LED is on for one second, then off for
 * one second. On the Arduino Mega that is the "L" LED, PB7, digital 13.
 *
 * Some Mega USB bootloaders leave the watchdog running, and a watchdog
 * reset sets WDRF, which forces the watchdog back on. Clear the flag
 * before disabling it so the one-second delays are not cut short.
 */
int main(void)
{
    MCUSR = 0;
    wdt_disable();
    led_init();

    for (;;) {
        led_set(true);
        _delay_ms(1000.0);
        led_set(false);
        _delay_ms(1000.0);
    }
}
