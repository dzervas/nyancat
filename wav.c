#include <avr/pgmspace.h>
#include "nyan.h"

volatile uint16_t sample;

/*
 * Play wav file
 * *out: Pointer to PWM pin for output
 * *clock: Pointer to the counter
 *  count: Amount of cycles (according to the prescalar you've set
 *		for *clock) that it takes to achieve the rate of wav, minus 1
 *		(ex. to achieve 8kHz with /8 prescalar in 16MHz, this is 249)
 */
void wplay(int *out, int *clock, int count) {
	for (sample = 0; sample < pcm_length; sample++) {
		if (*clock >= count) {
			*out = pgm_read_byte(&pcm_data[sample]);
			*clock = 0;
		}
	}
}
