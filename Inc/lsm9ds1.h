#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include "stm32f30x.h"

#ifndef LSM9DS1_H_
#define LSM9DS1_H_


// SPI INIT, READ AND WRITE //
void init_spi_lsm9ds1(void);
uint8_t lsm9ds1_read8(uint8_t addr);
uint16_t lsm9ds1_read16(uint8_t addr);
void lsm9ds1_write(uint8_t addr, uint8_t data_in);
void lsm9ds1_write16(uint8_t addr, uint16_t data_in);   // low byte first. TODO only works for magnetometer


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

#define OFFSET_X_REG_L_M    0x05    // high byte is 0x06
#define OFFSET_Y_REG_L_M    0x07    // high byte is 0x08
#define OFFSET_Z_REG_L_M    0x09    // high byte is 0x0A
// MAGNETOMETER
// Initialization, read and convert
int      mag_init(void);                    // returns 0 if OK, -1 if not found
void     mag_read_xyz(int16_t *x, int16_t *y, int16_t *z);
float    mag_raw_to_mgauss(int16_t raw, uint8_t ctrl_reg2);
// MAGNETOMETER OFFSET CALIBRATION

void     mag_write_offsets(int16_t x, int16_t y, int16_t z);
void     mag_read_offsets(int16_t *x, int16_t *y, int16_t *z);
void     mag_calibrate(uint32_t samples);   // rotate the board by hand while this runs
/** Device Identification (Who am I) **/
#define LSM9DS1_IMU_ID             0x68U

/** Device Identification (Who am I) **/
#define LSM9DS1_MAG_ID             0x3DU

// Accelerometer and Gyro register list
#define LSM9DS1_ACT_THS            0x04U
#define LSM9DS1_ACT_DUR            0x05U
#define LSM9DS1_INT_GEN_CFG_XL     0x06U
#define LSM9DS1_INT_GEN_THS_X_XL   0x07U
#define LSM9DS1_INT_GEN_THS_Y_XL   0x08U
#define LSM9DS1_INT_GEN_THS_Z_XL   0x09U
#define LSM9DS1_INT_GEN_DUR_XL     0x0AU
#define LSM9DS1_REFERENCE_G        0x0BU
#define LSM9DS1_INT1_CTRL          0x0CU
#define LSM9DS1_INT2_CTRL          0x0DU
#define LSM9DS1_CTRL_REG1_G        0x10U
#define LSM9DS1_WHO_AM_I           0x0FU
#define LSM9DS1_CTRL_REG1_G        0x10U
#define LSM9DS1_CTRL_REG2_G        0x11U
#define LSM9DS1_CTRL_REG3_G        0x12U
#define LSM9DS1_ORIENT_CFG_G       0x13U
#define LSM9DS1_INT_GEN_SRC_G      0x14U
#define LSM9DS1_OUT_TEMP_L         0x15U
#define LSM9DS1_OUT_TEMP_H         0x16U
#define LSM9DS1_STATUS_REG         0x17U
#define LSM9DS1_OUT_X_L_G          0x18U
#define LSM9DS1_OUT_X_H_G          0x19U
#define LSM9DS1_OUT_Y_L_G          0x1AU
#define LSM9DS1_OUT_Y_H_G          0x1BU
#define LSM9DS1_OUT_Z_L_G          0x1CU
#define LSM9DS1_OUT_Z_H_G          0x1DU
#define LSM9DS1_CTRL_REG4          0x1EU
#define LSM9DS1_CTRL_REG5_XL       0x1FU
#define LSM9DS1_CTRL_REG6_XL       0x20U
#define LSM9DS1_CTRL_REG7_XL       0x21U
#define LSM9DS1_CTRL_REG8          0x22U
#define LSM9DS1_CTRL_REG9          0x23U
#define LSM9DS1_CTRL_REG10         0x24U
#define LSM9DS1_INT_GEN_SRC_XL     0x26U
#define LSM9DS1_OUT_X_L_XL         0x28U
#define LSM9DS1_OUT_X_H_XL         0x29U
#define LSM9DS1_OUT_Y_L_XL         0x2AU
#define LSM9DS1_OUT_Y_H_XL         0x2BU
#define LSM9DS1_OUT_Z_L_XL         0x2CU
#define LSM9DS1_OUT_Z_H_XL         0x2DU
#define LSM9DS1_FIFO_CTRL          0x2EU
#define LSM9DS1_FIFO_SRC           0x2FU
#define LSM9DS1_INT_GEN_CFG_G      0x30U
#define LSM9DS1_INT_GEN_THS_XH_G   0x31U
#define LSM9DS1_INT_GEN_THS_XL_G   0x32U
#define LSM9DS1_INT_GEN_THS_YH_G   0x33U
#define LSM9DS1_INT_GEN_THS_YL_G   0x34U
#define LSM9DS1_INT_GEN_THS_ZH_G   0x35U
#define LSM9DS1_INT_GEN_THS_ZL_G   0x36U
#define LSM9DS1_INT_GEN_DUR_G      0x37U


// Magnetometer register list
#define LSM9DS1_OFFSET_X_REG_L_M   0x05U
#define LSM9DS1_OFFSET_X_REG_H_M   0x06U
#define LSM9DS1_OFFSET_Y_REG_L_M   0x07U
#define LSM9DS1_OFFSET_Y_REG_H_M   0x08U
#define LSM9DS1_OFFSET_Z_REG_L_M   0x09U
#define LSM9DS1_OFFSET_Z_REG_H_M   0x0AU
#define LSM9DS1_WHO_AM_I_M         0x0FU
#define LSM9DS1_CTRL_REG1_M        0x20U
#define LSM9DS1_CTRL_REG2_M        0x21U
#define LSM9DS1_CTRL_REG3_M        0x22U
#define LSM9DS1_CTRL_REG4_M        0x23U
#define LSM9DS1_CTRL_REG5_M        0x24U
#define LSM9DS1_STATUS_REG_M       0x27U
#define LSM9DS1_OUT_X_L_M          0x28U
#define LSM9DS1_OUT_X_H_M          0x29U
#define LSM9DS1_OUT_Y_L_M          0x2AU
#define LSM9DS1_OUT_Y_H_M          0x2BU
#define LSM9DS1_OUT_Z_L_M          0x2CU
#define LSM9DS1_OUT_Z_H_M          0x2DU
#define LSM9DS1_INT_CFG_M          0x30U
#define LSM9DS1_INT_SRC_M          0x31U
#define LSM9DS1_INT_THS_L_M        0x32U
#define LSM9DS1_INT_THS_H_M        0x33U

// Public functions
void init_spi_lsm9ds1(void);
int init_AG(void);
int16_t read_temp(void);
void read_gy(int16_t *value);
void read_xl(int16_t *value);
void calibrate_gy(int16_t *offset);
void calibrate_xl(int16_t *offset);

// Helper converter functions
float_t temp_raw_to_float(int16_t temp_raw);
float_t fs2000dps_to_mdps(int16_t gy_raw);
float_t fs4g_to_mg(int16_t xl_raw);

#endif /* LSM9DS1_H_ */
