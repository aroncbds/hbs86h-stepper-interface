#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdbool.h>
#include <stdint.h>

void console_init(void);
void console_write(const char *text);
void console_write_u16(uint16_t value);

bool console_read_char(char *character);

#endif /* CONSOLE_H */