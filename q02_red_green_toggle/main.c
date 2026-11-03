
#include <avr/io.h>
#include <util/delay.h>

#define RED_PIN   PB0
#define GREEN_PIN PB1
#define BTN_PIN   PD2

#define BTN_DOWN() (!(PIND & (1 << BTN_PIN)))

int main(void)
{
    DDRB |= (1 << RED_PIN) | (1 << GREEN_PIN);
    PORTB |= (1 << RED_PIN);                 
    PORTB &= ~(1 << GREEN_PIN);

    DDRD &= ~(1 << BTN_PIN);
    PORTD |= (1 << BTN_PIN);                 

    for (;;) {
        if (BTN_DOWN()) {
            _delay_ms(20);                  
            if (BTN_DOWN()) {
                
                PINB = (1 << RED_PIN) | (1 << GREEN_PIN);

                while (BTN_DOWN())           \
                    ;
                _delay_ms(20);
            }
        }
    }
}
