#include "lsm9ds1.h"

void exercise_4_1(void) {
	init_spi_lsm9ds1();

	uint8_t data_out8;
	uint16_t data_out16;

	int16_t mag_x,mag_y,mag_z; //variables to hold magnetometer values
	uint8_t scale = lsm9ds1_read8(CTRL_REG2_M);   // read the current range setting once

    if (mag_init() != 0) {
        printf("LSM9DS1 magnetometer not found\n");
        while (1);
    }

	while (1) {
//		data_out8 = lsm9ds1_read8(WHO_AM_I);
//		printf("Received data = %X\n", data_out8);

//		data_out16 = lsm9ds1_read16(WHO_AM_I);
//		printf("Received data = %X\n", data_out16);

		// Read magnetometer data using adresses $mag_x
		mag_read_xyz(&mag_x, &mag_y, &mag_z);
		float x_mg = mag_raw_to_mgauss(mag_x, scale);
		float y_mg = mag_raw_to_mgauss(mag_y, scale);
		float z_mg = mag_raw_to_mgauss(mag_z, scale);

		printf("X=%d Y=%d Z=%d (raw)\n", x_mg, y_mg, z_mg);

		// lsm9ds1_write(WHO_AM_I, 0xAA);
	}
}
