
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include "i2c.h"
#include "bmp280.h"
#include "oled.h"

static void delay_ms(uint16_t ms)
{
    while (ms--)
        _delay_ms(1);
}


static void format_temperature(char *buf, size_t size, int32_t t_c100)
{
    const char *sign = (t_c100 < 0) ? "-" : "";
    uint32_t a = (uint32_t)((t_c100 < 0) ? -t_c100 : t_c100);
    snprintf(buf, size, "%s%lu.%02lu *C", sign, (unsigned long)(a / 100), (unsigned long)(a % 100));
}

static void format_pressure(char *buf, size_t size, uint32_t p_pa)
{
    /* Pa -> hPa with one decimal */
    snprintf(buf, size, "%lu.%lu HPA", (unsigned long)(p_pa / 100), (unsigned long)((p_pa % 100) / 10));
}

int main(void)
{
    char line[20];
    int32_t t_c100;
    uint32_t p_pa;

    i2c_init(100000UL);

    if (oled_init()) {
        for (;;)
            ;                                        
    }
    oled_clear();
    oled_puts(0, 0, "BMP280 SENSOR", 1);

    if (bmp280_init()) {
        oled_puts(3, 0, "SENSOR NOT FOUND", 1);
        for (;;)
            ;
    }

    for (;;) {
        if (bmp280_read(&t_c100, &p_pa) == 0) {
            oled_clear();
            oled_puts(0, 0, "TEMPERATURE", 1);
            format_temperature(line, sizeof line, t_c100);
            oled_puts(1, 0, line, 2);               

            oled_puts(4, 0, "PRESSURE", 1);
            format_pressure(line, sizeof line, p_pa);
            oled_puts(5, 0, line, 2);                
        } else {
            oled_clear();
            oled_puts(3, 0, "READ ERROR", 1);
        }
        delay_ms(2000);
    }
}
