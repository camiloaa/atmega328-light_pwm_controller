all:
	avr-gcc -O -g -o light.elf -mmcu=avr4 -D__AVR_ATmega328P__ light.c
