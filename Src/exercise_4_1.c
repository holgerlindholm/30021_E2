#include "lsm9ds1.h"

void exercise_4_1(void) {
	printf("Build: %s %s\n", __DATE__, __TIME__);
    init_spi_lsm9ds1();
    init_AG();
    mag_init();

    uint8_t AG_status;
    int16_t temp_raw, data_raw_xl[3], data_raw_gy[3];
	int16_t offset_gy_raw[3], offset_xl_raw[3];
    int16_t mag_x, mag_y, mag_z;           // raw magnetometer values
    int16_t off_x, off_y, off_z;           // offsets stored in the sensor

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
    printf("Magnetometer offsets: X=%d Y=%d Z=%d\n", off_x, off_y, off_z); // Check that it is non zero

	while (1) {
		AG_status = get_AG_status();

		// Read values from temperature sensor
		if (AG_status & 0x04) {
			temp_raw = read_temp();

			// Convert values from int16 to float
			temp = temp_raw_to_float(temp_raw);
			printf("Temperature = %2.2f[°C]\n",temp);
		}

		// Read values from Gyroscope
		if (AG_status & 0x02) {
			read_gy(data_raw_gy);

			gy_x = fs2000dps_to_mdps(data_raw_gy[0] - offset_gy_raw[0]);
			gy_y = fs2000dps_to_mdps(data_raw_gy[1] - offset_gy_raw[1]);
			gy_z = fs2000dps_to_mdps(data_raw_gy[2] - offset_gy_raw[2]);

			printf("Gyroscope readings\tX = %4.2f [md ps]\t Y = %4.2f [mdps]\t Z = %4.2f [mdps]\n", gy_x, gy_y, gy_z);
		}

		// Read values from Accelerometer
		if (AG_status & 0x01) {
			read_xl(data_raw_xl);
			xl_x = fs4g_to_mg(data_raw_xl[0] - offset_xl_raw[0]);
			xl_y = fs4g_to_mg(data_raw_xl[1] - offset_xl_raw[1]);
			xl_z = fs4g_to_mg(data_raw_xl[2] - offset_xl_raw[2]);
			printf("Accelerometer readings\tX = %4.2f [mg]\t Y = %4.2f [mg]\t Z = %4.2f [mg]\n", xl_x, xl_y, xl_z);
		}

		// Read magnetometer data using adresses $mag_x
		mag_read_xyz(&mag_x, &mag_y, &mag_z);
		float x_mg = mag_raw_to_mgauss(mag_x);
		float y_mg = mag_raw_to_mgauss(mag_y);
		float z_mg = mag_raw_to_mgauss(mag_z);

		// lsm9ds1_write(WHO_AM_I, 0xAA);
		printf("Magnetometer readings\tX = %4.2f [mgauss]\t Y = %4.2f [mgauss]\t Z = %4.2f [mgauss]\n", x_mg, y_mg, z_mg);
//		printf("X=%d\t Y=%d\t Z=%d\t (raw)\n",
//		       mag_x, mag_y, mag_z);
	}

     once, before the loop
     It is possible to save to a file using logging in putty
    printf("temp,gx,gy,gz,ax,ay,az,mx,my,mz\n");

    while (1) {
        AG_status = get_AG_status();

        if ((AG_status & 0x07) == 0x07) {          // temp + gyro + accel all ready
            temp = temp_raw_to_float(read_temp());

            read_gy(data_raw_gy);
            gy_x = fs2000dps_to_mdps(data_raw_gy[0] - offset_gy_raw[0]);
            gy_y = fs2000dps_to_mdps(data_raw_gy[1] - offset_gy_raw[1]);
            gy_z = fs2000dps_to_mdps(data_raw_gy[2] - offset_gy_raw[2]);

            read_xl(data_raw_xl);
            xl_x = fs4g_to_mg(data_raw_xl[0] - offset_xl_raw[0]);
            xl_y = fs4g_to_mg(data_raw_xl[1] - offset_xl_raw[1]);
            xl_z = fs4g_to_mg(data_raw_xl[2] - offset_xl_raw[2]);

            mag_read_xyz(&mag_x, &mag_y, &mag_z);

            printf("%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
                   temp, gy_x, gy_y, gy_z, xl_x, xl_y, xl_z,
                   mag_raw_to_mgauss(mag_x),
                   mag_raw_to_mgauss(mag_y),
                   mag_raw_to_mgauss(mag_z));
        }
    }
}
