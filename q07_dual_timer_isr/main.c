
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define STATUS_LED    PB1
#define HEARTBEAT_LED PB2

ISR(TIMER1_COMPA_vect)
{
    PINB = (1 << STATUS_LED);                
}

ISR(TIMER2_COMPA_vect)
{
    static uint16_t ms = 0;
    if (++ms >= 500) {
        ms = 0;
        PINB = (1 << HEARTBEAT_LED);
    }
}

static void timer1_init(void)
{
    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);   
    OCR1A  = 31249;
    TIMSK1 = (1 << OCIE1A);
}

static void timer2_init(void)
{
    TCCR2A = (1 << WGM21);                               
    TCCR2B = (1 << CS22);                                
    OCR2A  = 249;
    TIMSK2 = (1 << OCIE2A);
}

int main(void)
{
    DDRB |= (1 << STATUS_LED) | (1 << HEARTBEAT_LED);

    timer1_init();
    timer2_init();
    sei();

    set_sleep_mode(SLEEP_MODE_IDLE);                    
    for (;;)
        sleep_mode();
}
