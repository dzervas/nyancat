#include <stdint.h>
#include <avr/io.h>
//#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "nyan.h"

#define SAMPLE_RATE 8000;
#define outLow(port,pin) port &= ~(1 << pin)
#define outHigh(port,pin) port |= (1 << pin)
#define outToggle(port,pin) port ^= (1 << pin)
#define setIn(portdir,pin) portdir &= ~(1 << pin)
#define setOut(portdir,pin) portdir |= (1 << pin)

volatile uint16_t sample;

int main(void) {
	sample = 0;

	setOut(DDRD, 3); // Speaker
	setOut(DDRD, 7);
	outHigh(PORTD, 7);

	/* Set clk/8 prescalar for the counter0 */
	TCCR0B |= (1 << CS01);
	/* Set fast PWM without prescalar for the counter2 */
	TCCR2A = (1 << COM2A1) | (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);
	TCCR2B = (1 << CS20);

	while (1) {

		if (TCNT0 >= 249) {
			outLow(PORTD, 7);
			OCR2B = pgm_read_byte(&pcm_data[sample++]);
			TCNT0 = 0;
		}

		if(sample >= pcm_length)
		 	 sample = 0;
	}

	return 0;
}
