/*
 * timer.h
 *
 *  Created on: 30/07/2015
 *      Author: Camilo Arboleda
 */

#ifndef INCLUDE_TIMER_H_
#define INCLUDE_TIMER_H_

#include "light.h"

extern volatile unsigned char timer_interrupt_req;

// #define PWM60Hz
#define PWM120Hz

void pwm_setup();

#endif /* INCLUDE_TIMER_H_ */
