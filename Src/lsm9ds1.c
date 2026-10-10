#include "lsm9ds1.h"
#include "lsm9ds1_reg.h"

// Initialize private functions.
static uint8_t get_AG_status(void);
static uint8_t M_read8(uint8_t addr);
static uint8_t AG_read8(uint8_t addr);
static void M_write(uint8_t addr, uint8_t data_in);
static void AG_write(uint8_t addr, uint8_t data_in);

float_t temp_raw_to_float(int16_t temp_raw) {
	return 25.0f + (temp_raw / 16.0f);
}

// SPI always sends and receives at the same time.
// Send one byte, return the byte that came in during that transfer.
static uint8_t spi2_xfer(uint8_t tx)
{
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) != SET) {}   // wait until TX buffer is empty
    SPI_SendData8(SPI2, tx);                                          // start transfer
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) != SET) {} // wait until a byte has arrived
    return SPI_ReceiveData8(SPI2);                                    // read it (also clears the flag)
}

// Read one register
uint8_t lsm9ds1_read8(uint8_t addr)
{
    uint8_t data;

    GPIOB->ODR &= ~(1 << 6);            // CS low = start transaction
    spi2_xfer(addr | 0x80);             // bit 7 = 1 means "read" (reply byte is junk, discard it)
    data = spi2_xfer(0x00);             // send dummy byte so the sensor can clock the data out
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET) {}   // wait until SPI is fully done
    GPIOB->ODR |= (1 << 6);             // CS high = end transaction

    return data;
}

// Read two consecutive registers (low byte first, then high byte)
// For the magnetometer, OR 0x40 into addr so the second byte comes from addr+1
uint16_t lsm9ds1_read16(uint8_t addr)
{
    uint8_t lsb, msb;

    GPIOB->ODR &= ~(1 << 6);            // CS low
    spi2_xfer(addr | 0x80);             // read command
    lsb = spi2_xfer(0x00);              // 1st byte = low register
    msb = spi2_xfer(0x00);              // 2nd byte = high register
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET) {}
    GPIOB->ODR |= (1 << 6);             // CS high

    return ((uint16_t)msb << 8) | lsb;  // combine into 16 bits
}

// Write one register
void lsm9ds1_write(uint8_t addr, uint8_t data_in)
{
    GPIOB->ODR &= ~(1 << 6);            // CS low
    spi2_xfer(addr & 0x7F);             // bit 7 = 0 means "write"
    spi2_xfer(data_in);                 // value to write
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET) {}
    GPIOB->ODR |= (1 << 6);             // CS high
}

// Write two consecutive registers in one transaction (low byte first, then high byte)
// Write 2nd byte to addr+1
void lsm9ds1_write16(uint8_t addr, uint16_t data_in)
{
	GPIOB->ODR &= ~(1 << 6);            // CS low
    spi2_xfer((addr & 0x3F) | MAG_AUTO_INC);            // bit 7 = 0 (write), bit 6 = 1 (auto-increment)
    spi2_xfer(data_in & 0xFF);                          // low byte  -> addr
    spi2_xfer((data_in >> 8) & 0xFF);                   // high byte -> addr + 1
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET) {}
    GPIOB->ODR |= (1 << 6);             // CS high
   }

// Returns 0 if OK, -1 if the magnetometer isn't found
// We can change settings using the CTRL registers depending on how we want it to operate
int mag_init(void)
{
    lsm9ds1_write(CTRL_REG3_M, 0x04);       // SIM=1 (allow SPI reads), MD=00 (continuous conversion)
    if (lsm9ds1_read8(WHO_AM_I_M) != 0x3D) return -1;   // check chip ID

    // Configure the CTRL registers
    lsm9ds1_write(CTRL_REG1_M, 0xFC);       // temp comp on, ultra-high performance XY, 80 Hz
    lsm9ds1_write(CTRL_REG2_M, 0x00);       // +/-4 gauss
    lsm9ds1_write(CTRL_REG4_M, 0x0C);       // ultra-high performance Z
    lsm9ds1_write(CTRL_REG5_M, 0x40);       // BDU on
    return 0;
}

