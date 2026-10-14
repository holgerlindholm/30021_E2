#include "lsm9ds1.h"
//#include "lsm9ds1_reg.h"

float_t temp_raw_to_float(int16_t temp_raw) {
	return 25.0f + (temp_raw / 16.0f);
}

float_t fs4g_to_mg(int16_t xl_raw){
	  return ((float_t)xl_raw * 0.122f);
}

float_t fs2000dps_to_mdps(int16_t gy_raw)
{
  return ((float_t)gy_raw * 70.0f);
}

// Convert a raw magnetometer reading to milligauss depening on scale
// scale is located in FS[1:0] in CTRL_REG2_M (bit value 5 and 6)
float mag_raw_to_mgauss(int16_t raw)
{
    uint8_t ctrl_reg2 = SPI_read8(CS_M_PIN,LSM9DS1_CTRL_REG2_M );

    // Extract FS[1:0] (bits 6:5)
    switch ((ctrl_reg2 >> 5) & 0x03)
    {
        case 0:  return raw * 0.14f;  // ±4 gauss
        case 1:  return raw * 0.29f;  // ±8 gauss
        case 2:  return raw * 0.43f;  // ±12 gauss
        case 3:  return raw * 0.58f;  // ±16 gauss
        default: return 0.0f;         // should never happen
    }
}

