#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"

#define LCD_RS_PIN PB0
#define LCD_E_PIN  PB1

static void pulse_enable(void)
{
    PORTB |= (1 << LCD_E_PIN);
    _delay_us(2);
    PORTB &= ~(1 << LCD_E_PIN);
    _delay_us(60);                       /* most commands need ~37 us */
}

/* Put the low nibble of 'nib' on D4..D7 (PD4..PD7), keep PD0..PD3 untouched. */
static void write_nibble(uint8_t nib)
{
    PORTD = (PORTD & 0x0F) | (uint8_t)((nib & 0x0F) << 4);
    pulse_enable();
}

static void write_byte(uint8_t value, uint8_t rs)
{
    if (rs)
        PORTB |= (1 << LCD_RS_PIN);
    else
        PORTB &= ~(1 << LCD_RS_PIN);

    write_nibble(value >> 4);
    write_nibble(value & 0x0F);
}

void lcd_cmd(uint8_t cmd)
{
    write_byte(cmd, 0);
    if (cmd <= 0x03)                     /* clear display / return home are slow */
        _delay_ms(2);
}

void lcd_data(uint8_t data)
{
    write_byte(data, 1);
}

void lcd_init(void)
{
    DDRB |= (1 << LCD_RS_PIN) | (1 << LCD_E_PIN);
    DDRD |= 0xF0;
    PORTB &= ~((1 << LCD_RS_PIN) | (1 << LCD_E_PIN));

    _delay_ms(50);                       /* power-up wait */

    /* Standard HD44780 "initialise by instruction" sequence for 4-bit mode */
    PORTB &= ~(1 << LCD_RS_PIN);
    write_nibble(0x03);
    _delay_ms(5);
    write_nibble(0x03);
    _delay_us(150);
    write_nibble(0x03);
    _delay_us(150);
    write_nibble(0x02);                  /* switch to 4-bit */
    _delay_us(150);

    lcd_cmd(0x28);                       /* 4-bit, 2 lines, 5x8 font */
    lcd_cmd(0x0C);                       /* display on, cursor off */
    lcd_cmd(0x06);                       /* entry mode: increment, no shift */
    lcd_cmd(0x01);                       /* clear */
}

void lcd_goto(uint8_t row, uint8_t col)
{
    lcd_cmd(0x80 | (uint8_t)((row ? 0x40 : 0x00) + col));
}

void lcd_puts(const char *s)
{
    while (*s)
        lcd_data((uint8_t)*s++);
}
