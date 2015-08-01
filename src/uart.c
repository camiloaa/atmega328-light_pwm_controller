/*
 * uart.c
 *
 *  Created on: 30/07/2015
 *      Author: Camilo Arboleda
 */

#include "uart.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdint.h>

enum cmd_steps_e {
	WCMD,
	REGHN,
	REGLN,
	VALHN,
	VALLN,
};

enum cmd_e {
	NO_CMD,
	WRITE_CMD,
	READ_CMD,
	ERR_CMD,
};

// Global variables
volatile bool serial_cmd_ready = false;
volatile char serial_cmd_answer[4];

// Local variables
static volatile unsigned char step = WCMD;
static volatile unsigned char cmd = NO_CMD;
static volatile unsigned char reg_num = 0;
static volatile unsigned char reg_val = 0;
static volatile unsigned char *registers;

// Process incoming data
ISR(USART_RX_vect) {
	char inchar;
	inchar = UDR0;

	PORTB ^= _BV(PB5);

	/* Check if it's a command
	 * 'W' writes to a register
	 * 'R' reads from a register
	 * All values are in HEX     */
	if (inchar == 'w' || inchar == 'W') {
		cmd = WRITE_CMD;
		step = REGHN; // Waiting for register high nibble
		return;
	}
	if (inchar == 'r' || inchar == 'R') {
		cmd = READ_CMD;
		step = REGHN; // Waiting for register high nibble
		return;
	}

	/* Process incoming char
	 * It must be a valid HEX character */
	if (inchar < '0') { // Invalid input
		goto no_cmd_label;
	}
	if (inchar >= 'a')
		inchar -= 0x27; // Magic number 'a'->0x3A ('0'+0x0A)
	else if (inchar >= 'A')
		inchar -= 0x07; // Same magic
	inchar -= '0';
	if (inchar > 0x0F) { // Invalid input
		goto no_cmd_label;
	}

	/* Store the incoming char in the proper variable */
	switch (step) {
		case REGHN:
			reg_num = inchar << 4;
			step = REGLN;
		break;
		case REGLN:
			reg_num |= inchar;
			step = (cmd==WRITE_CMD) ? VALHN:WCMD;
			if (reg_num >= MAX_REG) {
				reg_num = 0;
				cmd = ERR_CMD;
			}
		break;
		case VALHN:
			reg_val = inchar << 4;
			step = VALLN;
		break;
		case VALLN:
			reg_val |= inchar;
			step = WCMD;
		break;
		default: // Error, no command
			goto no_cmd_label;
	}
	if (step == WCMD) {
		if (cmd == WRITE_CMD) {
			registers[reg_num] = reg_val; // Set the register value
			serial_cmd_answer[0] = 'W';
		} else if (cmd == READ_CMD) {
			serial_cmd_answer[0] = 'R';
		} else {
			serial_cmd_answer[0] = 'E';
		}
		// Return value as answer
		serial_cmd_answer[1] = 0x30 + (registers[reg_num] >> 4);
		if (serial_cmd_answer[1] > '9')
			serial_cmd_answer[1] += 0x07;
		serial_cmd_answer[2] = 0x30 + (registers[reg_num] & 0x0F);
		if (serial_cmd_answer[2] > '9')
			serial_cmd_answer[2] += 0x07;
		serial_cmd_answer[3] = '\n';
		// Command completed
		serial_cmd_ready = true;
	}
	return;
no_cmd_label:
	step = WCMD;
	cmd = NO_CMD;
	return;
}

/*! \brief Transmits a byte
 * 	Use this function if the TX interrupt is not enabled.
 * 	Blocks the serial port while TX completes
 */
void put_byte(unsigned char data)
{
	// Stay here until data buffer is empty
	while (TX_BUSY());
	UDR0 = (unsigned char) data;
}

void uart_setup() {
	// Point to light structures.
	// No security here, you can do whatever you want.
	registers = (unsigned char *)(all_lights);
	// Serial Comm:
	clb(PRR, PRUSART0);// Turn on USART
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0); // Enable USART
	UCSR0C =  (1 << UCSZ00) | (1 << UCSZ01); // 8-N-1
	UBRR0L = BAUD_PRESCALE;
	UBRR0H = (BAUD_PRESCALE >> 8);
	RX_INTEN();
	TX_INTDIS();
}