void init_spi_lsm9ds1(void) {
	// Enable Clocks
	RCC->AHBENR |= 0x00020000 | 0x00040000; // Enable Clock for GPIO Banks A and B
	RCC->APB1ENR |= 0x00004000;                 // Enable Clock for SPI2

	// Connect pins to SPI2
	GPIOB->AFR[13 >> 0x03] &= ~(0x0000000F << ((13 & 0x00000007) * 4)); // Clear alternate function for PB13
	GPIOB->AFR[13 >> 0x03] |= (0x00000005 << ((13 & 0x00000007) * 4)); // Set alternate 5 function for PB13 - SCLK
	GPIOB->AFR[15 >> 0x03] &= ~(0x0000000F << ((15 & 0x00000007) * 4)); // Clear alternate function for PB15
	GPIOB->AFR[15 >> 0x03] |= (0x00000005 << ((15 & 0x00000007) * 4)); // Set alternate 5 function for PB15 - MOSI

	// Configure pins PB13 and PB15 for 10 MHz alternate function
	GPIOB->OSPEEDR &= ~(0x00000003 << (13 * 2) | 0x00000003 << (15 * 2)); // Clear speed register
	GPIOB->OSPEEDR |= (0x00000001 << (13 * 2) | 0x00000001 << (15 * 2)); // set speed register (0x01 - 10 MHz, 0x02 - 2 MHz, 0x03 - 50 MHz)
	GPIOB->OTYPER &= ~(0x0001 << (13) | 0x0001 << (15)); // Clear output type register
	GPIOB->OTYPER |= (0x0000 << (13) | 0x0000 << (15)); // Set output type register (0x00 - Push pull, 0x01 - Open drain)
	GPIOB->MODER &= ~(0x00000003 << (13 * 2) | 0x00000003 << (15 * 2)); // Clear mode register
	GPIOB->MODER |= (0x00000002 << (13 * 2) | 0x00000002 << (15 * 2)); // Set mode register (0x00 - Input, 0x01 - Output, 0x02 - Alternate Function, 0x03 - Analog in/out)
	GPIOB->PUPDR &= ~(0x00000003 << (13 * 2) | 0x00000003 << (15 * 2)); // Clear push/pull register
	GPIOB->PUPDR |= (0x00000000 << (13 * 2) | 0x00000000 << (15 * 2)); // Set push/pull register (0x00 - No pull, 0x01 - Pull-up, 0x02 - Pull-down)

	// Initialize REEST, nCS, and A0
	// PB14 = SPI2 MISO
	GPIOB->AFR[14 >> 3] &= ~(0xF << ((14 & 7) * 4));
	GPIOB->AFR[14 >> 3] |= (0x5 << ((14 & 7) * 4));

	GPIOB->MODER &= ~(0x3 << (14 * 2));
	GPIOB->MODER |= (0x2 << (14 * 2));       // Alternate function

	GPIOB->PUPDR &= ~(0x3 << (14 * 2));
	// optionally:
	// GPIOB->PUPDR |=  (0x1 << (14 * 2));    // pull-up, if appropriate

	// Configure pins PB6 and PB10 for 10 MHz output
	GPIOB->OSPEEDR &= ~(0x00000003 << (6 * 2) | 0x00000003 << (10 * 2)); // Clear speed register
	GPIOB->OSPEEDR |= (0x00000001 << (6 * 2) | 0x00000001 << (10 * 2)); // set speed register (0x01 - 10 MHz, 0x02 - 2 MHz, 0x03 - 50 MHz)
	GPIOB->OTYPER &= ~(0x0001 << (6) | 0x0001 << (10)); // Clear output type register
	GPIOB->OTYPER |= (0x0000 << (6) | 0x0000 << (10)); // Set output type register (0x00 - Push pull, 0x01 - Open drain)
	GPIOB->MODER &= ~(0x00000003 << (6 * 2) | 0x00000003 << (10 * 2)); // Clear mode register
	GPIOB->MODER |= (0x00000001 << (6 * 2) | 0x00000001 << (10 * 2)); // Set mode register (0x00 - Input, 0x01 - Output, 0x02 - Alternate Function, 0x03 - Analog in/out)
	GPIOB->PUPDR &= ~(0x00000003 << (6 * 2) | 0x00000003 << (10 * 2)); // Clear push/pull register
	GPIOB->PUPDR |= (0x00000000 << (6 * 2) | 0x00000000 << (10 * 2)); // Set push/pull register (0x00 - No pull, 0x01 - Pull-up, 0x02 - Pull-down)

	GPIOB->ODR |= (0x0001 << 6); // CS = 1
	GPIOB->ODR |= (0x0001 << 10); // D10 = 1

	// Configure SPI2
	SPI2->CR1 &= 0x3040; // Clear CR1 Register
	SPI2->CR1 |= 0x0000; // Configure direction (0x0000 - 2 Lines Full Duplex, 0x0400 - 2 Lines RX Only, 0x8000 - 1 Line RX, 0xC000 - 1 Line TX)
	SPI2->CR1 |= 0x0104; // Configure mode (0x0000 - Slave, 0x0104 - Master)
	SPI2->CR1 |= 0x0002; // Configure clock polarity (0x0000 - Low, 0x0002 - High)
	SPI2->CR1 |= 0x0001; // Configure clock phase (0x0000 - 1 Edge, 0x0001 - 2 Edge)
	SPI2->CR1 |= 0x0200; // Configure chip select (0x0000 - Hardware based, 0x0200 - Software based)
	SPI2->CR1 |= 0x0008; // Set Baud Rate Prescaler (0x0000 - 2, 0x0008 - 4, 0x0018 - 8, 0x0020 - 16, 0x0028 - 32, 0x0028 - 64, 0x0030 - 128, 0x0038 - 128)
	SPI2->CR1 |= 0x0000; // Set Bit Order (0x0000 - MSB First, 0x0080 - LSB First)
	SPI2->CR2 &= ~0x0F00; // Clear CR2 Register
	SPI2->CR2 |= 0x0700; // Set Number of Bits (0x0300 - 4, 0x0400 - 5, 0x0500 - 6, ...);
	SPI2->I2SCFGR &= ~0x0800; // Disable I2S
	SPI2->CRCPR = 7; // Set CRC polynomial order
	SPI2->CR2 &= ~0x1000;
	SPI2->CR2 |= 0x1000; // Configure RXFIFO return at (0x0000 - Half-full (16 bits), 0x1000 - Quarter-full (8 bits))
	SPI2->CR1 |= 0x0040; // Enable SPI2
}

