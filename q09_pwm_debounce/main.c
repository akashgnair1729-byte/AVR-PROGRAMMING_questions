
#include <avr/io.h>
#include <util/delay.h>

#define PWM_PIN     PD6
#define BTN_PIN     PD2
#define DEBOUNCE_MS 20

static const uint8_t duty_percent[4] = { 25, 50, 75, 100 };

static void pwm_init(void)
{
    DDRD |= (1 << PWM_PIN);
    TCCR0A = (1 << COM0A1) | (1 << WGM01) | (1 << WGM00);
    TCCR0B = (1 << CS01) | (1 << CS00);
}

static void pwm_set_percent(uint8_t percent)
{
    OCR0A = (uint8_t)((uint16_t)percent * 256 / 100 - 1);
}


static uint8_t button_pressed_event(void)
{
    static uint8_t stable = 1;              
    static uint8_t counter = 0;
    uint8_t raw = (PIND & (1 << BTN_PIN)) ? 1 : 0;

    if (raw == stable) {
        counter = 0;
        return 0;
    }
    if (++counter >= DEBOUNCE_MS) {
        counter = 0;
        stable = raw;
        if (stable == 0)
            return 1;                        
    }
    return 0;
}

int main(void)
{
    uint8_t level = 0;

    pwm_init();
    pwm_set_percent(duty_percent[level]);

    DDRD &= ~(1 << BTN_PIN);
    PORTD |= (1 << BTN_PIN);

    for (;;) {
        if (button_pressed_event()) {
            level = (level + 1) & 0x03;     
            pwm_set_percent(duty_percent[level]);
        }
        _delay_ms(1);
    }
}
