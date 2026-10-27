
#ifndef LCD_HD44780_H
#define LCD_HD44780_H

#include <stdint.h>
#include "twi.h"

#define LCD_I2C_ADDRESS 0x27U

#define LCD_COLS 20U
#define LCD_ROWS 4U

twi_result_t lcd_init(void);
twi_result_t lcd_clear(void);
twi_result_t lcd_goto(uint8_t row, uint8_t col);
twi_result_t lcd_write(const char *text);

#endif