int init_AG(void) {

//	//------------
//	// Register 1
//	// Set Gyroscope scale = 2000 dps | 0x18
//	AG_write(LSM9DS1_CTRL_REG1_G, LSM9DS1_2000dps);
//	// Set Gyro filter bandwidth = Ultra light | 0x03
//	AG_write(LSM9DS1_CTRL_REG1_G, LSM9DS1_LP_ULTRA_LIGHT);
//	// Set Gyro output data rate = 59.5Hz | 0x40
//	AG_write(LSM9DS1_CTRL_REG1_G, 0x40);
//
//	//------------
//	// Register 2
//	// Set out selection? | 0x02
//	AG_write(LSM9DS1_CTRL_REG2_G, 0x02);
//
//	//------------
//	// Register 3
//	// Set Gyro High pass filter bandwith = Medium | 0x05
//	AG_write(LSM9DS1_CTRL_REG3_G, LSM9DS1_HP_MEDIUM);
//	// Enable Gyro high pass filter | 0x40
//	AG_write(LSM9DS1_CTRL_REG3_G, 0x40);
//	// Angular rate sensor disable low power mode | 0x00
//	AG_write(LSM9DS1_CTRL_REG3_G, 0x00);
//
//	//------------
//	// Register 6
//	// Set Accelerometer scale = 4g | 0x10
//	AG_write(LSM9DS1_CTRL_REG6_XL, LSM9DS1_4g);
//	// Accelerometer filter bandwidth anti-alias = Auto | 0x00
//	AG_write(LSM9DS1_CTRL_REG6_XL, LSM9DS1_AUTO);
//	// Set Accelerometer output data rate = 50Hz | 0x40
//	AG_write(LSM9DS1_CTRL_REG6_XL, 0x40);
//
//	//------------
//	// Register 7
//	// Set filter Low pass bandwith = ODR/50 | 0x80
//	AG_write(LSM9DS1_CTRL_REG7_XL, LSM9DS1_LP_ODR_DIV_50);
//	//Set filter out path = Low pass out | 0x00
//	AG_write(LSM9DS_CTRL_REG7_XL, LSM9DS1_LP_OUT);
	// Check ID
	if (SPI_read8(CS_AG_PIN,LSM9DS1_WHO_AM_I) != LSM9DS1_IMU_ID) return -1;

	// Set Gyroscope, scale = 2000 dps, filter bandwith = ultra light, output data rate = 59.5Hz
	SPI_write(CS_AG_PIN,LSM9DS1_CTRL_REG1_G, 0x5A);

	// Set gyroscope out selection
	SPI_write(CS_AG_PIN,LSM9DS1_CTRL_REG2_G, 0x02);

	// Set Gyroscope Enable, HP filter = Medium, Disable low power mode.
	SPI_write(CS_AG_PIN,LSM9DS1_CTRL_REG3_G, 0x45);

	// Set Accelerometer scale = 4g, bandwidth anti-alias = auto, output data rate = 50Hz
	SPI_write(CS_AG_PIN,LSM9DS1_CTRL_REG6_XL, 0x50);

	//Set Accelerometer LP filter out, LP Bandwith = ODR/50
	SPI_write(CS_AG_PIN,LSM9DS1_CTRL_REG7_XL, 0x80);

	// Read measurement from temperature, gyroscope and accelerometer, to discard initial value.
	int16_t temp_buff[3];
	read_temp();
	read_gy(temp_buff);
	read_xl(temp_buff);

	return 0;
}

// Returns 0 if OK, -1 if the magnetometer isn't found
int mag_init(void)
{
	// CTRL_REG3_M: I2C_DISABLE = 1, SIM = 0 (4-wire SPI, data on SDO_M)
	CS_write(CS_M_PIN,LSM9DS1_CTRL_REG3_M, 0x80); // <- This was a pain in the ass to figure out

	uint8_t id = SPI_read8(CS_M_PIN,LSM9DS1_WHO_AM_I_M);
	printf("WHO_AM_I_M = 0x%02X\n", id);

//	if (M_read8(LSM9DS1_WHO_AM_I_M) != 0x3D) return -1;   // check chip ID

    // Configure the CTRL registers
    CS_write(CS_M_PIN,LSM9DS1_CTRL_REG1_M, 0xFC);       // temp comp on, ultra-high performance XY, 80 Hz
    CS_write(CS_M_PIN,LSM9DS1_CTRL_REG2_M, 0x00);       // +/-4 gauss
    CS_write(CS_M_PIN,LSM9DS1_CTRL_REG4_M, 0x0C);       // ultra-high performance Z
    CS_write(CS_M_PIN,LSM9DS1_CTRL_REG5_M, 0x40);       // BDU on
    return 0;
}

uint8_t SPI_read8(uint8_t cs_pin, uint8_t addr)
{
    uint8_t data_out8;

    // Assert CS (active low)
    GPIOB->ODR &= ~(1 << cs_pin);

    // Send read command (MSB = 1)
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) != SET) {}
    SPI_SendData8(SPI2, addr | 0x80);

    // Discard received byte during address phase
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) != SET) {}
    SPI_ReceiveData8(SPI2);

    // Clock in the data byte
    SPI_SendData8(SPI2, 0x00);

    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) != SET) {}
    data_out8 = SPI_ReceiveData8(SPI2);

    // Deassert CS
    GPIOB->ODR |= (1 << cs_pin);

    return data_out8;
}

