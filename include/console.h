#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>

void console_init(void);
void console_write(const char *text);
void console_write_u16(uint16_t value);

#endif /* CONSOLE_H */
