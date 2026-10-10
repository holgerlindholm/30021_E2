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
	init_spi_lsm9ds1();
	init_AG();

	int16_t temp_raw, data_raw_xl[3], data_raw_gy[3];
	int16_t offset_gy_raw[3], offset_xl_raw[3];

	float temp;
	float xl_x, xl_y, xl_z;
	float gy_x, gy_y, gy_z;

	// Calibrate static offsets when the board is sitting still.
	calibrate_gy(offset_gy_raw);
	calibrate_xl(offset_xl_raw);

	float gy_offset_x = fs2000dps_to_mdps(offset_gy_raw[0]);
	float gy_offset_y = fs2000dps_to_mdps(offset_gy_raw[1]);
	float gy_offset_z = fs2000dps_to_mdps(offset_gy_raw[2]);
	float xl_offset_x = fs4g_to_mg(offset_xl_raw[0]);
	float xl_offset_y = fs4g_to_mg(offset_xl_raw[1]);
	float xl_offset_z = fs4g_to_mg(offset_xl_raw[2]);

	printf("Accelerometer offset = %f4.2 [mdps]\t Y = %4.2f [mdps]\t Z = %4.2f [mdps]\n", gy_offset_x, gy_offset_y, gy_offset_z);
	printf("Gyroscope offset = %f4.2 [mg]\t Y = %4.2f [mg]\t Z = %4.2f [mg]\n", xl_offset_x, xl_offset_y, xl_offset_z);

    // Magnetometer calibration: rotate the board in all directions while this runs
    printf("Rotate the board in all directions...\n");
    // This calibrates and saves the offsets to the offset registers
    mag_calibrate(800); // about 10 s at 80 Hz (We can change sampling rate using Table: 111

    mag_read_offsets(&off_x, &off_y, &off_z);
    printf("Offsets: X=%d Y=%d Z=%d\n", off_x, off_y, off_z); // Check that it is non zero

	while (1) {
		// Read values from lsm9ds1
		temp_raw = read_temp();
		read_xl(data_raw_xl);
		read_gy(data_raw_gy);

		// Convert values from int16 to float
		temp = temp_raw_to_float(temp_raw);

		xl_x = fs4g_to_mg(data_raw_xl[0] - offset_xl_raw[0]);
		xl_y = fs4g_to_mg(data_raw_xl[1] - offset_xl_raw[1]);
		xl_z = fs4g_to_mg(data_raw_xl[2] - offset_xl_raw[2]);

		gy_x = fs2000dps_to_mdps(data_raw_gy[0] - offset_gy_raw[0]);
		gy_y = fs2000dps_to_mdps(data_raw_gy[1] - offset_gy_raw[1]);
		gy_z = fs2000dps_to_mdps(data_raw_gy[2] - offset_gy_raw[2]);

		printf("Temperature = %2.2f[°C]\n",temp);
		printf("Accelerometer readings\tX = %4.2f [mg]\t Y = %4.2f [mg]\t Z = %4.2f [mg]\n", xl_x, xl_y, xl_z);
		printf("Gyro readings\tX = %4.2f [mdps]\t Y = %4.2f [mdps]\t Z = %4.2f [mdps]\n", gy_x, gy_y, gy_z);

		// Read magnetometer data using adresses $mag_x
		mag_read_xyz(&mag_x, &mag_y, &mag_z);
		float x_mg = mag_raw_to_mgauss(mag_x, scale);
		float y_mg = mag_raw_to_mgauss(mag_y, scale);
		float z_mg = mag_raw_to_mgauss(mag_z, scale);

		printf("X=%d Y=%d Z=%d (raw)\n", x_mg, y_mg, z_mg);

		// lsm9ds1_write(WHO_AM_I, 0xAA);
	}
}
