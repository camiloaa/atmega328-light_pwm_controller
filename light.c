/*
 * light.c
 *
 *  Created on: 13/6/2015
 *      Author: camilo
 */

#define F_CPU 8000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define USART_BAUDRATE 9600
#define BAUD_PRESCALE (((F_CPU/(USART_BAUDRATE*16UL)))-1)

#define stb(byte, bit) (byte |= _BV(bit))
#define clb(byte, bit) (byte &= ~_BV(bit))
#define tgb(byte, bit) (byte ^= _BV(bit))

#define MIN_ON 0
#define MIN_OFF 254
#define FULL_CYCLE 0xFF
#define HALF_CYCLE 0x80
#define OFF 0
#define ON 1

const unsigned char table_log1[256] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5,
		5, 5, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9,
		10, 10, 10, 10, 11, 11, 11, 11, 12, 12, 12, 13, 13, 13, 14, 14, 14, 14,
		15, 15, 16, 16, 16, 17, 17, 17, 18, 18, 19, 19, 20, 20, 20, 21, 21, 22,
		22, 23, 23, 24, 25, 25, 26, 26, 27, 27, 28, 29, 29, 30, 31, 31, 32, 33,
		34, 34, 35, 36, 37, 38, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
		50, 51, 52, 54, 55, 56, 57, 59, 60, 61, 63, 64, 65, 67, 68, 70, 72, 73,
		75, 76, 78, 80, 82, 83, 85, 87, 89, 91, 93, 95, 97, 99, 102, 104, 106,
		109, 111, 114, 116, 119, 121, 124, 127, 129, 132, 135, 138, 141, 144,
		148, 151, 154, 158, 161, 165, 168, 172, 176, 180, 184, 188, 192, 196,
		201, 205, 209, 214, 219, 224, 229, 234, 239, 244, 249, 255 };
const unsigned char table_log2[256] = { 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 254, 254, 254,
		254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254,
		254, 254, 254, 254, 254, 254, 254, 253, 253, 253, 253, 253, 253, 253,
		253, 253, 253, 253, 253, 253, 253, 253, 252, 252, 252, 252, 252, 252,
		252, 252, 252, 252, 252, 252, 251, 251, 251, 251, 251, 251, 251, 251,
		251, 250, 250, 250, 250, 250, 250, 250, 250, 249, 249, 249, 249, 249,
		249, 248, 248, 248, 248, 248, 248, 247, 247, 247, 247, 247, 246, 246,
		246, 246, 246, 245, 245, 245, 245, 244, 244, 244, 244, 243, 243, 243,
		242, 242, 242, 241, 241, 241, 241, 240, 240, 239, 239, 239, 238, 238,
		238, 237, 237, 236, 236, 235, 235, 235, 234, 234, 233, 233, 232, 232,
		231, 230, 230, 229, 229, 228, 228, 227, 226, 226, 225, 224, 224, 223,
		222, 221, 221, 220, 219, 218, 217, 217, 216, 215, 214, 213, 212, 211,
		210, 209, 208, 207, 206, 205, 204, 203, 201, 200, 199, 198, 196, 195,
		194, 192, 191, 190, 188, 187, 185, 183, 182, 180, 179, 177, 175, 173,
		172, 170, 168, 166, 164, 162, 160, 158, 156, 153, 151, 149, 146, 144,
		141, 139, 136, 134, 131, 128, 126, 123, 120, 117, 114, 111, 107, 104,
		101, 97, 94, 90, 87, 83, 79, 75, 71, 67, 63, 59, 54, 50, 46, 41, 36, 31,
		26, 21, 16, 11, 6, 0 };

static unsigned short long_warm_ref, long_cold_ref;
static unsigned char step_counter_warm, step_counter_cold;
static char step_warm, step_cold;
static unsigned char dc_warm, dc_cold;
static unsigned short long_warm, long_cold;
volatile unsigned char timer_interrupt_req = 0;

ISR(TIMER0_OVF_vect) {
	timer_interrupt_req = 0xFF;
}

