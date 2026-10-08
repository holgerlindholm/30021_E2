#include <stdio.h>
#include "stm32f30x.h"

#ifndef LSM9DS1_H_
#define LSM9DS1_H_

#define WHO_AM_I 0x0F
#define CTRL_REG1 0x20
#define CTRL_REG2 0x21
#define CTRL_REG3 0x22
#define CTRL_REG4 0x23
#define CTRL_REG5 0x24
#define REFERENCE 0x25
#define OUT_TEMP 0x26
#define STATUS_REG 0x27
#define OUT_X_L 0x28
#define OUT_X_H 0x29
#define OUT_Y_L 0x2A
#define OUT_Y_H 0x2B
#define OUT_Z_L 0x2C
#define OUT_Z_H 0x2D
#define FIFO_CTRL_REG 0x2E
#define FIFO_SRC_REG 0x2F
#define INT1_CFG 0x30
#define INT1_SRC 0x31
#define INT1_TSH_XH 0x32
#define INT1_TSH_XL 0x33
#define INT1_TSH_YH 0x34
#define INT1_TSH_YL 0x35
#define INT1_TSH_ZH 0x36
#define INT1_TSH_ZL 0x37
#define INT1_DURATION 0x38

// SPI INIT, READ AND WRITE //
void init_spi_lsm9ds1(void);
uint8_t lsm9ds1_read8(uint8_t addr);
uint16_t lsm9ds1_read16(uint8_t addr);
void lsm9ds1_write(uint8_t addr, uint8_t data_in);

// MAGNETOMETER REGISTER ADRESSES
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

// FUNCTIONS SPECIFIC TO MAGNETOMETER
int      mag_init(void);                    // returns 0 if OK, -1 if not found
void     mag_read_xyz(int16_t *x, int16_t *y, int16_t *z);
float    mag_raw_to_mgauss(int16_t raw, uint8_t ctrl_reg2);


#endif /* LSM9DS1_H_ */
