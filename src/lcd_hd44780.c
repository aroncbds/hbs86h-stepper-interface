
#include "lcd_hd44780.h"

#include <util/delay.h>

#define LCD_RS 0x01U
#define LCD_RW 0x02U
#define LCD_EN 0x04U
#define LCD_BL 0x08U

static twi_result_t lcd_expander_write(uint8_t value)
{
    return twi_write(
        LCD_I2C_ADDRESS,
        (uint8_t)(value | LCD_BL)
    );
}

static twi_result_t lcd_nibble(uint8_t nibble, uint8_t rs)
{
    uint8_t value =
        (uint8_t)((nibble & 0x0FU) << 4);

    twi_result_t result;

    if (rs != 0U) {
        value |= LCD_RS;
    }

    /* EN low */
    result = lcd_expander_write(value);
    if (result != TWI_OK) {
        return result;
    }

    /* EN high */
    result = lcd_expander_write(
        (uint8_t)(value | LCD_EN)
    );
    if (result != TWI_OK) {
        return result;
    }

    _delay_us(2);

    /* Falling edge latches the nibble. */
    result = lcd_expander_write(value);

    _delay_us(50);
    return result;
}

static twi_result_t lcd_send(uint8_t value, uint8_t rs)
{
    twi_result_t result;

    result = lcd_nibble((uint8_t)(value >> 4), rs);
    if (result != TWI_OK) {
        return result;
    }

    return lcd_nibble((uint8_t)(value & 0x0FU), rs);
}

static twi_result_t lcd_command(uint8_t command)
{
    return lcd_send(command, 0U);
}

twi_result_t lcd_clear(void)
{
    twi_result_t result = lcd_command(0x01U);

    if (result == TWI_OK) {
        _delay_ms(2);
    }

    return result;
}

twi_result_t lcd_goto(uint8_t row, uint8_t col)
{
    static const uint8_t row_offsets[4] = {
        0x00U, 0x40U, 0x14U, 0x54U
    };

    if (row >= LCD_ROWS || col >= LCD_COLS) {
        return TWI_ERR_BUS;
    }

    return lcd_command(
        (uint8_t)(0x80U | (row_offsets[row] + col))
    );
}

twi_result_t lcd_write(const char *text)
{
    twi_result_t result;

    if (text == 0) {
        return TWI_ERR_BUS;
    }

    while (*text != '\0') {
        result = lcd_send((uint8_t)*text, 1U);

        if (result != TWI_OK) {
            return result;
        }

        ++text;
    }

    return TWI_OK;
}

twi_result_t lcd_init(void)
{
    twi_result_t result;

    /* LCD power-up delay. */
    _delay_ms(50);

    /*
     * Standard HD44780 4-bit initialization.
     */
    result = lcd_nibble(0x03U, 0U);
    if (result != TWI_OK) return result;
    _delay_ms(5);

    result = lcd_nibble(0x03U, 0U);
    if (result != TWI_OK) return result;
    _delay_us(150);

    result = lcd_nibble(0x03U, 0U);
    if (result != TWI_OK) return result;

    result = lcd_nibble(0x02U, 0U);
    if (result != TWI_OK) return result;

    /* 4-bit, 2-line controller mode, 5x8 font. */
    result = lcd_command(0x28U);
    if (result != TWI_OK) return result;

    /* Display off. */
    result = lcd_command(0x08U);
    if (result != TWI_OK) return result;

    result = lcd_clear();
    if (result != TWI_OK) return result;

    /* Entry mode: increment cursor. */
    result = lcd_command(0x06U);
    if (result != TWI_OK) return result;

    /* Display on, cursor off, blink off. */
    return lcd_command(0x0CU);
}
