# Build every question with:  make            (all hex files land in build/)
# Build one question:         make build/q01_led_timing.hex
# Flash one question:         make flash Q=q01   (override PORT / PROGRAMMER if needed)

MCU        ?= atmega328p
F_CPU      ?= 16000000UL
PROGRAMMER ?= arduino
PORT       ?= /dev/ttyACM0
BAUD       ?= 115200

CC      = avr-gcc
OBJCOPY = avr-objcopy
SIZE    = avr-size
CFLAGS  = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -std=gnu99 -Os -Wall -Wextra \
          -ffunction-sections -fdata-sections
LDFLAGS = -mmcu=$(MCU) -Wl,--gc-sections

DIRS  := $(sort $(patsubst %/,%,$(wildcard q[0-9][0-9]_*/)))
HEXES := $(addprefix build/,$(addsuffix .hex,$(DIRS)))

.PHONY: all clean list flash

all: $(HEXES)

define BUILD_RULE
build/$(1).elf: $(wildcard $(1)/*.c) $(wildcard $(1)/*.h)
	@mkdir -p build
	$(CC) $(CFLAGS) $(LDFLAGS) -o $$@ $(wildcard $(1)/*.c)
endef
$(foreach d,$(DIRS),$(eval $(call BUILD_RULE,$(d))))

build/%.hex: build/%.elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@
	@$(SIZE) -C --mcu=$(MCU) $<

TARGET = $(firstword $(filter $(Q)_%,$(DIRS)))

flash: build/$(TARGET).hex
	avrdude -p $(MCU) -c $(PROGRAMMER) -P $(PORT) -b $(BAUD) -U flash:w:$<:i

list:
	@printf '%s\n' $(DIRS)

clean:
	rm -rf build
