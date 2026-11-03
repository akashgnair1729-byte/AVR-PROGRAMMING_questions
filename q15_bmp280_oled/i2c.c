#include <avr/io.h>
#include "i2c.h"

#define TW_STATUS (TWSR & 0xF8)

/* Wait for TWINT with a timeout so a missing device cannot hang the MCU. Returns 1 on success. */
static uint8_t wait_twint(void)
{
    uint16_t timeout = 60000;
    while (!(TWCR & (1 << TWINT))) {
        if (--timeout == 0)
            return 0;
    }
    return 1;
}

void i2c_init(uint32_t scl_hz)
{
    TWSR = 0;                                        /* prescaler = 1 */
    TWBR = (uint8_t)((F_CPU / scl_hz - 16) / 2);
    TWCR = (1 << TWEN);
}

uint8_t i2c_start(uint8_t addr_rw)
{
    uint8_t status;

    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);        /* START / repeated START */
    if (!wait_twint())
        return 1;
    status = TW_STATUS;
    if (status != 0x08 && status != 0x10)                    /* START / REP START sent */
        return 1;

    TWDR = addr_rw;
    TWCR = (1 << TWINT) | (1 << TWEN);
    if (!wait_twint())
        return 1;
    status = TW_STATUS;
    if (status != 0x18 && status != 0x40) {                  /* SLA+W / SLA+R ACKed */
        i2c_stop();
        return 1;
    }
    return 0;
}

void i2c_stop(void)
{
    uint16_t timeout = 60000;
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
    while ((TWCR & (1 << TWSTO)) && --timeout)
        ;
}

uint8_t i2c_write(uint8_t data)
{
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    if (!wait_twint())
        return 1;
    return (TW_STATUS != 0x28);                              /* data ACKed */
}

uint8_t i2c_read(uint8_t ack)
{
    TWCR = (1 << TWINT) | (1 << TWEN) | (ack ? (1 << TWEA) : 0);
    if (!wait_twint())
        return 0xFF;
    return TWDR;
}
