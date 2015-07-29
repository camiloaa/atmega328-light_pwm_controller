all:
	avr-gcc -O -g -o light.elf -mmcu=atmega328p -D__AVR_ATmega328P__ light.c
