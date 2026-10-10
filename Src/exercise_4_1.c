#include "lsm9ds1.h"

void exercise_4_1(void) {
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

	}
}