// Raw readings (signed 16-bit) using pointers
void mag_read_xyz(int16_t *x, int16_t *y, int16_t *z)
{
    while (!(lsm9ds1_read8(STATUS_REG_M) & 0x08)) {}   // wait until new XYZ data is ready

    // Read each registers and store it to memory location pointed to by *x, *y and *z
    *x = (int16_t)lsm9ds1_read16(OUT_X_L_M | MAG_AUTO_INC);   // reads 0x28 + 0x29
    *y = (int16_t)lsm9ds1_read16(OUT_Y_L_M | MAG_AUTO_INC);   // reads 0x2A + 0x2B
    *z = (int16_t)lsm9ds1_read16(OUT_Z_L_M | MAG_AUTO_INC);   // reads 0x2C + 0x2D
}

// Convert a raw magnetometer reading to milligauss depening on scale
// scale is located in FS[1:0] in CTRL_REG2_M (bit value 5 and 6)
float mag_raw_to_mgauss(int16_t raw, uint8_t ctrl_reg2)
{
	// move bits 5 times to the right and only read last two significant bits
    switch ((ctrl_reg2 >> 5) & 0x03) {   // FS[1:0] = bits 6:5
    	// Sensititity values found in Section 2.1 Sensor characteristics
        case 0:  return raw * 0.14f;     // +/-4 gauss
        case 1:  return raw * 0.29f;     // +/-8 gauss
        case 2:  return raw * 0.43f;     // +/-12 gauss
        default: return raw * 0.58f;     // +/-16 gauss
    }
}

// Write the offsets into the sensor. It subtracts them from every reading.
void mag_write_offsets(int16_t x, int16_t y, int16_t z)
{
    lsm9ds1_write16(OFFSET_X_REG_L_M, x);   // 0x05 + 0x06
    lsm9ds1_write16(OFFSET_Y_REG_L_M, y);   // 0x07 + 0x08
    lsm9ds1_write16(OFFSET_Z_REG_L_M, z);   // 0x09 + 0x0A
}

// Read the offsets currently stored in the sensor
void mag_read_offsets(int16_t *x, int16_t *y, int16_t *z)
{
    *x = (int16_t)lsm9ds1_read16(OFFSET_X_REG_L_M | MAG_AUTO_INC);   // 0x05 + 0x06
    *y = (int16_t)lsm9ds1_read16(OFFSET_Y_REG_L_M | MAG_AUTO_INC);   // 0x07 + 0x08
    *z = (int16_t)lsm9ds1_read16(OFFSET_Z_REG_L_M | MAG_AUTO_INC);   // 0x09 + 0x0A
float_t fs2000dps_to_mdps(int16_t gy_raw)
{
  return ((float_t)gy_raw * 70.0f);
}

float_t fs4g_to_mg(int16_t xl_raw)
{
  return ((float_t)xl_raw * 0.122f);
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

uint8_t M_read8(uint8_t addr) {
	uint8_t data_out8;

	GPIOB->ODR &= ~(1 << 10);

	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) != SET) {}
	SPI_SendData8(SPI2, addr | 0x80);

	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) != SET) {}
	SPI_ReceiveData8(SPI2);

	SPI_SendData8(SPI2, 0x00);

	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) != SET) {}
	data_out8 = SPI_ReceiveData8(SPI2);

	GPIOB->ODR |= (1 << 10);

	return data_out8;
}

uint8_t AG_read8(uint8_t addr) {
	uint8_t data_out8;

    GPIOB->ODR &= ~(1 << 6);

    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) != SET) {}
	SPI_SendData8(SPI2, addr | 0x80);

	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) != SET) {}
	SPI_ReceiveData8(SPI2);

	SPI_SendData8(SPI2, 0x00);

	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) != SET) {}
	data_out8 = SPI_ReceiveData8(SPI2);

	GPIOB->ODR |= (1 << 6);

	return data_out8;
}

void M_write(uint8_t addr, uint8_t data_in) {
	GPIOB->ODR &= ~(1 << 10);

	SPI_SendData8(SPI2, addr);

	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) != SET) {}
	SPI_ReceiveData8(SPI2);

	SPI_SendData8(SPI2, data_in);
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) != SET) {}
	SPI_ReceiveData8(SPI2);

	GPIOB->ODR |= (1 << 10);
}

