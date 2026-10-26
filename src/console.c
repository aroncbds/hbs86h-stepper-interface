
#include "console.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#define CONSOLE_RX_SIZE 64U
#define CONSOLE_RX_MASK (CONSOLE_RX_SIZE - 1U)

static SemaphoreHandle_t tx_lock;

static volatile uint8_t rx_buffer[CONSOLE_RX_SIZE];
static volatile uint8_t rx_head;
static volatile uint8_t rx_tail;
static volatile uint8_t rx_overflow;

ISR(USART0_RX_vect)
{
    uint8_t data = UDR0;
    uint8_t next = (uint8_t)(
        (rx_head + 1U) & CONSOLE_RX_MASK
    );

    if (next != rx_tail) {
        rx_buffer[rx_head] = data;
        rx_head = next;
    } else {
        rx_overflow = 1U;
    }
}

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
        (F_CPU + (4UL * BOARD_USART_BAUD)) /
        (8UL * BOARD_USART_BAUD) - 1UL
    );

    rx_head = 0U;
    rx_tail = 0U;
    rx_overflow = 0U;

    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)ubrr;

    UCSR0A = (uint8_t)_BV(U2X0);

    UCSR0C = (uint8_t)(
        _BV(UCSZ01) | _BV(UCSZ00)
    );

    UCSR0B = (uint8_t)(
        _BV(TXEN0) |
        _BV(RXEN0) |
        _BV(RXCIE0)
    );

    tx_lock = xSemaphoreCreateMutex();
    configASSERT(tx_lock != NULL);
}

bool console_read_char(char *character)
{
    bool available = false;

    if (character == NULL) {
        return false;
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (rx_head != rx_tail) {
            *character = (char)rx_buffer[rx_tail];

            rx_tail = (uint8_t)(
                (rx_tail + 1U) & CONSOLE_RX_MASK
            );

            available = true;
        }
    }

    return available;
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
