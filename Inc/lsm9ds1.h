#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include "stm32f30x.h"

#ifndef LSM9DS1_H_
#define LSM9DS1_H_


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
