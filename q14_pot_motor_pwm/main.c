
#include <avr/io.h>
#include <util/delay.h>

#define MOTOR_PIN PD6

static void pwm_init(void)
{
    DDRD |= (1 << MOTOR_PIN);
    TCCR0A = (1 << WGM01) | (1 << WGM00);            
    TCCR0B = (1 << CS01) | (1 << CS00);             
}

static void adc_init(void)
{
    ADMUX  = (1 << REFS0);                           
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);   
}


static uint8_t read_pot_8bit(void)
{
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC))
        ;
    return (uint8_t)(ADC >> 2);                      
}

static void motor_set_speed(uint8_t ocr)
{
    if (ocr == 0) {
        TCCR0A &= ~(1 << COM0A1);                    
        PORTD &= ~(1 << MOTOR_PIN);
    } else {
        OCR0A = ocr;
        TCCR0A |= (1 << COM0A1);                    
    }
}

int main(void)
{
    adc_init();
    pwm_init();

    for (;;) {
        motor_set_speed(read_pot_8bit());
        _delay_ms(10);
    }
}
