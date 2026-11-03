
#include <avr/io.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define BAUD     9600UL
#define UBRR_VAL ((F_CPU / 16UL / BAUD) - 1)

#define LED_PIN  PD6
#define CMD_MAX  16


static void uart_init(void)
{
    UBRR0H = (uint8_t)(UBRR_VAL >> 8);
    UBRR0L = (uint8_t)UBRR_VAL;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);          
}

static void uart_putc(char c)
{
    while (!(UCSR0A & (1 << UDRE0)))
        ;
    UDR0 = (uint8_t)c;
}

static void uart_puts(const char *s)
{
    while (*s)
        uart_putc(*s++);
}

static char uart_getc(void)
{
    while (!(UCSR0A & (1 << RXC0)))
        ;
    return (char)UDR0;
}


static uint8_t uart_getline(char *buf, uint8_t size)
{
    uint8_t n = 0;
    for (;;) {
        char c = uart_getc();
        if (c == '\r' || c == '\n') {
            buf[n] = '\0';
            return n;
        }
        if (n < size - 1)
            buf[n++] = (char)toupper((unsigned char)c);
    }
}


static void pwm_init(void)
{
    DDRD |= (1 << LED_PIN);
    TCCR0A = (1 << WGM01) | (1 << WGM00);           
    TCCR0B = (1 << CS01);                            
}


static void pwm_set_percent(uint8_t percent)
{
    if (percent == 0) {
        TCCR0A &= ~(1 << COM0A1);                    
        PORTD &= ~(1 << LED_PIN);
    } else {
        OCR0A = (uint8_t)((uint16_t)percent * 256 / 100 - 1);
        TCCR0A |= (1 << COM0A1);
    }
}


static uint8_t is_number(const char *s)
{
    if (*s == '\0')
        return 0;
    while (*s) {
        if (*s < '0' || *s > '9')
            return 0;
        s++;
    }
    return 1;
}

static void handle_command(const char *cmd)
{
    char num[8];

    if (strcmp(cmd, "ON") == 0) {
        pwm_set_percent(100);
        uart_puts("LED is now ON\r\n");
    } else if (strcmp(cmd, "OFF") == 0) {
        pwm_set_percent(0);
        uart_puts("LED is now OFF\r\n");
    } else if (is_number(cmd) && strlen(cmd) <= 3 && atoi(cmd) <= 100) {
        uint8_t value = (uint8_t)atoi(cmd);
        pwm_set_percent(value);
        utoa(value, num, 10);
        uart_puts("Brightness set to ");
        uart_puts(num);
        uart_puts("%\r\n");
    } else {
        uart_puts("Unknown command. Use ON, OFF or 0-100\r\n");
    }
}

int main(void)
{
    char line[CMD_MAX];

    uart_init();
    pwm_init();
    pwm_set_percent(0);
    uart_puts("Ready. Commands: ON, OFF, 0-100\r\n");

    for (;;) {
        if (uart_getline(line, sizeof line) > 0)
            handle_command(line);
    }
}
