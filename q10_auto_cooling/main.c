
#include <avr/io.h>
#include <util/delay.h>

#define BTN_PIN  PD2
#define MOTOR_PIN PD6

#define TEMP_MED_X10   300     
#define TEMP_HIGH_X10  400     
#define SPEED_LOW      40
#define SPEED_MED      70
#define SPEED_HIGH     100

#define SAMPLES 8

/* ---------- ADC ---------- */
static void adc_init(void)
{
    ADMUX  = (1 << REFS1) | (1 << REFS0);                      
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);   
    _delay_ms(10);
    ADCSRA |= (1 << ADSC);                                  
    while (ADCSRA & (1 << ADSC))
        ;
}

static uint16_t adc_read(void)
{
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC))
        ;
    return ADC;
}


static uint16_t read_temperature_x10(void)
{
    uint32_t sum = 0;
    for (uint8_t i = 0; i < SAMPLES; i++)
        sum += adc_read();
    uint32_t avg = sum / SAMPLES;
    return (uint16_t)(avg * 1100UL / 1024UL);
}


static uint8_t calc_speed_percent(uint16_t temp_x10)
{
    if (temp_x10 >= TEMP_HIGH_X10) return SPEED_HIGH;
    if (temp_x10 >= TEMP_MED_X10)  return SPEED_MED;
    return SPEED_LOW;
}


static void motor_init(void)
{
    DDRD |= (1 << MOTOR_PIN);
    TCCR0A = (1 << WGM01) | (1 << WGM00);        
    TCCR0B = (1 << CS01) | (1 << CS00);         
}

static void motor_set_percent(uint8_t percent)
{
    if (percent == 0) {
        TCCR0A &= ~(1 << COM0A1);                
        PORTD &= ~(1 << MOTOR_PIN);
    } else {
        OCR0A = (uint8_t)((uint16_t)percent * 256 / 100 - 1);
        TCCR0A |= (1 << COM0A1);
    }
}


static uint8_t button_pressed_event(void)
{
    static uint8_t stable = 1, counter = 0;
    uint8_t raw = (PIND & (1 << BTN_PIN)) ? 1 : 0;

    if (raw == stable) {
        counter = 0;
        return 0;
    }
    if (++counter >= 3) {                      
        counter = 0;
        stable = raw;
        if (!stable)
            return 1;
    }
    return 0;
}

int main(void)
{
    uint8_t system_on = 0;
    uint8_t tick = 0;

    DDRD &= ~(1 << BTN_PIN);
    PORTD |= (1 << BTN_PIN);

    adc_init();
    motor_init();
    motor_set_percent(0);

    for (;;) {
        if (button_pressed_event()) {
            system_on = !system_on;
            tick = 0;
            if (system_on)
                motor_set_percent(calc_speed_percent(read_temperature_x10()));
            else
                motor_set_percent(0);
        }

        if (system_on && ++tick >= 50) {         
            tick = 0;
            motor_set_percent(calc_speed_percent(read_temperature_x10()));
        }

        _delay_ms(10);
    }
}
