# AVR Programming (ATmega328P) Assignments

Solutions for the 15 ATmega328P assignment questions, written in plain C for `avr-gcc` / `avr-libc`.
One folder per question, each one builds into its own `.hex`.

## Assumptions

- Target: **ATmega328P at 16 MHz** (Arduino Uno / Nano or a bare chip with a 16 MHz crystal). If your clock is different, build with `make F_CPU=8000000UL`.
- Pin names are AVR port names (`PD5`, `PB1`, ...). On an Uno: PD0-PD7 = D0-D7, PB0-PB5 = D8-D13, PC0-PC5 = A0-A5.
- Buttons are wired between the pin and GND and use the internal pull-up (pressed = LOW).
- LEDs always need a series resistor (220-470 ohm). Motors need a transistor/driver and a flyback diode; servos need their own 5 V supply with a common GND.

## Questions and wiring

| Folder | Topic | Wiring |
|---|---|---|
| `q01_led_timing` | LED 3 s on / 5 s off | LED on PD5 |
| `q02_red_green_toggle` | Red/Green toggle by button | Red PB0, Green PB1, button PD2 |
| `q03_led_train` | 8-LED "train" | LEDs on PB0-PB7 (see note below) |
| `q04_binary_counter` | 4-bit counter on a button | LEDs PB0-PB3, button PD2 |
| `q05_traffic_light_timer` | Traffic light, Timer1, no `_delay` | Green PB0, Orange PB1, Red PB2 |
| `q06_traffic_light_int0` | Traffic light + INT0 emergency flag | same LEDs, button on PD2 (INT0) |
| `q07_dual_timer_isr` | Timer1 (2 s) + Timer2 (500 ms) ISRs | Status LED PB1, Heartbeat LED PB2 |
| `q08_pwm_levels` | Timer0 Fast PWM, 4 duty levels | LED/driver PD6 (OC0A), button PD2 |
| `q09_pwm_debounce` | Same, with proper debounce | same as Q8 |
| `q10_auto_cooling` | LM35 + motor + ON/OFF button | LM35 PC0 (ADC0), motor driver PD6, button PD2 |
| `q11_servo_pot` | Servo from potentiometer (Timer1 phase-correct PWM) | Pot PC0 (ADC0), servo PB1 (OC1A) |
| `q12_lcd_counter` | 9-digit LCD counter, short/long press | LCD RS PB0, E PB1, D4-D7 PD4-PD7, RW GND; button PD2 |
| `q13_uart_led` | LED control over UART (ON / OFF / 0-100) | LED PD6 (OC0A); USB serial at 9600 baud |
| `q14_pot_motor_pwm` | Pot -> DC motor speed | Pot PC0, motor driver PD6 |
| `q15_bmp280_oled` | BMP280 -> SSD1306 OLED over I2C | SDA PC4, SCL PC5; BMP280 (0x76/0x77) and OLED (0x3C) on the same bus |

### Notes on the assignment sheet

- **Q3:** PB6/PB7 are the crystal pins. On an Uno they can't be used as GPIO, so either run a bare chip from the internal oscillator or switch the three `TRAIN_*` macros in `q03_led_train/main.c` to PORTD.
- **Q8 / Q9:** the sheet says "100% OFF", which is read as a typo for 100%. Q9 is Q8 plus a stricter debounce.
- **Q14:** it sits under the "SPI & I2C" heading, but the described task only needs ADC and PWM, so no SPI/I2C is used.
- **Q15:** the sheet asks for temperature, the objective also mentions pressure, so both are displayed. The OLED font only contains the characters the program prints; add glyphs in `oled.c` if you want more text.

## Build

Install the toolchain:

- Windows: [Microchip AVR-GCC toolchain](https://www.microchip.com/en-us/tools-resources/develop/microchip-studio/gcc-compilers) or WinAVR, plus `make` and `avrdude`
- Ubuntu/Debian: `sudo apt install gcc-avr avr-libc avrdude make`
- macOS: `brew tap osx-cross/avr && brew install avr-gcc avrdude`

```bash
make                       # build every question into build/*.hex
make build/q01_led_timing.hex   # build just one
make list                  # list all questions
make clean
```

## Flash

```bash
make flash Q=q01 PORT=/dev/ttyACM0          # Arduino Uno bootloader (Windows: PORT=COM3)
make flash Q=q05 PROGRAMMER=usbasp PORT=usb # USBasp programmer (drop BAUD/PORT as needed)
```

## Simulating

Every program can also be run without hardware in Wokwi or Proteus: build the `.hex` and load it into an ATmega328P (or Arduino Uno) part, then place the parts as listed in the wiring table.
