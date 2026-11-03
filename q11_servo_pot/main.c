
#include <avr/io.h>
#include <util/delay.h>

#define SERVO_MIN_US 544
#define SERVO_MAX_US 2400

static void timer1_servo_init(void)
{
    DDRB |= (1 << PB1);
    ICR1   = 20000;                                   
    OCR1A  = (SERVO_MIN_US + SERVO_MAX_US) / 2;       
    TCCR1A = (1 << COM1A1) | (1 << WGM11);            
    TCCR1B = (1 << WGM13) | (1 << CS11);             
}

static void adc_init(void)
{
    ADMUX  = (1 << REFS0);                            
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);   
}


static uint16_t adc_read_pot(void)
{
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC))
        ;
    return ADC;
}


static uint16_t adc_to_pulse_us(uint16_t adc)
{
    return (uint16_t)(SERVO_MIN_US +
                      ((uint32_t)adc * (SERVO_MAX_US - SERVO_MIN_US)) / 1023UL);
}

int main(void)
{
    adc_init();
    timer1_servo_init();

    for (;;) {
        OCR1A = adc_to_pulse_us(adc_read_pot());
        _delay_ms(20);                               
    }
}
