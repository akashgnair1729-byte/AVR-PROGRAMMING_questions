#ifndef LCD_H
#define LCD_H

#include <stdint.h>

/* HD44780 16x2 LCD, 4-bit mode.
 *   RS -> PB0, E -> PB1, RW -> GND
 *   D4..D7 -> PD4..PD7
 */
void lcd_init(void);
void lcd_cmd(uint8_t cmd);
void lcd_data(uint8_t data);
void lcd_goto(uint8_t row, uint8_t col);
void lcd_puts(const char *s);

#endif
