#include <avr/io.h>
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
void wplay(volatile uint8_t *out, volatile uint8_t *clock, int count) {
	for (sample = 0; sample < pcm_length;) {
		if (*clock >= count) {
			*out = pgm_read_byte(&pcm_data[sample++]);
			*clock = 0;
		}
	}
}

int main(void) {
	DDRD |= (1 << 3); // Speaker

	/* Set clk/8 prescalar for the counter0 */
	TCCR0B |= (1 << CS01);

	/* Set fast PWM without prescalar for the counter2 */
	TCCR2A = (1 << COM2A1) | (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);
	TCCR2B = (1 << CS20);

	while (1)
		wplay(&OCR2B, &TCNT0, 249);

	return 0;
}