int16_t SPI_read16(uint8_t cs_pin, uint8_t low_reg)
{
    uint8_t low  = SPI_read8(cs_pin, low_reg);
    uint8_t high = SPI_read8(cs_pin, low_reg + 1);

    return (int16_t)(((uint16_t)high << 8) | low);
}


void SPI_write(uint8_t cs_pin, uint8_t addr, uint8_t data_in)
{
    // Assert CS (active low)
    GPIOB->ODR &= ~(1 << cs_pin);

    // Send register address
    SPI_SendData8(SPI2, addr);
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) != SET) {}
    SPI_ReceiveData8(SPI2);  // Clear received dummy byte

    // Send data
    SPI_SendData8(SPI2, data_in);
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) != SET) {}
    SPI_ReceiveData8(SPI2);  // Clear received dummy byte

    // Deassert CS
    GPIOB->ODR |= (1 << cs_pin);
}

// Write 2 consecutive bytes to low and high register pair
void SPI_write16(uint8_t cs_pin, uint8_t reg_low, int16_t value)
{
    SPI_write(cs_pin, reg_low,     (uint8_t)(value & 0xFF));        // Low byte
    SPI_write(cs_pin, reg_low + 1, (uint8_t)((value >> 8) & 0xFF)); // High byte
}

// Raw readings (signed 16-bit) using pointers
void mag_read_xyz(int16_t *x, int16_t *y, int16_t *z)
{
    *x = SPI_read16(CS_M_PIN, LSM9DS1_OUT_X_L_M);
    *y = SPI_read16(CS_M_PIN, LSM9DS1_OUT_Y_L_M);
    *z = SPI_read16(CS_M_PIN, LSM9DS1_OUT_Z_L_M);
}

// Write the offsets into the sensor. It subtracts them from every reading.
void mag_write_offset(int16_t x_off, int16_t y_off, int16_t z_off)
{
	SPI_write16(CS_M_PIN,LSM9DS1_OFFSET_X_REG_L_M, x_off);
	SPI_write16(CS_M_PIN,LSM9DS1_OFFSET_Y_REG_L_M, y_off);
	SPI_write16(CS_M_PIN,LSM9DS1_OFFSET_Z_REG_L_M, z_off);
}

void mag_read_offsets(int16_t *x, int16_t *y, int16_t *z)
{
    *x = SPI_read16(CS_M_PIN,LSM9DS1_OFFSET_X_REG_L_M);
    *y = SPI_read16(CS_M_PIN,LSM9DS1_OFFSET_Y_REG_L_M);
    *z = SPI_read16(CS_M_PIN,LSM9DS1_OFFSET_Z_REG_L_M);
}

// Find minimum and maximum magnetometer values while rotating the sensor.
// The center between min and max should ideally be zero.
void mag_calibrate(uint32_t samples)
{
    int16_t min[3] = { 32767, 32767, 32767 };
    int16_t max[3] = {-32768,-32768,-32768 };
    int16_t x, y, z;
    int16_t off_x, off_y, off_z;

    printf("Rotate sensor in all directions...\n");

    // Clear any existing offsets
    mag_write_offset(0, 0, 0);

    // Discard one sample to ensure offsets have taken effect
    mag_read_xyz(&x, &y, &z);

    for (uint32_t n = 0; n < samples; n++)
    {
        mag_read_xyz(&x, &y, &z);

        if (x < min[0]) min[0] = x;
        if (x > max[0]) max[0] = x;

        if (y < min[1]) min[1] = y;
        if (y > max[1]) max[1] = y;

        if (z < min[2]) min[2] = z;
        if (z > max[2]) max[2] = z;
    }

    // Compute hard-iron offsets
    off_x = (int16_t)(((int32_t)max[0] + min[0]) / 2);
    off_y = (int16_t)(((int32_t)max[1] + min[1]) / 2);
    off_z = (int16_t)(((int32_t)max[2] + min[2]) / 2);

    // Program offsets into the LSM9DS1
    mag_write_offset(off_x, off_y, off_z);

    printf("Done calibrating magnetometer \n");
}

uint8_t get_AG_status(void)
{
    return SPI_read8(CS_AG_PIN, LSM9DS1_STATUS_REG);
}

uint8_t get_M_status(void)
{
    return SPI_read8(CS_M_PIN, LSM9DS1_STATUS_REG_M);
}

int16_t read_temp(void)
{
    return SPI_read16(CS_AG_PIN, LSM9DS1_OUT_TEMP_L);
}

