
#include <avr/io.h>

#define GREEN_PIN  PB0
#define ORANGE_PIN PB1
#define RED_PIN    PB2
#define ALL_LEDS   ((1 << GREEN_PIN) | (1 << ORANGE_PIN) | (1 << RED_PIN))

static void timer1_init_1s(void)
{
    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);   
    OCR1A  = 15624;
    TIFR1  = (1 << OCF1A);                               
}


static void wait_seconds(uint8_t seconds)
{
    while (seconds--) {
        while (!(TIFR1 & (1 << OCF1A)))
            ;
        TIFR1 = (1 << OCF1A);                            /
    }
}

static void set_light(uint8_t pin)
{
    PORTB = (PORTB & ~ALL_LEDS) | (1 << pin);
}

int main(void)
{
    DDRB |= ALL_LEDS;
    timer1_init_1s();

    for (;;) {
        set_light(GREEN_PIN);
        wait_seconds(10);
        set_light(ORANGE_PIN);
        wait_seconds(3);
        set_light(RED_PIN);
        wait_seconds(10);
    }
}
