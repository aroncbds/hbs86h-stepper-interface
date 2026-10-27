
#include "lcd_ui.h"
#include "lcd_hd44780.h"

#include <stdint.h>
#include <string.h>

#define UI_ROWS 4U
#define UI_COLS 20U

static char previous[UI_ROWS][UI_COLS];
static uint8_t initialized = 0U;

static void clear_line(char *line)
{
    uint8_t i;

    for (i = 0U; i < UI_COLS; ++i) {
        line[i] = ' ';
    }
}

static void append_text(
    char *line,
    uint8_t *column,
    const char *text)
{
    if (text == 0) {
        return;
    }

    while (*text != '\0' && *column < UI_COLS) {
        line[(*column)++] = *text++;
    }
}

static void append_i32(
    char *line,
    uint8_t *column,
    int32_t value)
{
    char digits[11];
    uint8_t count = 0U;
    uint32_t magnitude;

    if (value < 0) {
        if (*column < UI_COLS) {
            line[(*column)++] = '-';
        }

        /* Handles INT32_MIN without signed overflow. */
        magnitude = (uint32_t)(-(value + 1)) + 1UL;
    } else {
        magnitude = (uint32_t)value;
    }

    do {
        digits[count++] =
            (char)('0' + (magnitude % 10UL));
        magnitude /= 10UL;
    } while (magnitude != 0UL);

    while (count != 0U && *column < UI_COLS) {
        line[(*column)++] = digits[--count];
    }
}

static void build_lines(
    char lines[UI_ROWS][UI_COLS],
    const lcd_ui_snapshot_t *snapshot)
{
    uint8_t row;
    uint8_t col;

    for (row = 0U; row < UI_ROWS; ++row) {
        clear_line(lines[row]);
    }

    col = 0U;
    append_text(lines[0], &col, "STATE: ");
    append_text(lines[0], &col, snapshot->state);

    col = 0U;
    append_text(lines[1], &col, "POS: ");

    if (snapshot->position_valid) {
        append_i32(
            lines[1], &col,
            snapshot->position_steps
        );
    } else {
        append_text(lines[1], &col, "UNKNOWN");
    }

    col = 0U;
    append_text(lines[2], &col, "TARGET: ");

    if (snapshot->position_valid) {
        append_i32(
            lines[2], &col,
            snapshot->target_steps
        );
    } else {
        append_text(lines[2], &col, "UNKNOWN");
    }

    col = 0U;
    append_text(lines[3], &col, "STATUS: ");
    append_text(lines[3], &col, snapshot->status);
}

void lcd_ui_init(void)
{
    initialized = 0U;
}

twi_result_t lcd_ui_update(
    const lcd_ui_snapshot_t *snapshot)
{
    char lines[UI_ROWS][UI_COLS];
    uint8_t row;
    uint8_t col;
    twi_result_t result;

    if (snapshot == 0) {
        return TWI_ERR_BUS;
    }

    build_lines(lines, snapshot);

    for (row = 0U; row < UI_ROWS; ++row) {
        col = 0U;

        while (col < UI_COLS) {
            uint8_t start;
            char segment[UI_COLS + 1U];
            uint8_t length = 0U;

            if (initialized &&
                lines[row][col] == previous[row][col]) {
                ++col;
                continue;
            }

            start = col;

            /*
             * Group adjacent changed characters
             * into one LCD write operation.
             */
            while (col < UI_COLS &&
                   (!initialized ||
                    lines[row][col] != previous[row][col])) {
                segment[length++] = lines[row][col];
                ++col;
            }

            segment[length] = '\0';

            result = lcd_goto(row, start);
            if (result != TWI_OK) {
                initialized = 0U;
                return result;
            }

            result = lcd_write(segment);
            if (result != TWI_OK) {
                initialized = 0U;
                return result;
            }
        }
    }

    memcpy(previous, lines, sizeof(previous));
    initialized = 1U;

    return TWI_OK;
}
