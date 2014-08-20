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
	DDDR |= _BV(DSCK) | _BV(DSDIN) | _BV(DDC) | _BV(DSCE) | _BV(DRES);

	DPORT = 0;
	DPORT |= _BV(DSCE) | _BV(DRES);
	DPORT &= ~_BV(DSCK) & ~_BV(DSDIN) & ~_BV(DDC);
	_delay_ms(2);

	DPORT &= ~_BV(DRES);
	_delay_ms(2);
	DPORT |= _BV(DRES);
	_delay_ms(2);

	/* Initiate screen (see datasheet p.14) */
	dsend(_BV(5) | 1);		/* Set function set with PD=0, V=0, H=1 */
	dsend(_BV(2) | 0);		/* Set temperature coefficient to 0 (0-3) */
	dsend(_BV(4) | 3);		/* Set bias mode 1:48 (0-7) */
	dsend(_BV(7) | 52);		/* Set contrast to 52 (0-127) */
	dsend(_BV(5));		/* Set function set with PD=0, V=0, H=0 */
	dsend(_BV(3) | _BV(2));	/* Set screen to normal mode */
	_delay_ms(200);

	DPORT |= _BV(DDC);
	dsend(255);
}

void dsend(uint8_t data) {
	DPORT &= ~_BV(DSCE);
	_delay_ms(2);

	for (uint8_t i = 7; i >= 0; i++) {
		DPORT |= (((data & _BV(i)) >> i) << DSDIN);
		_delay_ms(2);

		DPORT &= ~_BV(DSCK);
		_delay_ms(2);
		DPORT |= _BV(DSCK);
		_delay_ms(2);

		DPORT &= ~_BV(DSDIN);
	}

	DPORT |= _BV(DSCE);
}

int main(void) {
	DDRD |= _BV(3); /* Speaker */

	/* Set clk/8 prescalar for the counter0 */
	TCCR0B |= _BV(CS01);

	/* Set fast PWM without prescalar for the counter2 */
	TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
	TCCR2B = _BV(CS20);

	dinit();

	while (1)
		wplay(&OCR2B, &TCNT0, 249);

	return 0;
}
