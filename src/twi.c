
#include "twi.h"

#include <avr/io.h>
#include <util/twi.h>

#define TWI_CLOCK_HZ 100000UL
#define TWI_TIMEOUT_LOOPS 30000UL

static uint8_t twi_wait(void)
{
    uint32_t remaining = TWI_TIMEOUT_LOOPS;

    while ((TWCR & _BV(TWINT)) == 0U) {
        if (--remaining == 0UL) {
            return 0U;
        }
    }

    return 1U;
}

static void twi_stop(void)
{
    TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWSTO);
}

void twi_init(void)
{
    TWSR = 0U;  /* Prescaler = 1 */

    TWBR = (uint8_t)(
        ((F_CPU / TWI_CLOCK_HZ) - 16UL) / 2UL
    );

    TWCR = _BV(TWEN);
}

static twi_result_t twi_start_address(uint8_t address7)
{
    uint8_t status;

    if (address7 > 0x7FU) {
        return TWI_ERR_BUS;
    }

    TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);

    if (!twi_wait()) {
        return TWI_ERR_TIMEOUT;
    }

    status = TW_STATUS;

    if (status != TW_START && status != TW_REP_START) {
        return TWI_ERR_START;
    }

    TWDR = (uint8_t)(address7 << 1);
    TWCR = _BV(TWINT) | _BV(TWEN);

    if (!twi_wait()) {
        return TWI_ERR_TIMEOUT;
    }

    status = TW_STATUS;

    if (status == TW_MT_SLA_NACK) {
        return TWI_ERR_NACK;
    }

    if (status != TW_MT_SLA_ACK) {
        return TWI_ERR_BUS;
    }

    return TWI_OK;
}

twi_result_t twi_probe(uint8_t address7)
{
    twi_result_t result = twi_start_address(address7);
    twi_stop();
    return result;
}

twi_result_t twi_write(uint8_t address7, uint8_t data)
{
    twi_result_t result = twi_start_address(address7);

    if (result != TWI_OK) {
        twi_stop();
        return result;
    }

    TWDR = data;
    TWCR = _BV(TWINT) | _BV(TWEN);

    if (!twi_wait()) {
        result = TWI_ERR_TIMEOUT;
    } else {
        uint8_t status = TW_STATUS;

        if (status == TW_MT_DATA_NACK) {
            result = TWI_ERR_NACK;
        } else if (status != TW_MT_DATA_ACK) {
            result = TWI_ERR_BUS;
        }
    }

    twi_stop();
    return result;
}
