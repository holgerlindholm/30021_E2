#include "stm32f30x_conf.h"
#include "30010_io.h"
#include <stdio.h>
#include <inttypes.h>

#include "openlog.h"
#include "exercises.h"
#include "30010_io.h"

int main(void) {
	uart_init(9600);

	exercise_5();

	while(1)
	{

	}
}
