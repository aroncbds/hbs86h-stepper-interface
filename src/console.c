#include "console.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include <avr/io.h>

/*
 * USART0, 8N1, U2X. On the Arduino Mega this is PE0/PE1, wired to the
 * ATmega16U2 USB-serial bridge (Arduino pins 0 and 1). On the ATmega1284P
 * the same registers are PD0/PD1. The peripheral takes the pins when TXEN
 * and RXEN are set, so the GPIO direction registers are left alone.
 *
 * 16 MHz / 115200 baud with U2X is about 2.1% high (UBRR = 16).
 */

static SemaphoreHandle_t tx_lock;

static void console_lock(void)
{
    if ((tx_lock != NULL) &&
        (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)) {
        (void)xSemaphoreTake(tx_lock, portMAX_DELAY);
    }
}

static void console_unlock(void)
{
    if ((tx_lock != NULL) &&
        (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)) {
        (void)xSemaphoreGive(tx_lock);
    }
}

static void console_putc(char ch)
{
    while ((UCSR0A & (uint8_t)_BV(UDRE0)) == 0U) {
    }
    UDR0 = (uint8_t)ch;
}

void console_init(void)
{
#ifndef BOARD_USART_BAUD
#error BOARD_USART_BAUD is not set
#endif
    const uint16_t ubrr = (uint16_t)(
        (F_CPU + (4UL * BOARD_USART_BAUD)) / (8UL * BOARD_USART_BAUD) - 1UL);

    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)ubrr;
    UCSR0A = (uint8_t)_BV(U2X0);
    UCSR0B = (uint8_t)(_BV(TXEN0) | _BV(RXEN0));
    UCSR0C = (uint8_t)(_BV(UCSZ01) | _BV(UCSZ00));

    tx_lock = xSemaphoreCreateMutex();
    configASSERT(tx_lock != NULL);
}

void console_write(const char *text)
{
    console_lock();
    while (*text != '\0') {
        if (*text == '\n') {
            console_putc('\r');
        }
        console_putc(*text);
        text++;
    }
    console_unlock();
}

void console_write_u16(uint16_t value)
{
    char buf[6];
    uint8_t i = 5U;

    buf[5] = '\0';
    do {
        buf[--i] = (char)('0' + (value % 10U));
        value = (uint16_t)(value / 10U);
    } while (value != 0U);

    console_write(&buf[i]);
}
