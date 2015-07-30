/*
 * timer.c
 *
 *  Created on: 30/07/2015
 *      Author: Camilo Arboleda
 */

#include "timer.h"
#include <avr/interrupt.h>

ISR(TIMER0_OVF_vect) {
	timer_interrupt_req = 1;
}

volatile unsigned char timer_interrupt_req = 0;


#ifdef PWM60Hz
// Freq = (16E6)/(1024*256) = 62 Hz
// Mode 2 (non-inverted) OC0A (0x80)
// Mode 3 (inverted) OC0B     (0x30)
// Mode 3 (Fast PWM) WGM      (0x03)
// Prescaler 1/1024
#define REGA 0xB3
#define REGB 0x05
#elif defined(PWM120Hz)
// Freq = (16E6)/(2*256*255) = 122 Hz
// Mode 2 (non-inverted) OC0A       (0x80)
// Mode 3 (inverted) OC0B           (0x30)
// Mode 1 (Phase-corrected PWM) WGM (0x01)
// Prescaler 1/256
#define REGA 0xB1
#define REGB 0x04
#else
#error "Must define a PWM frequency"
#endif

void pwm_setup(){
	// Setup PWM
	clb(PRR, PRTIM0);// Turn on TIMER0

	// Pinmodes: Set OC0A and OC0B as outputs
	DDRD |=  _BV(PD5) | _BV(PD6);

	// Set Initial Timer value
	TCNT0=0;
	//set inital compare values
	OCR0A=0;
	OCR0B=0xFF;
	//set non inverted PWM on OC1A pin
	//and inverted on OC1B
	TCCR0A = REGA;
	// Start PWM
	TCCR0B = REGB;
	TIFR0  |= _BV(TOV0); // Clean interrupt request
	TIMSK0 |= _BV(TOIE0); // Enable interrupt request
}

