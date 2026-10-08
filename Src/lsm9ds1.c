#include "lsm9ds1.h"

void init_spi_lsm9ds1(void) {
    // Enable Clocks
    RCC->AHBENR  |= 0x00020000 | 0x00040000;    // Enable Clock for GPIO Banks A and B
    RCC->APB1ENR |= 0x00004000;                 // Enable Clock for SPI2

    // Connect pins to SPI2
    GPIOB->AFR[13 >> 0x03] &= ~(0x0000000F << ((13 & 0x00000007) * 4)); // Clear alternate function for PB13
    GPIOB->AFR[13 >> 0x03] |=  (0x00000005 << ((13 & 0x00000007) * 4)); // Set alternate 5 function for PB13 - SCLK
    GPIOB->AFR[15 >> 0x03] &= ~(0x0000000F << ((15 & 0x00000007) * 4)); // Clear alternate function for PB15
    GPIOB->AFR[15 >> 0x03] |=  (0x00000005 << ((15 & 0x00000007) * 4)); // Set alternate 5 function for PB15 - MOSI

    // Configure pins PB13 and PB15 for 10 MHz alternate function
    GPIOB->OSPEEDR &= ~(0x00000003 << (13 * 2) | 0x00000003 << (15 * 2));    // Clear speed register
    GPIOB->OSPEEDR |=  (0x00000001 << (13 * 2) | 0x00000001 << (15 * 2));    // set speed register (0x01 - 10 MHz, 0x02 - 2 MHz, 0x03 - 50 MHz)
    GPIOB->OTYPER  &= ~(0x0001     << (13)     | 0x0001     << (15));        // Clear output type register
    GPIOB->OTYPER  |=  (0x0000     << (13)     | 0x0000     << (15));        // Set output type register (0x00 - Push pull, 0x01 - Open drain)
    GPIOB->MODER   &= ~(0x00000003 << (13 * 2) | 0x00000003 << (15 * 2));    // Clear mode register
    GPIOB->MODER   |=  (0x00000002 << (13 * 2) | 0x00000002 << (15 * 2));    // Set mode register (0x00 - Input, 0x01 - Output, 0x02 - Alternate Function, 0x03 - Analog in/out)
    GPIOB->PUPDR   &= ~(0x00000003 << (13 * 2) | 0x00000003 << (15 * 2));    // Clear push/pull register
    GPIOB->PUPDR   |=  (0x00000000 << (13 * 2) | 0x00000000 << (15 * 2));    // Set push/pull register (0x00 - No pull, 0x01 - Pull-up, 0x02 - Pull-down)

    // Initialize REEST, nCS, and A0
    // PB14 = SPI2 MISO
    GPIOB->AFR[14 >> 3] &= ~(0xF << ((14 & 7) * 4));
    GPIOB->AFR[14 >> 3] |=  (0x5 << ((14 & 7) * 4));

    GPIOB->MODER &= ~(0x3 << (14 * 2));
    GPIOB->MODER |=  (0x2 << (14 * 2));       // Alternate function

    GPIOB->PUPDR &= ~(0x3 << (14 * 2));
    // optionally:
    // GPIOB->PUPDR |=  (0x1 << (14 * 2));    // pull-up, if appropriate
    GPIOB->MODER &= ~(0x3 << (6 * 2));
    GPIOB->MODER |=  (0x1 << (6 * 2));    // PB6 = GPIO output

    GPIOB->OTYPER &= ~(1 << 6);            // push-pull
    GPIOB->OSPEEDR &= ~(0x3 << (6 * 2));
    GPIOB->OSPEEDR |=  (0x1 << (6 * 2));
    GPIOB->PUPDR &= ~(0x3 << (6 * 2));

    GPIOB->ODR |= (1 << 6);                // CS high

    // Configure pin PA8 for 10 MHz output
    GPIOA->OSPEEDR &= ~0x00000003 << (8 * 2);    // Clear speed register
    GPIOA->OSPEEDR |=  0x00000001 << (8 * 2);    // set speed register (0x01 - 10 MHz, 0x02 - 2 MHz, 0x03 - 50 MHz)
    GPIOA->OTYPER  &= ~0x0001     << (8);        // Clear output type register
    GPIOA->OTYPER  |=  0x0000     << (8);        // Set output type register (0x00 - Push pull, 0x01 - Open drain)


    GPIOA->MODER   &= ~0x00000003 << (8 * 2);    // Clear mode register
    GPIOA->MODER   |=  0x00000001 << (8 * 2);    // Set mode register (0x00 - Input, 0x01 - Output, 0x02 - Alternate Function, 0x03 - Analog in/out)

    GPIOA->MODER   &= ~(0x00000003 << (2 * 2) | 0x00000003 << (3 * 2));    // This is needed for UART to work. It makes no sense.
    GPIOA->MODER   |=  (0x00000002 << (2 * 2) | 0x00000002 << (3 * 2));

    GPIOA->PUPDR   &= ~0x00000003 << (8 * 2);    // Clear push/pull register
    GPIOA->PUPDR   |=  0x00000000 << (8 * 2);    // Set push/pull register (0x00 - No pull, 0x01 - Pull-up, 0x02 - Pull-down)

    GPIOB->ODR |=  (0x0001 << 6); // CS = 1

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
    msb = spi2_xfer(0x00);              // 2nd byte = high register (needs auto-increment)
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

/* ---------- Magnetometer ---------- */
// Register addresses: LSM9DS1 datasheet, magnetometer register map
// Table 22. Magnetic sensor register address map

#define MAG_AUTO_INC    0x40    // bit 6 of the SPI address byte: read next register automatically

#define WHO_AM_I_M      0x0F    // should read 0x3D for magnetometer
#define CTRL_REG1_M     0x20    // temp comp, performance mode XY, data rate
#define CTRL_REG2_M     0x21    // full-scale range
#define CTRL_REG3_M     0x22    // SPI mode, conversion mode
#define CTRL_REG4_M     0x23    // performance mode Z
#define CTRL_REG5_M     0x24    // block data update
#define STATUS_REG_M    0x27    // bit 3 = new XYZ data ready

#define OUT_X_L_M       0x28    // high byte is 0x29
#define OUT_Y_L_M       0x2A    // high byte is 0x2B
#define OUT_Z_L_M       0x2C    // high byte is 0x2D

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

// Raw readings (signed 16-bit)
void mag_read_xyz(int16_t *x, int16_t *y, int16_t *z)
{
    while (!(lsm9ds1_read8(STATUS_REG_M) & 0x08)) {}   // wait until new XYZ data is ready

    *x = (int16_t)lsm9ds1_read16(OUT_X_L_M | MAG_AUTO_INC);   // reads 0x28 + 0x29
    *y = (int16_t)lsm9ds1_read16(OUT_Y_L_M | MAG_AUTO_INC);   // reads 0x2A + 0x2B
    *z = (int16_t)lsm9ds1_read16(OUT_Z_L_M | MAG_AUTO_INC);   // reads 0x2C + 0x2D
}
