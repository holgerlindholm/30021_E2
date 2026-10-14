#include "lsm9ds1.h"

void exercise_4_1(void) {
	printf("Build: %s %s\n", __DATE__, __TIME__);
    init_spi_lsm9ds1();
    init_AG();
    mag_init();

    uint8_t AG_status;
    uint8_t M_status;
    int16_t data_raw_xl[3] = {0, 0, 0}, data_raw_gy[3] = {0, 0, 0};
	int16_t offset_gy_raw[3], offset_xl_raw[3];
    int16_t mag_x, mag_y, mag_z;           // raw magnetometer values
    int16_t off_x, off_y, off_z;           // offsets stored in the sensor

	float temp;
	float xl_x, xl_y, xl_z;
	float gy_x, gy_y, gy_z;
	float x_mg, y_mg, z_mg;

	// Calibrate static offsets when the board is sitting still.
	calibrate_gy(offset_gy_raw);
	// calibrate_xl(offset_xl_raw);

	float gy_offset_x = fs2000dps_to_mdps(offset_gy_raw[0]);
	float gy_offset_y = fs2000dps_to_mdps(offset_gy_raw[1]);
	float gy_offset_z = fs2000dps_to_mdps(offset_gy_raw[2]);
//	float xl_offset_x = fs4g_to_mg(offset_xl_raw[0]);
//	float xl_offset_y = fs4g_to_mg(offset_xl_raw[1]);
//	float xl_offset_z = fs4g_to_mg(offset_xl_raw[2]);


	printf("Accelerometer offset = %f4.2 [mdps]\t Y = %4.2f [mdps]\t Z = %4.2f [mdps]\n", gy_offset_x, gy_offset_y, gy_offset_z);
	//printf("Gyroscope offset = %f4.2 [mg]\t Y = %4.2f [mg]\t Z = %4.2f [mg]\n", xl_offset_x, xl_offset_y, xl_offset_z);

    // Magnetometer calibration: rotate the board in all directions while this runs
	// This calibrates and saves the offsets to the offset registers
    printf("Rotate the board in all directions...\n");
    mag_calibrate(800000); // See Table: 111

    mag_read_offsets(&off_x, &off_y, &off_z);
    printf("Magnetometer offsets: X=%d Y=%d Z=%d\n", off_x, off_y, off_z); // Check that it is non zero


    while (1) {
        AG_status = get_AG_status();
        M_status = get_M_status();

        if (AG_status & 0x04) {
            temp = temp_raw_to_float(read_temp());
        }

        if (AG_status & 0x02) {
            read_gy(data_raw_gy);
            gy_x = fs2000dps_to_mdps(data_raw_gy[0] - offset_gy_raw[0]);
            gy_y = fs2000dps_to_mdps(data_raw_gy[1] - offset_gy_raw[1]);
            gy_z = fs2000dps_to_mdps(data_raw_gy[2] - offset_gy_raw[2]);
        }

        if (AG_status & 0x01) {
            read_xl(data_raw_xl);
            xl_x = fs4g_to_mg(data_raw_xl[0] - offset_xl_raw[0]);
            xl_y = fs4g_to_mg(data_raw_xl[1] - offset_xl_raw[1]);
            xl_z = fs4g_to_mg(data_raw_xl[2] - offset_xl_raw[2]);
        }

        if (M_status & 0x08) {
			// Read magnetometer data using adresses $mag_x
			mag_read_xyz(&mag_x, &mag_y, &mag_z);
			x_mg = mag_raw_to_mgauss(mag_x);
			y_mg = mag_raw_to_mgauss(mag_y);
			z_mg = mag_raw_to_mgauss(mag_z);
        }

            printf("%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
                   temp,
				   gy_x, gy_y, gy_z,
				   xl_x, xl_y, xl_z,
				   x_mg, y_mg, z_mg);
    }
}
