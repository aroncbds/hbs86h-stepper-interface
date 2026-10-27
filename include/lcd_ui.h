
#ifndef LCD_UI_H
#define LCD_UI_H

#include <stdint.h>
#include "twi.h"

typedef struct {
    const char *state;
    int32_t position_steps;
    int32_t target_steps;
    uint8_t position_valid;
    const char *status;
} lcd_ui_snapshot_t;

void lcd_ui_init(void);
twi_result_t lcd_ui_update(
    const lcd_ui_snapshot_t *snapshot
);

#endif
