
#include <avr/io.h>
#include <util/delay.h>

#define LED_PIN PD5


static void delay_ms(uint16_t ms)
{
    while (ms--)
        _delay_ms(1);
}

int main(void)
{
    DDRD |= (1 << LED_PIN);        

    for (;;) {
        PORTD |= (1 << LED_PIN);    
        delay_ms(3000);
        PORTD &= ~(1 << LED_PIN);    
        delay_ms(5000);
    }
}
