/*
 * light.c
 *
 *  Created on: 13/6/2015
 *      Author: camilo
 */

#include "light.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <stdbool.h>
#include "timer.h"
#include "uart.h"

#define LIGHT_ON

#ifdef LIGHT_ON
#define INIT_STEP   1
#define INIT_COUNT 30
#define INIT_REF   10
#else
#define INIT_STEP  0
#define INIT_COUNT 0
#define INIT_REF   0
#endif

ISR(__vector_default) {
}

extern const unsigned char dc_table_log_inv[] PROGMEM;
extern const unsigned char dc_table_log_norm[] PROGMEM;

light_t all_lights[N_LIGHT];

/**
 * Process variations of duty-cycle
 * When a number of cycles (long_count) has been reached, increment or
 * decrement duty-cycle according to the requested step_x.
 */
bool process_gradient(light_t *light) {
	/* Process gradients for warm light */
	if ((0 != light->step) && (light->long_ref < (++light->long_count))) {
		light->long_count = 0;
		if (0 < light->step_count) {
			--light->step_count;
			if ((light->step > 0) && (FULL_CYCLE - light->step < light->dc)) {
				// Maximum duty-cycle reached
				light->dc = FULL_CYCLE;
				light->step = 0;
			} else if ((light->step < 0) && (-light->step > light->dc)) {
				// Minimum duty-cycle reached
				light->dc = 0;
				light->step = 0;
			} else {
				light->dc += light->step;
			}
		} else {
			// All duty-cycle steps executed
			light->step = 0;
			light->long_ref = 0;
		}
		return true;
	}
	return false;
}

int main() {
	unsigned char tx_pointer = 0;
	// HW-Initialization
	cli();
	PRR = 0xFF; // Turn off all peripherals
	wdt_disable();
	// No watchdog. FIXME

	// Initialize variables
	// Turn on the lights when processor resets defined by LIGHT_ON macro.
	all_lights[WARM_LIGHT].long_ref = INIT_REF;
	all_lights[WARM_LIGHT].step_count = INIT_COUNT;
	all_lights[WARM_LIGHT].step = 2*INIT_STEP;
	all_lights[WARM_LIGHT].dc = 0;
	all_lights[WARM_LIGHT].long_count = 0;
	all_lights[COLD_LIGHT].long_ref = INIT_REF;
	all_lights[COLD_LIGHT].step_count = INIT_COUNT;
	all_lights[COLD_LIGHT].step = 1*INIT_STEP;
	all_lights[COLD_LIGHT].dc = 0;
	all_lights[COLD_LIGHT].long_count = 0;
	all_lights[WARM_LIGHT].dc_out = pgm_read_byte(
			&(dc_table_log_norm[all_lights[WARM_LIGHT].dc]));
	all_lights[COLD_LIGHT].dc_out = pgm_read_byte(
			&(dc_table_log_inv[all_lights[COLD_LIGHT].dc]));

	// Serial Comm:
	uart_setup();

	// Configure PWM
	pwm_setup();

	// Configure LED
	DDRB |= _BV(PB5);
	PORTB = 0;

	// Activate the lamps
	OCR0A = all_lights[WARM_LIGHT].dc_out;
	OCR0B = all_lights[COLD_LIGHT].dc_out;

	sei();
	// Main loop
	while (1) {
		// Check serial data
		if (serial_cmd_ready && TX_READY()) {
			put_byte(serial_cmd_answer[tx_pointer]);
			++tx_pointer;
			tx_pointer %= 4;
			if (0 == tx_pointer) // Answer transmited
				serial_cmd_ready = false;
		}
		// Check capacitive input

		// Full PWM cycle. Process gradients
		if (timer_interrupt_req) {
			timer_interrupt_req = false;
			// First calculate new duty cycle
			// Then use linear to log transformation:
			//     50% perceived brightness corresponds to 25% power output
			if (process_gradient(all_lights + WARM_LIGHT)) {
				all_lights[WARM_LIGHT].dc_out = pgm_read_byte(
						&(dc_table_log_norm[all_lights[WARM_LIGHT].dc]));
			}
			if (process_gradient(all_lights + COLD_LIGHT)) {
				all_lights[COLD_LIGHT].dc_out = pgm_read_byte(
						&(dc_table_log_inv[all_lights[COLD_LIGHT].dc]));
			}
			// And adjust the actual duty cycle in TIMER0
			if (OCR0A != all_lights[WARM_LIGHT].dc_out)
				OCR0A = all_lights[WARM_LIGHT].dc_out;
			if (OCR0B != all_lights[COLD_LIGHT].dc_out)
				OCR0B = all_lights[COLD_LIGHT].dc_out;
		}
	}

	return 0;
}
