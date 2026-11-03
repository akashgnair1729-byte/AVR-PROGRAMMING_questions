
#include <avr/io.h>
#include <util/delay.h>

#define PWM_PIN PD6
#define BTN_PIN PD2
#define BTN_DOWN() (!(PIND & (1 << BTN_PIN)))

static const uint8_t duty_ocr[4] = { 63, 127, 191, 255 };

static void pwm_init(void)
{
    DDRD |= (1 << PWM_PIN);
    TCCR0A = (1 << COM0A1) | (1 << WGM01) | (1 << WGM00);   
    TCCR0B = (1 << CS01) | (1 << CS00);                     
    OCR0A  = duty_ocr[0];                                   
}


static void pwm_set_level(uint8_t level)
{
    OCR0A = duty_ocr[level];
}

int main(void)
{
    uint8_t level = 0;

    pwm_init();

    DDRD &= ~(1 << BTN_PIN);
    PORTD |= (1 << BTN_PIN);

    for (;;) {
        if (BTN_DOWN()) {
            _delay_ms(20);
            if (BTN_DOWN()) {
                level = (level + 1) & 0x03;      
                pwm_set_level(level);
                while (BTN_DOWN())
                    ;
                _delay_ms(20);
            }
        }
    }
}