void AG_write(uint8_t addr, uint8_t data_in) {
	GPIOB->ODR &= ~(1 << 6);

	SPI_SendData8(SPI2, addr);

	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) != SET) {}
	SPI_ReceiveData8(SPI2);

	SPI_SendData8(SPI2, data_in);
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) != SET) {}
	SPI_ReceiveData8(SPI2);

	GPIOB->ODR |= (1 << 6);
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
	if (AG_read8(LSM9DS1_WHO_AM_I) != LSM9DS1_IMU_ID) return -1;

	// Set Gyroscope, scale = 2000 dps, filter bandwith = ultra light, output data rate = 59.5Hz
	AG_write(LSM9DS1_CTRL_REG1_G, 0x5A);

	// Set gyroscope out selection
	AG_write(LSM9DS1_CTRL_REG2_G, 0x02);

	// Set Gyroscope Enable, HP filter = Medium, Disable low power mode.
	AG_write(LSM9DS1_CTRL_REG3_G, 0x45);

	// Set Accelerometer scale = 4g, bandwidth anti-alias = auto, output data rate = 50Hz
	AG_write(LSM9DS1_CTRL_REG6_XL, 0x50);

	//Set Accelerometer LP filter out, LP Bandwith = ODR/50
	AG_write(LSM9DS1_CTRL_REG7_XL, 0x80);

	// Read measurement from temperature, gyroscope and accelerometer, to discard initial value.
	int16_t temp_buff[3];
	read_temp();
	read_gy(temp_buff);
	read_xl(temp_buff);

	return 0;
}

uint8_t get_AG_status(void) {
	return AG_read8(LSM9DS1_STATUS_REG);
}

int16_t read_temp(void) {
	// Wait for new temperature value to be ready.
	while(!(get_AG_status() & 0x04)) {}

	return ((int16_t)AG_read8(LSM9DS1_OUT_TEMP_H) << 8) | (int16_t)AG_read8(LSM9DS1_OUT_TEMP_L);
}


void read_gy(int16_t *value) {
	// Wait for new gyroscope value to be ready.
	while(!(get_AG_status() & 0x02)) {}

	value[0] = ((int16_t)AG_read8(LSM9DS1_OUT_X_H_G) << 8) | (int16_t)AG_read8(LSM9DS1_OUT_X_L_G);
	value[1] = ((int16_t)AG_read8(LSM9DS1_OUT_Y_H_G) << 8) | (int16_t)AG_read8(LSM9DS1_OUT_Y_L_G);
	value[2] = ((int16_t)AG_read8(LSM9DS1_OUT_Z_H_G) << 8) | (int16_t)AG_read8(LSM9DS1_OUT_Z_L_G);

//	printf("Gyroscope raw readings:\tX = %X\t Y = %X\t Z = %X\n",value[0],value[1],value[2]);
}

void read_xl(int16_t *value) {
	// Wait for new acceleromter value to be ready.
	while(!(get_AG_status() & 0x01)) {}

	value[0] = ((int16_t)AG_read8(LSM9DS1_OUT_X_H_XL) << 8) | (int16_t)AG_read8(LSM9DS1_OUT_X_L_XL);
	value[1] = ((int16_t)AG_read8(LSM9DS1_OUT_Y_H_XL) << 8) | (int16_t)AG_read8(LSM9DS1_OUT_Y_L_XL);
	value[2] = ((int16_t)AG_read8(LSM9DS1_OUT_Z_H_XL) << 8) | (int16_t)AG_read8(LSM9DS1_OUT_Z_L_XL);

//	printf("Accelerometer raw readings:\tX = %X\t Y = %X\t Z = %X\n",value[0],value[1],value[2]);
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

// Function to find minimin and maximim magnetometer value whilst rotating the sensor
// It is expected that the center between the min and max values when rotating the sensor should be zero
void mag_calibrate(uint32_t samples)
{
    int16_t min[3] = { 32767,  32767,  32767};
    int16_t max[3] = {-32768, -32768, -32768};
    int16_t v[3];
    int16_t off[3];
    int i;
    printf("Rotate in all directions \n");
    mag_write_offsets(0, 0, 0);                 // clear old offsets so we see uncorrected data
    mag_read_xyz(&v[0], &v[1], &v[2]);          // discard one sample that may still have the old offset

    for (uint32_t n = 0; n < samples; n++) {
        mag_read_xyz(&v[0], &v[1], &v[2]);
        for (i = 0; i < 3; i++) {
            if (v[i] < min[i]) min[i] = v[i];
            if (v[i] > max[i]) max[i] = v[i];
        }
    }

    for (i = 0; i < 3; i++) {
        off[i] = (int16_t)(((int32_t)max[i] + min[i]) / 2);   // center of the range
    }
    printf("Done calibrating\n");
    mag_write_offsets(off[0], off[1], off[2]);
}

