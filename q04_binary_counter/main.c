
#include <avr/io.h>
#include <util/delay.h>

#define BTN_PIN PD2
#define BTN_DOWN() (!(PIND & (1 << BTN_PIN)))


static void update_leds(uint8_t value)
{
    PORTB = (PORTB & 0xF0) | (value & 0x0F);
}

int main(void)
{
    uint8_t count = 0;

    DDRB |= 0x0F;
    update_leds(count);

    DDRD &= ~(1 << BTN_PIN);
    PORTD |= (1 << BTN_PIN);

    for (;;) {
        if (BTN_DOWN()) {
            _delay_ms(20);                     
            if (BTN_DOWN()) {
                count = (count >= 15) ? 0 : (uint8_t)(count + 1);
                update_leds(count);

                while (BTN_DOWN())              
                    ;
                _delay_ms(20);
            }
        }
    }
}
