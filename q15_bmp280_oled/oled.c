#include <avr/io.h>
#include <avr/pgmspace.h>
#include <ctype.h>
#include "i2c.h"
#include "oled.h"

/* Many SSD1306 modules answer at 0x3C. If yours uses 0x3D change it here.
 * (1.3" modules that use an SH1106 chip also need a +2 column offset.) */
#define OLED_ADDR 0x3C

/* Characters supported by the font, in the same order as FONT[]. */
static const char CHARSET[] PROGMEM = " .-:%*0123456789ABCDEFHMNOPRSTU";

/* 5x7 font, one byte per column, bit 0 = top row. */
static const uint8_t FONT[][5] PROGMEM = {
    {0x00, 0x00, 0x00, 0x00, 0x00},   /* space */
    {0x00, 0x60, 0x60, 0x00, 0x00},   /* . */
    {0x08, 0x08, 0x08, 0x08, 0x08},   /* - */
    {0x00, 0x36, 0x36, 0x00, 0x00},   /* : */
    {0x23, 0x13, 0x08, 0x64, 0x62},   /* % */
    {0x00, 0x06, 0x09, 0x09, 0x06},   /* * -> degree sign */
    {0x3E, 0x51, 0x49, 0x45, 0x3E},   /* 0 */
    {0x00, 0x42, 0x7F, 0x40, 0x00},   /* 1 */
    {0x42, 0x61, 0x51, 0x49, 0x46},   /* 2 */
    {0x21, 0x41, 0x45, 0x4B, 0x31},   /* 3 */
    {0x18, 0x14, 0x12, 0x7F, 0x10},   /* 4 */
    {0x27, 0x45, 0x45, 0x45, 0x39},   /* 5 */
    {0x3C, 0x4A, 0x49, 0x49, 0x30},   /* 6 */
    {0x01, 0x71, 0x09, 0x05, 0x03},   /* 7 */
    {0x36, 0x49, 0x49, 0x49, 0x36},   /* 8 */
    {0x06, 0x49, 0x49, 0x29, 0x1E},   /* 9 */
    {0x7E, 0x11, 0x11, 0x11, 0x7E},   /* A */
    {0x7F, 0x49, 0x49, 0x49, 0x36},   /* B */
    {0x3E, 0x41, 0x41, 0x41, 0x22},   /* C */
    {0x7F, 0x41, 0x41, 0x22, 0x1C},   /* D */
    {0x7F, 0x49, 0x49, 0x49, 0x41},   /* E */
    {0x7F, 0x09, 0x09, 0x09, 0x01},   /* F */
    {0x7F, 0x08, 0x08, 0x08, 0x7F},   /* H */
    {0x7F, 0x02, 0x0C, 0x02, 0x7F},   /* M */
    {0x7F, 0x04, 0x08, 0x10, 0x7F},   /* N */
    {0x3E, 0x41, 0x41, 0x41, 0x3E},   /* O */
    {0x7F, 0x09, 0x09, 0x09, 0x06},   /* P */
    {0x7F, 0x09, 0x19, 0x29, 0x46},   /* R */
    {0x46, 0x49, 0x49, 0x49, 0x31},   /* S */
    {0x01, 0x01, 0x7F, 0x01, 0x01},   /* T */
    {0x3F, 0x40, 0x40, 0x40, 0x3F},   /* U */
};

/* ---------- low-level ---------- */
static uint8_t oled_cmd(uint8_t c)
{
    if (i2c_start((uint8_t)(OLED_ADDR << 1)))
        return 1;
    uint8_t err = i2c_write(0x00) | i2c_write(c);            /* 0x00 = command stream */
    i2c_stop();
    return err;
}

static void set_cursor(uint8_t page, uint8_t col)
{
    oled_cmd((uint8_t)(0xB0 | (page & 0x07)));
    oled_cmd((uint8_t)(0x00 | (col & 0x0F)));
    oled_cmd((uint8_t)(0x10 | (col >> 4)));
}

static uint8_t glyph_col(char ch, uint8_t i)
{
    ch = (char)toupper((unsigned char)ch);
    for (uint8_t n = 0; pgm_read_byte(&CHARSET[n]) != '\0'; n++) {
        if ((char)pgm_read_byte(&CHARSET[n]) == ch)
            return pgm_read_byte(&FONT[n][i]);
    }
    return 0x00;                                             /* unsupported -> blank */
}

/* Double every bit of a 4-bit value -> 8 bits (used for 2x vertical scaling). */
static uint8_t spread_nibble(uint8_t n)
{
    uint8_t r = 0;
    for (uint8_t i = 0; i < 4; i++)
        if (n & (1 << i))
            r |= (uint8_t)(3 << (2 * i));
    return r;
}

/* ---------- public API ---------- */
uint8_t oled_init(void)
{
    static const uint8_t init_seq[] = {
        0xAE,             /* display off */
        0xD5, 0x80,       /* clock divide */
        0xA8, 0x3F,       /* multiplex 1/64 */
        0xD3, 0x00,       /* display offset 0 */
        0x40,             /* start line 0 */
        0x8D, 0x14,       /* charge pump on */
        0x20, 0x02,       /* page addressing mode */
        0xA1,             /* segment remap */
        0xC8,             /* COM scan direction remapped */
        0xDA, 0x12,       /* COM pins */
        0x81, 0xCF,       /* contrast */
        0xD9, 0xF1,       /* pre-charge */
        0xDB, 0x40,       /* VCOMH */
        0xA4,             /* follow RAM content */
        0xA6,             /* normal (not inverted) */
        0xAF              /* display on */
    };

    for (uint8_t i = 0; i < sizeof init_seq; i++)
        if (oled_cmd(init_seq[i]))
            return 1;
    return 0;
}

void oled_clear(void)
{
    for (uint8_t page = 0; page < 8; page++) {
        set_cursor(page, 0);
        if (i2c_start((uint8_t)(OLED_ADDR << 1)))
            return;
        i2c_write(0x40);                                     /* 0x40 = data stream */
        for (uint8_t col = 0; col < 128; col++)
            i2c_write(0x00);
        i2c_stop();
    }
}

void oled_puts(uint8_t page, uint8_t col, const char *s, uint8_t scale)
{
    uint8_t pages = (scale == 2) ? 2 : 1;

    for (uint8_t p = 0; p < pages; p++) {
        set_cursor((uint8_t)(page + p), col);
        if (i2c_start((uint8_t)(OLED_ADDR << 1)))
            return;
        i2c_write(0x40);

        for (const char *c = s; *c; c++) {
            for (uint8_t i = 0; i < 5; i++) {
                uint8_t b = glyph_col(*c, i);
                if (scale == 2) {
                    uint8_t v = p ? spread_nibble(b >> 4) : spread_nibble(b & 0x0F);
                    i2c_write(v);
                    i2c_write(v);
                } else {
                    i2c_write(b);
                }
            }
            i2c_write(0x00);                                 /* 1-column gap */
            if (scale == 2)
                i2c_write(0x00);
        }
        i2c_stop();
    }
}
