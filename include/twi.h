
#ifndef TWI_H
#define TWI_H

#include <stdint.h>

typedef enum {
    TWI_OK = 0,
    TWI_ERR_TIMEOUT,
    TWI_ERR_START,
    TWI_ERR_NACK,
    TWI_ERR_BUS
} twi_result_t;

void twi_init(void);
twi_result_t twi_write(uint8_t address7, uint8_t data);
twi_result_t twi_probe(uint8_t address7);

#endif
