#include <avr/io.h>
#include <avr/pgmspace.h>
#include "nyan.h"

volatile uint16_t sample;

void wplay(volatile uint8_t *out, volatile uint8_t *clock, int count);

/*
 * Play wav file
 * *out: Pointer to PWM pin for output
 * *clock: Pointer to the counter
 *  count: Amount of cycles (according to the prescalar you've set
 *		for *clock) that it takes to achieve the rate of wav, minus 1
 *		(ex. to achieve 8kHz with /8 prescalar in 16MHz, this is 249)
 */
void wplay(volatile uint8_t *out, volatile uint8_t *clock, int count) {
	for (sample = 0; sample < pcm_length;) {
		if (*clock >= count) {
			*out = pgm_read_byte(&pcm_data[sample++]);
			*clock = 0;
		}
	}
}

int main(void) {
	DDRD |= _BV(3); /* Speaker */

	/* Set clk/8 prescalar for the counter0 */
	TCCR0B |= _BV(CS01);

	/* Set fast PWM without prescalar for the counter2 */
	TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
	TCCR2B = _BV(CS20);

	while (1)
		/* To find clock tick: <CPU frequency>/8 (due to the prescalara) and devide
		   by <Sample rate> (which is 8000). Remove one due to the counter
		   nature (zero based) */
		wplay(&OCR2B, &TCNT0, (F_CPU/8/8000) - 1);

	return 0;
}
