#ifndef I2C_H
#define I2C_H

#include <stdint.h>

/* Minimal TWI (I2C) master driver for the ATmega328P (SDA = PC4, SCL = PC5). */
void    i2c_init(uint32_t scl_hz);
uint8_t i2c_start(uint8_t addr_rw);   /* addr<<1 | R/W ; returns 0 on success (ACKed) */
void    i2c_stop(void);
uint8_t i2c_write(uint8_t data);      /* returns 0 on success (ACKed) */
uint8_t i2c_read(uint8_t ack);        /* ack = 1 -> send ACK (more bytes follow), 0 -> NACK (last byte) */

#endif
