#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "nyan.h"

#define DPORT	PORTC
#define DDDR	DDRC
#define DSCK	5
#define DSDIN	4
#define DDC	3
#define DSCE	2
#define DRES	1

volatile uint16_t sample;

void wplay(volatile uint8_t *out, volatile uint8_t *clock, int count);
void dinit();
void dsend(uint8_t data);

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

void dinit() {
	DDDR = 0;
	DDDR |= (1 << DSCK) | (1 << DSDIN) | (1 << DDC) | (1 << DSCE) | (1 << DRES);

	DPORT = 0;
	DPORT |= (1 << DSCE) | (1 << DRES);
	DPORT &= ~(1 << DSCK) & ~(1 << DSDIN) & ~(1 << DDC);

	DPORT &= ~(1 << DRES);
	DPORT |= (1 << DRES);
	_delay_ms(200);

	/* Initiate screen (see datasheet p.14) */
	dsend((1 << 5) | 1);		/* Set function set with PD=0, V=0, H=1 */
	dsend((1 << 2) | 0);		/* Set temperature coefficient to 0 (0-3) */
	dsend((1 << 4) | 3);		/* Set bias mode 1:48 (0-7) */
	dsend((1 << 7) | 52);		/* Set contrast to 52 (0-127) */
	dsend((1 << 5));		/* Set function set with PD=0, V=0, H=0 */
	dsend((1 << 3) | (1 << 2));	/* Set screen to normal mode */
	_delay_ms(200);

	DPORT |= (1 << DDC);
	dsend(255);
}

void dsend(uint8_t data) {
	DPORT &= ~(1 << DSCE);

	for (uint8_t i = 0; i < 8; i++) {
		DPORT |= (((data & (1 << i)) >> i) << DSDIN);
		_delay_ms(25);

		DPORT &= ~(1 << DSCK);
		_delay_ms(25);
		DPORT |= (1 << DSCK);
		_delay_ms(25);

		DPORT &= ~(1 << DSDIN);
	}

	DPORT |= (1 << DSCE);
}

int main(void) {
	DDRD |= (1 << 3); /* Speaker */

	/* Set clk/8 prescalar for the counter0 */
	TCCR0B |= (1 << CS01);

	/* Set fast PWM without prescalar for the counter2 */
	TCCR2A = (1 << COM2A1) | (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);
	TCCR2B = (1 << CS20);

	dinit();

	while (1)
		wplay(&OCR2B, &TCNT0, 249);

	return 0;
}
