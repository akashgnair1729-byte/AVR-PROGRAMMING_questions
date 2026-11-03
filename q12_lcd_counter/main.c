
#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"

#define BTN_PIN     PD2
#define BTN_DOWN()  (!(PIND & (1 << BTN_PIN)))

#define DEBOUNCE_MS  20
#define LONG_PRESS_MS 2000
#define COUNT_MAX    999999999UL

typedef enum { PRESS_NONE, PRESS_SHORT, PRESS_LONG } press_t;


static press_t button_check(void)
{
    uint16_t held_ms = 0;

    if (!BTN_DOWN())
        return PRESS_NONE;

    _delay_ms(DEBOUNCE_MS);
    if (!BTN_DOWN())
        return PRESS_NONE;                   

    while (BTN_DOWN()) {
        _delay_ms(1);
        if (++held_ms >= LONG_PRESS_MS)
            return PRESS_LONG;
    }
    _delay_ms(DEBOUNCE_MS);                 
    return PRESS_SHORT;
}

static void button_wait_release(void)
{
    while (BTN_DOWN())
        ;
    _delay_ms(DEBOUNCE_MS);
}


static void lcd_show_count(uint32_t value)
{
    char buf[10];

    for (int8_t i = 8; i >= 0; i--) {
        buf[i] = (char)('0' + (value % 10));
        value /= 10;
    }
    buf[9] = '\0';

    lcd_goto(1, 0);
    lcd_puts(buf);
}

int main(void)
{
    uint32_t count = 0;

    DDRD &= ~(1 << BTN_PIN);
    PORTD |= (1 << BTN_PIN);

    lcd_init();
    lcd_goto(0, 0);
    lcd_puts("COUNT:");
    lcd_show_count(count);

    for (;;) {
        switch (button_check()) {
        case PRESS_SHORT:
            count = (count >= COUNT_MAX) ? 0 : count + 1;
            lcd_show_count(count);
            break;
        case PRESS_LONG:
            count = 0;
            lcd_show_count(count);
            button_wait_release();           
            break;
        default:
            break;
        }
    }
}
