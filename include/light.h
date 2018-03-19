/*
 * light.h
 *
 *  Created on: 30/07/2015
 *      Author: Camilo Arboleda
 */

#ifndef INCLUDE_LIGHT_H_
#define INCLUDE_LIGHT_H_

#define F_CPU 16000000UL

#define stb(byte, bit) (byte |= _BV(bit))
#define clb(byte, bit) (byte &= ~_BV(bit))
#define tgb(byte, bit) (byte ^= _BV(bit))

#define FULL_CYCLE 0xFF
#define OFF 0
#define ON 1

/* Data structure for controling light behavior.
 * */
typedef struct light_struct light_t;
struct __attribute__((packed)) light_struct {
	char step;				// 0 : Duty cycle auto-increment value
	unsigned char dc;			// 1 : Perceived Duty Cycle (128 = 50% brightness)
	unsigned char step_count;		// 2 : Number of auto-increment steps
	unsigned char dc_out;			// 3 : Actual Duty Cycle (128 = 50% DC)
	unsigned short long_ref;		// 4 - 5 : Time between auto-increments (10ms resolution)
	unsigned short long_count;		// 6 - 7 
	// This structure MUST BE 16 bytes
	unsigned char RFU[8];
};

/*
typedef enum light_registers {
	STEP_REG		= GET_MEMBER_OFFSET(step, light_t),
	DUTY_CYCLE_REG		= GET_MEMBER_OFFSET(dc, light_t),
	STEP_COUNT_REG		= GET_MEMBER_OFFSET(step_counter, light_t),
	OUT_CYCLE_REG		= GET_MEMBER_OFFSET(dc_out, light_t),
	LONG_COUNT_L_REG	= GET_MEMBER_OFFSET(long_count, light_t),
	LONG_COUNT_H_REG 	= GET_MEMBER_OFFSET(long_count, light_t) + 1,
	LONG_REF_L_REG 		= GET_MEMBER_OFFSET(long_ref, light_t),
	LONG_REF_H_REG 		= GET_MEMBER_OFFSET(long_ref, light_t) + 1,
} light_registers_t;
*/

enum light_name {
	WARM_LIGHT,
	COLD_LIGHT,
	N_LIGHT,
};

#define MAX_REG (N_LIGHT * sizeof(light_t))

extern light_t all_lights[N_LIGHT];

#endif /* INCLUDE_LIGHT_H_ */
