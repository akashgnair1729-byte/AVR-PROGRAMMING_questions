
#include <avr/io.h>
#include <avr/interrupt.h>

#define GREEN_PIN  PB0
#define ORANGE_PIN PB1
#define RED_PIN    PB2
#define ALL_LEDS   ((1 << GREEN_PIN) | (1 << ORANGE_PIN) | (1 << RED_PIN))

#define GREEN_S            10
#define ORANGE_S           3
#define RED_S              10
#define EMERGENCY_GREEN_S  15

typedef enum { ST_GREEN, ST_ORANGE, ST_RED } state_t;

static volatile uint8_t emergency_flag = 0;

ISR(INT0_vect)
{
    emergency_flag = 1;              

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
        TIFR1 = (1 << OCF1A);
    }
}

static void set_light(uint8_t pin)
{
    PORTB = (PORTB & ~ALL_LEDS) | (1 << pin);
}

static void int0_init(void)
{
    DDRD  &= ~(1 << PD2);
    PORTD |= (1 << PD2);            
    EICRA  = (1 << ISC01);           
    EIMSK  = (1 << INT0);
}

int main(void)
{
    state_t state = ST_GREEN;

    DDRB |= ALL_LEDS;
    int0_init();
    timer1_init_1s();
    sei();

    for (;;) {
      
        switch (state) {
        case ST_GREEN:  set_light(GREEN_PIN);  wait_seconds(GREEN_S);  break;
        case ST_ORANGE: set_light(ORANGE_PIN); wait_seconds(ORANGE_S); break;
        case ST_RED:    set_light(RED_PIN);    wait_seconds(RED_S);    break;
        }

       
        if (emergency_flag) {
            set_light(GREEN_PIN);
            wait_seconds(EMERGENCY_GREEN_S);
            emergency_flag = 0;     
            state = ST_ORANGE;      
        } else {
            state = (state == ST_RED) ? ST_GREEN : (state_t)(state + 1);
        }
    }
}
