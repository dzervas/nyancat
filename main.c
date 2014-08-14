#include <avr/io.h>
#include "wav.h"

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
