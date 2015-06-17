/*
 * light.c
 *
 *  Created on: 13/6/2015
 *      Author: camilo
 */

#include <stdio.h>
#include <unistd.h>

#define MIN_ON 10
#define MIN_OFF 90
#define FULL_CYCLE 100
#define HALF_CYCLE 50
#define DEFAULT_OFF_WARM (HALF_CYCLE/2)
#define DEFAULT_OFF_COLD 0
#define OFF 0
#define ON 1

const unsigned char table_flog100[100] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
                1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5,
                5, 5, 5, 6, 6, 6, 7, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 14,
                14, 15, 16, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 27, 28, 29, 31, 32,
                34, 36, 37, 39, 41, 43, 45, 47, 50, 52, 54, 57, 60, 63, 66, 69, 72, 76,
                79, 83, 87, 91, 95, 100 };
const unsigned char table_mlog100[100] = { 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
                7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16,
                17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25,
                26, 26, 27, 27, 28, 28, 29, 29, 30, 30, 31, 31, 32, 32, 33, 33, 34, 34,
                35, 35, 36, 36, 37, 37, 38, 38, 39, 39, 40, 40, 41, 43, 45, 47, 50, 52,
                54, 57, 60, 63, 66, 69, 72, 76, 79, 83, 87, 91, 95, 100 };
const unsigned char table_flog255[100] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
                1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6,
                6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 12, 13, 13, 14, 15, 16, 17, 18, 19, 20,
                21, 23, 24, 25, 27, 28, 30, 32, 34, 36, 38, 40, 42, 45, 48, 50, 53, 56,
                60, 63, 67, 71, 75, 79, 83, 88, 93, 99, 104, 110, 117, 123, 131, 138,
                146, 154, 163, 173, 183, 193, 204, 216, 228, 241, 255 };
const unsigned char table_mlog255[100] = { 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6,
                6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16,
                16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25,
                25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 30, 31, 31, 32, 34, 36, 38, 40,
                42, 45, 48, 50, 53, 56, 60, 63, 67, 71, 75, 79, 83, 88, 93, 99, 104,
                110, 117, 123, 131, 138, 146, 154, 163, 173, 183, 193, 204, 216, 228,
                241, 255 };

static unsigned short long_warm_ref, long_cold_ref;
static unsigned char step_counter_warm, step_counter_cold;
static unsigned char step_warm, step_cold;
static unsigned char off_warm, off_cold;
static unsigned short long_warm, long_cold;
static int out_warm, out_cold;

void soft_pwm() {
    static unsigned char dc_warm = 0, dc_cold = HALF_CYCLE;
    static char local_warm = 0;
    static char local_cold = 0;

    out_warm = local_warm;
    out_cold = local_cold;

    /* Process end of a cycle for warm light */
    if (FULL_CYCLE <= ++dc_warm) {
        dc_warm = 0;
        /* Turn on light if necessary */
        if (MIN_ON <= off_warm) {
            local_warm = ON;
        }
        /* Process gradients */
        if ((0 != step_warm) && (long_warm_ref < ++long_warm)) {
            long_warm = 0;
            if (0 < step_counter_warm) {
                step_counter_warm--;
                off_warm += step_warm;
                if ((MIN_OFF < off_warm) || (MIN_ON > off_warm)) {
                    step_counter_warm = 0;
                    step_warm = 0;
                }
            }
        }
    }

    /* Process end of a cycle for cold light */
    if (FULL_CYCLE <= ++dc_cold) {
        dc_cold = 0;
        /* Turn on light if necessary */
        if (MIN_ON <= off_cold) {
            local_cold = ON;
        }
        /* Process gradients */
        if ((0 != step_cold) && (long_cold_ref < ++long_cold)) {
            long_cold = 0;
            if (0 < step_counter_cold) {
                step_counter_cold--;
                off_cold += step_cold;
                if ((MIN_OFF < off_cold) || (MIN_ON > off_cold)) {
                    step_counter_cold = 0;
                    step_cold = 0;
                }
            }
        }
    }

    /* Decide next output for warm light */
    if ((dc_warm == off_warm) && (MIN_OFF >= off_warm)) {
        local_warm = OFF;
    }

    /* Decide next output for cold light */
    if ((dc_cold == off_cold) && (MIN_OFF >= off_cold)) {
        local_cold = OFF;
    }
}

int main() {
    unsigned int super_long_count = 0;

    /* Initialize variables */
    long_cold_ref = long_warm_ref = 0;
    step_counter_warm = step_counter_cold = 0;
    step_warm = step_cold = 0;
    off_warm = HALF_CYCLE;
    off_cold = 0;
    long_warm = long_cold = 0;

    while (1) {
        soft_pwm();
        usleep(100);
    }
}
