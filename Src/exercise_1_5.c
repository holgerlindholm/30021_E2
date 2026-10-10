#include "gpio.h"

void exercise_1_5(void) {
	float number = 3.1415926535;
	char str[100];
	sprintf(str,"sprint floating point value, 8 digits %.8f \n", number);
	printf("%s\n",str);
}