//void read_gy(int16_t *value) {
//	value[0] = ((int16_t)AG_read8(LSM9DS1_OUT_X_H_G) << 8) | (int16_t)AG_read8(LSM9DS1_OUT_X_L_G);
//	value[1] = ((int16_t)AG_read8(LSM9DS1_OUT_Y_H_G) << 8) | (int16_t)AG_read8(LSM9DS1_OUT_Y_L_G);
//	value[2] = ((int16_t)AG_read8(LSM9DS1_OUT_Z_H_G) << 8) | (int16_t)AG_read8(LSM9DS1_OUT_Z_L_G);
//
////	printf("Gyroscope raw readings:\tX = %X\t Y = %X\t Z = %X\n",value[0],value[1],value[2]);
//}
//
//void read_xl(int16_t *value) {
//	value[0] = ((int16_t)AG_read8(LSM9DS1_OUT_X_H_XL) << 8) | (int16_t)AG_read8(LSM9DS1_OUT_X_L_XL);
//	value[1] = ((int16_t)AG_read8(LSM9DS1_OUT_Y_H_XL) << 8) | (int16_t)AG_read8(LSM9DS1_OUT_Y_L_XL);
//	value[2] = ((int16_t)AG_read8(LSM9DS1_OUT_Z_H_XL) << 8) | (int16_t)AG_read8(LSM9DS1_OUT_Z_L_XL);
//
////	printf("Accelerometer raw readings:\tX = %X\t Y = %X\t Z = %X\n",value[0],value[1],value[2]);
//}

void read_gy(int16_t *value)
{
    value[0] = SPI_read16(CS_AG_PIN, LSM9DS1_OUT_X_L_G);
    value[1] = SPI_read16(CS_AG_PIN, LSM9DS1_OUT_Y_L_G);
    value[2] = SPI_read16(CS_AG_PIN, LSM9DS1_OUT_Z_L_G);

    // printf("Gyroscope raw readings:\tX = %X\t Y = %X\t Z = %X\n",
    //        value[0], value[1], value[2]);
}

void read_xl(int16_t *value)
{
    value[0] = SPI_read16(CS_AG_PIN, LSM9DS1_OUT_X_L_XL);
    value[1] = SPI_read16(CS_AG_PIN, LSM9DS1_OUT_Y_L_XL);
    value[2] = SPI_read16(CS_AG_PIN, LSM9DS1_OUT_Z_L_XL);

    // printf("Accelerometer raw readings:\tX = %X\t Y = %X\t Z = %X\n",
    //        value[0], value[1], value[2]);
}

void calibrate_gy(int16_t *offset){
	printf("Calibrating Gyroscope.\n Keep board steady!\n");

	offset[0] = 0; offset[1] = 0; offset[2] = 0;

	int samples = 10;
	int16_t raw_gyro_value[3];
	int16_t value_calibration[3][samples];

	// Read [samples] amount of values into array
	for (int i = 0; i <= samples; i++) {
		while(!(get_AG_status() & 0x02)) {}

		read_gy(raw_gyro_value);
		value_calibration[0][i] = raw_gyro_value[0];
		value_calibration[1][i] = raw_gyro_value[1];
		value_calibration[2][i] = raw_gyro_value[2];

		printf(".");
	}

	// Find average of read values.
	for (int i = 0; i <= samples; i++) {
		offset[0] += value_calibration[0][i] / samples;
		offset[1] += value_calibration[1][i] / samples;
		offset[2] += value_calibration[2][i] / samples;
	}
	printf("\nDone calibrating Gyroscope.\n");
}

void calibrate_xl(int16_t *offset){
	printf("Calibrating Acceleromter.\n Keep board steady!\n");

	offset[0] = 0; offset[1] = 0; offset[2] = 0;

	int samples = 10;
	int16_t raw_xl_value[3];
	int16_t value_calibration[3][samples];

	// Read [samples] amount of values into array
	for (int i = 0; i <= samples; i++) {
		while(!(get_AG_status() & 0x01)) {}

		read_xl(raw_xl_value);
		value_calibration[0][i] = raw_xl_value[0];
		value_calibration[1][i] = raw_xl_value[1];
		value_calibration[2][i] = raw_xl_value[2];

		printf(".");
	}

	// Find average of read values.
	for (int i = 0; i <= samples; i++) {
		offset[0] += (float_t)value_calibration[0][i] / samples;
		offset[1] += (float_t)value_calibration[1][i] / samples;
		offset[2] += (float_t)value_calibration[2][i] / samples;
	}
	printf("\nDone calibrating Accelerometer.\n");
}

