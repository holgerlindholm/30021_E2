#include "lsm9ds1.h"

void exercise_4_1(void) {
    init_spi_lsm9ds1();

    int16_t mag_x, mag_y, mag_z;           // raw magnetometer values
    int16_t off_x, off_y, off_z;           // offsets stored in the sensor

    if (mag_init() != 0) {
        printf("LSM9DS1 magnetometer not found\n");
        while (1);
    }

    uint8_t scale = lsm9ds1_read8(CTRL_REG2_M);   // Currently it is set to +- 4gauss

    // For testing read and write
	uint8_t data_out8;
	uint16_t data_out16;

    // Magnetometer calibration: rotate the board in all directions while this runs
    printf("Rotate the board in all directions...\n");
    // This calibrates and saves the offsets to the offset registers
    mag_calibrate(800); // about 10 s at 80 Hz (We can change sampling rate using Table: 111

    mag_read_offsets(&off_x, &off_y, &off_z);
    printf("Offsets: X=%d Y=%d Z=%d\n", off_x, off_y, off_z); // Check that it is non zero

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
