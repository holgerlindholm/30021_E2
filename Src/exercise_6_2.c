#include "timer.h"
#include "stm32f30x.h"

void exercise_6_2(void) {
	while (1) {
		// To activate the sensor a HIGH pulse of at least 10μs must be put on Trig.
		//Once the trigger pulse goes back to LOW 8 sonic pulses at 40 kHz are transmitted from the sensor and the Echo pin goes to HIGH
		//and stays like this until the reflection of the 8 sonic pulses is detected. To determine the distance the time that the
		// Echo pin is HIGH must be measured.

		// Trigger of wifth minimi  10 mus, trigger max rep rate 50 mus (GPIO)

		// Echo detect pulse width in muS --> conert to range (cm) by muS / 58 (GPIO)

	}
}

