
#include <avr/io.h>
#include <util/delay.h>

#define TRAIN_DDR  DDRB
#define TRAIN_PORT PORTB

#define STEP_MS 150

static void delay_ms(uint16_t ms)
{
    while (ms--)
        _delay_ms(1);
}

int main(void)
{
    uint8_t pattern = 0x01;          

    TRAIN_DDR = 0xFF;                

    for (;;) {
        TRAIN_PORT = pattern;
        delay_ms(STEP_MS);

        pattern <<= 1;               
        if (pattern == 0)            
            pattern = 0x01;
    }
}
