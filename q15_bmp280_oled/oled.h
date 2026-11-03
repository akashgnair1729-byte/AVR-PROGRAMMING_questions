#ifndef OLED_H
#define OLED_H

#include <stdint.h>

/* SSD1306 128x64 OLED over I2C (address 0x3C), page addressing: 8 pages of 8 rows.
 * The built-in font is a small 5x7 set: space . - : % * (degree) 0-9 and the capital
 * letters A B C D E F H M N O P R S T U. Anything else is drawn as a space;
 * lower-case is converted to upper-case first. */
uint8_t oled_init(void);
void    oled_clear(void);

/* Draw text starting at (page, col_pixel). scale = 1 -> 1 page tall, 6 px per char.
 * scale = 2 -> 2 pages tall (uses page and page+1), 12 px per char. */
void    oled_puts(uint8_t page, uint8_t col, const char *s, uint8_t scale);

#endif
