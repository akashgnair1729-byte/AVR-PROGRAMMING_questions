#include <avr/io.h>
#include <util/delay.h>
#include "bmp280.h"
#include "i2c.h"

#define REG_CALIB   0x88
#define REG_ID      0xD0
#define REG_RESET   0xE0
#define REG_STATUS  0xF3
#define REG_CTRL    0xF4
#define REG_CONFIG  0xF5
#define REG_DATA    0xF7

static uint8_t dev_addr = 0x76;

static uint16_t dig_T1;
static int16_t  dig_T2, dig_T3;
static uint16_t dig_P1;
static int16_t  dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
static int32_t  t_fine;

/* ---------- low-level register access ---------- */
static uint8_t read_regs(uint8_t reg, uint8_t *buf, uint8_t len)
{
    if (i2c_start((uint8_t)(dev_addr << 1)))
        return 1;
    if (i2c_write(reg)) {
        i2c_stop();
        return 1;
    }
    if (i2c_start((uint8_t)((dev_addr << 1) | 1)))           /* repeated START, read mode */
        return 1;
    for (uint8_t i = 0; i < len; i++)
        buf[i] = i2c_read(i < len - 1);                      /* NACK on the last byte */
    i2c_stop();
    return 0;
}

static uint8_t write_reg(uint8_t reg, uint8_t value)
{
    if (i2c_start((uint8_t)(dev_addr << 1)))
        return 1;
    uint8_t err = i2c_write(reg) | i2c_write(value);
    i2c_stop();
    return err;
}

/* ---------- init ---------- */
uint8_t bmp280_init(void)
{
    uint8_t id = 0, c[24];
    static const uint8_t addrs[2] = { 0x76, 0x77 };
    uint8_t found = 0;

    for (uint8_t i = 0; i < 2 && !found; i++) {
        dev_addr = addrs[i];
        if (read_regs(REG_ID, &id, 1) == 0 &&
            (id == 0x58 || id == 0x56 || id == 0x57 || id == 0x60))   /* BMP280 (or BME280) */
            found = 1;
    }
    if (!found)
        return 1;

    write_reg(REG_RESET, 0xB6);                              /* soft reset */
    _delay_ms(10);

    if (read_regs(REG_CALIB, c, 24))
        return 1;

    dig_T1 = (uint16_t)(c[1] << 8 | c[0]);
    dig_T2 = (int16_t)(c[3] << 8 | c[2]);
    dig_T3 = (int16_t)(c[5] << 8 | c[4]);
    dig_P1 = (uint16_t)(c[7] << 8 | c[6]);
    dig_P2 = (int16_t)(c[9] << 8 | c[8]);
    dig_P3 = (int16_t)(c[11] << 8 | c[10]);
    dig_P4 = (int16_t)(c[13] << 8 | c[12]);
    dig_P5 = (int16_t)(c[15] << 8 | c[14]);
    dig_P6 = (int16_t)(c[17] << 8 | c[16]);
    dig_P7 = (int16_t)(c[19] << 8 | c[18]);
    dig_P8 = (int16_t)(c[21] << 8 | c[20]);
    dig_P9 = (int16_t)(c[23] << 8 | c[22]);

    write_reg(REG_CONFIG, 0x00);                             /* no filter, no standby (forced mode) */
    return 0;
}

/* ---------- compensation (integer formulas from the Bosch datasheet) ---------- */
static int32_t compensate_T(int32_t adc_T)
{
    int32_t var1, var2;

    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) *
              ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    t_fine = var1 + var2;
    return (t_fine * 5 + 128) >> 8;                          /* 0.01 deg C */
}

static uint32_t compensate_P(int32_t adc_P)
{
    int32_t var1, var2;
    uint32_t p;

    var1 = (t_fine >> 1) - (int32_t)64000;
    var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int32_t)dig_P6);
    var2 = var2 + ((var1 * ((int32_t)dig_P5)) << 1);
    var2 = (var2 >> 2) + (((int32_t)dig_P4) << 16);
    var1 = ((((int32_t)dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) +
            ((((int32_t)dig_P2) * var1) >> 1)) >> 18;
    var1 = ((((32768 + var1)) * ((int32_t)dig_P1)) >> 15);
    if (var1 == 0)
        return 0;                                            /* avoid division by zero */
    p = (((uint32_t)(((int32_t)1048576) - adc_P) - (uint32_t)(var2 >> 12))) * 3125;
    if (p < 0x80000000UL)
        p = (p << 1) / ((uint32_t)var1);
    else
        p = (p / (uint32_t)var1) * 2;
    var1 = (((int32_t)dig_P9) * ((int32_t)(((p >> 3) * (p >> 3)) >> 13))) >> 12;
    var2 = (((int32_t)(p >> 2)) * ((int32_t)dig_P8)) >> 13;
    p = (uint32_t)((int32_t)p + ((var1 + var2 + dig_P7) >> 4));
    return p;                                                /* Pa */
}

/* ---------- measurement ---------- */
uint8_t bmp280_read(int32_t *temp_c100, uint32_t *press_pa)
{
    uint8_t d[6], status;

    /* osrs_t = x2, osrs_p = x16, mode = forced  ->  0b010_101_01 */
    if (write_reg(REG_CTRL, 0x55))
        return 1;

    _delay_ms(5);
    for (uint8_t i = 0; i < 30; i++) {                       /* wait while "measuring" bit is set */
        if (read_regs(REG_STATUS, &status, 1))
            return 1;
        if (!(status & 0x08))
            break;
        _delay_ms(5);
    }

    if (read_regs(REG_DATA, d, 6))
        return 1;

    int32_t adc_P = ((int32_t)d[0] << 12) | ((int32_t)d[1] << 4) | (d[2] >> 4);
    int32_t adc_T = ((int32_t)d[3] << 12) | ((int32_t)d[4] << 4) | (d[5] >> 4);

    *temp_c100 = compensate_T(adc_T);                        /* must run first: sets t_fine */
    *press_pa  = compensate_P(adc_P);
    return 0;
}