/**
 * Process variations of duty-cycle
 * When a number of cycles (long_x) has been reached, increment or
 * decrement duty-cycle according to the requested step_x.
 */
void timer_interrupt() {
	/* Process gradients for warm light */
	if ((0 != step_warm) && (long_warm_ref < ++long_warm)) {
		long_warm = 0;
		if (0 < step_counter_warm) {
			step_counter_warm--;
			if (step_warm > 0 && FULL_CYCLE - step_warm < dc_warm ) {
				// Maximum duty-cycle reached
				dc_warm = FULL_CYCLE;
				step_warm = 0;
			} else if (step_warm < 0 && -step_warm > dc_warm) {
				// Minimum duty-cycle reached
				dc_warm = 0;
				step_warm = 0;
			} else {
				dc_warm += step_warm;
			}
		} else {
			// All duty-cycle steps executed
			step_warm = 0;
		}
	}

	/* Process gradients for cold light */
	if ((0 != step_cold) && (long_cold_ref < ++long_cold)) {
		long_cold = 0;
		if (0 < step_counter_cold) {
			step_counter_cold--;
			if (step_cold > 0 && FULL_CYCLE - step_cold < dc_cold ) {
				dc_cold = FULL_CYCLE;
				step_counter_cold = 0;
				step_cold = 0;
			} else if (step_cold < 0 && -step_cold > dc_cold) {
				dc_cold = 0;
				step_counter_cold = 0;
				step_cold = 0;
			} else {
				dc_cold += step_cold;
			}
		} else {
			step_cold = 0;
		}
	}
}

static void pwm_setup_60hz() {
	// Setup PWM
	// Freq = (16E6)/(1024*256) = 62 Hz

	// Pinmodes: Set OC0A and OC0B as outputs
	DDRD |=  _BV(PD5) | _BV(PD6);

	// Set Initial Timer value
	TCNT0=0x00;
	//set non inverted PWM on OC1A pin
	//and inverted on OC1B
	TCCR0A = 0xB3;	// Mode 2 (non-inverted) OC0A (0x80)
					// Mode 3 (inverted) OC0B     (0x30)
					// Mode 3 (Fast PWM) WGM      (0x03)
	//set compare values
	OCR0A=0x64;
	OCR0B=0x96;
	// Start PWM
	TCCR0B = 0x05;	// Prescaler 1/1024
}

static void pwm_setup_120hz() {
	// Setup PWM
	// Freq = (16E6)/(2*256*255) = 122 Hz

	// Pinmodes: Set OC0A and OC0B as outputs
	DDRD |=  _BV(PD5) | _BV(PD6);

	// Set Initial Timer value
	TCNT0=0x00;
	//set non inverted PWM on OC1A pin
	//and inverted on OC1B
	TCCR0A = 0xB1;	// Mode 2 (non-inverted) OC0A       (0x80)
					// Mode 3 (inverted) OC0B           (0x30)
					// Mode 1 (Phase-corrected PWM) WGM (0x01)
	//set compare values
	OCR0A=0x64;
	OCR0B=0x96;
	// Start PWM
	TCCR0B = 0x04;	// Prescaler 1/1024
}

int main() {
	unsigned int super_long_count = 0;

	/* HW-Initialization */
	PRR = 0; // Turn off all peripherals

	// Serial Comm:
	clb(PRR, PRUSART0);// Turn on USART
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0);
	UCSR0C |= (1 << UCSZ00) | (1 << UCSZ01);
	UBRR0H = (BAUD_PRESCALE >> 8);
	UBRR0L = BAUD_PRESCALE;

	// Configure PWM
	pwm_setup_60hz();

	/* Initialize variables */
	long_cold_ref = long_warm_ref = 0;
	step_counter_warm = step_counter_cold = 0;
	step_warm = step_cold = 0;
	dc_warm = HALF_CYCLE;
	dc_cold = 0;
	long_warm = long_cold = 0;

	// Main loop
	while (1) {
		// Check serial data
		// Check capacitive input
		// Full PWM cycle. Process gradients
		if (timer_interrupt_req)
			timer_interrupt();
	}

	return 0;
}
