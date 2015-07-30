/*
 * uart.h
 *
 *  Created on: 30/07/2015
 *      Author: Camilo Arboleda
 *      Based on https://gist.github.com/gshrikant/8549474
 */

#ifndef INCLUDE_UART_H_
#define INCLUDE_UART_H_

#include "light.h"
#include <stdbool.h>

#define USART_BAUDRATE 9600
#define BAUD_PRESCALE (((F_CPU/(USART_BAUDRATE*16UL)))-1)

/* Useful macros */
#define TX_START()		UCSR0B |= _BV(TXEN0)	// Enable TX
#define TX_STOP()		UCSR0B &= ~_BV(TXEN0)	// Disable TX
#define RX_START()		UCSR0B |= _BV(RXEN0)	// Enable RX
#define RX_STOP()		UCSR0B &= ~_BV(RXEN0)	// Disable RX
#define COMM_START()	do{	TX_START(); RX_START();} while(0)	// Enable communications
#define TX_READY()		(UCSR0A & _BV(UDRE0))
#define TX_BUSY()		!(TX_READY())

/* Interrupt macros; Remember to set the GIE bit in SREG before using (see datasheet) */
#define RX_INTEN()		UCSR0B |= _BV(RXCIE0)	// Enable interrupt on RX complete
#define RX_INTDIS()		UCSR0B &= ~_BV(RXCIE0)	// Disable RX interrupt
#define TX_INTEN()		UCSR0B |= _BV(TXCIE0)	// Enable interrupt on TX complete
#define TX_INTDIS()		UCSR0B &= ~_BV(TXCIE0)	// Disable TX interrupt

/* Function prototypes */
void uart_setup();
void put_byte(unsigned char data);

/* Shared variables */
extern volatile bool serial_cmd_ready;
extern volatile char serial_cmd_answer[4];

#endif /* INCLUDE_UART_H_ */
