#include "stm32f30x.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "gpio.h"
#include "flash.h"
#include "lcd.h"
#include "string.h"

/* I2C1_SCL = PB8, I2C1_SDA = PB9 (AF4)
 * Devices on the bus: MMA7660FC accelerometer, M75BD (LM75-compatible) thermometer */

/* 8-bit addresses (7-bit address << 1) */
#define MMA7660_ADDR   0x98u        /* 7-bit 0x4C */
#define LM75_ADDR      0x90u        /* 7-bit 0x48, A2:A1:A0 = 000. Do NOT use 100 (0x4C) */

#define MMA_REG_XOUT   0x00u        /* X, Y, Z at 0x00..0x02 */
#define MMA_REG_MODE   0x07u
#define MMA_ALERT_BIT  0x40u        /* set = sample was mid-update */

#define LM75_REG_TEMP  0x00u        /* 16-bit, read-only, MSB first */

#define I2C1_TIMING    0x10420F13u  /* 100 kHz @ 8 MHz I2CCLK */
#define I2C_TIMEOUT    100000u

typedef struct { int32_t x, y, z; } Accel;   /* milli-g */

/* ---------- I2C (shared by both devices) ---------- */
static bool i2cWait(uint32_t flag, FlagStatus state)
{
    uint32_t timeout = I2C_TIMEOUT;

    while (I2C_GetFlagStatus(I2C1, flag) != state)
    {
        if (I2C_GetFlagStatus(I2C1, I2C_FLAG_NACKF) == SET || --timeout == 0)
        {
            I2C_Cmd(I2C1, DISABLE);     /* toggling PE clears the peripheral */
            I2C_Cmd(I2C1, ENABLE);
            return false;
        }
    }
    return true;
}

#define WAIT(flag, state)  do { if (!i2cWait(flag, state)) return false; } while (0)

void initI2C1(void)
{
    GPIO_InitTypeDef gpio;
    I2C_InitTypeDef  i2c;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_4);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_4);

    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin   = GPIO_Pin_8 | GPIO_Pin_9;
    gpio.GPIO_Mode  = GPIO_Mode_AF;
    gpio.GPIO_OType = GPIO_OType_OD;
    gpio.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &gpio);

    I2C_StructInit(&i2c);
    i2c.I2C_Timing       = I2C1_TIMING;
    i2c.I2C_AnalogFilter = I2C_AnalogFilter_Enable;
    i2c.I2C_Ack          = I2C_Ack_Enable;
    I2C_Init(I2C1, &i2c);
    I2C_Cmd(I2C1, ENABLE);
}

static bool writeReg(uint8_t addr, uint8_t reg, uint8_t value)
{
    WAIT(I2C_FLAG_BUSY, RESET);
    I2C_TransferHandling(I2C1, addr, 2,
                         I2C_AutoEnd_Mode, I2C_Generate_Start_Write);
    WAIT(I2C_FLAG_TXIS, SET);
    I2C_SendData(I2C1, reg);
    WAIT(I2C_FLAG_TXIS, SET);
    I2C_SendData(I2C1, value);
    WAIT(I2C_FLAG_STOPF, SET);
    I2C_ClearFlag(I2C1, I2C_ICR_STOPCF);
    return true;
}

static bool readRegs(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len)
{
    WAIT(I2C_FLAG_BUSY, RESET);
    I2C_TransferHandling(I2C1, addr, 1,
                         I2C_SoftEnd_Mode, I2C_Generate_Start_Write);
    WAIT(I2C_FLAG_TXIS, SET);
    I2C_SendData(I2C1, reg);
    WAIT(I2C_FLAG_TC, SET);

    I2C_TransferHandling(I2C1, addr, len,
                         I2C_AutoEnd_Mode, I2C_Generate_Start_Read);
    for (uint8_t i = 0; i < len; i++)
    {
        WAIT(I2C_FLAG_RXNE, SET);
        buf[i] = I2C_ReceiveData(I2C1);
    }
    WAIT(I2C_FLAG_STOPF, SET);
    I2C_ClearFlag(I2C1, I2C_ICR_STOPCF);
    return true;
}

/* ---------- MMA7660 accelerometer ---------- */
bool MMA7660_Init(void)
{
    return writeReg(MMA7660_ADDR, MMA_REG_MODE, 0x01);    /* active mode */
}

/* Sign-extend 6-bit two's complement */
static int32_t sext6(uint8_t raw)
{
    return ((int8_t)(raw << 2)) >> 2;
}

/* Returns false on bus error or invalid sample. */
bool MMA7660_ReadMg(Accel *a)
{
    uint8_t raw[3];

    if (!readRegs(MMA7660_ADDR, MMA_REG_XOUT, raw, 3) ||
        ((raw[0] | raw[1] | raw[2]) & MMA_ALERT_BIT))
        return false;

    a->x = sext6(raw[0]) * 375 / 8;     /* 46.875 mg per count, integer math */
    a->y = sext6(raw[1]) * 375 / 8;
    a->z = sext6(raw[2]) * 375 / 8;
    return true;
}

/* ---------- M75BD / LM75 thermometer ---------- */
/* Temperature register: 11-bit two's complement, left-justified in 16 bits.
 * Shift right 5 to get the signed count; each count = 0.125 C = 125 mC.
 * Powers up in normal (continuous) mode, so no init is needed. */
bool LM75_ReadMilliC(int32_t *milliC)
{
    uint8_t raw[2];

    if (!readRegs(LM75_ADDR, LM75_REG_TEMP, raw, 2))
        return false;

    int16_t t = (int16_t)((raw[0] << 8) | raw[1]);
    *milliC = (t >> 5) * 125;
    return true;
}

/* ---------- Application ---------- */
#define LCD_UPDATE_EVERY  10     /* 10 samples x 10 ms = 10 Hz display refresh */

void exercise_4_1_internalGyro(void)
{
    uint8_t fbuffer[512];
    char    line[22];            /* 21 chars fit on one LCD line, plus NUL */
    Accel   raw, filt = {0, 0, 0};
    bool    first = true;
    uint8_t count = 0;

    init_spi_lcd();
    initI2C1();
    MMA7660_Init();

    while (1)
    {
        if (MMA7660_ReadMg(&raw))
        {
            if (first) { filt = raw; first = false; }
            else
            {
                filt.x += (raw.x - filt.x) / 4;
                filt.y += (raw.y - filt.y) / 4;
                filt.z += (raw.z - filt.z) / 4;
            }

            if (++count >= LCD_UPDATE_EVERY)
            {
                const char    name[3] = { 'X', 'Y', 'Z' };
                const int32_t val[3]  = { filt.x, filt.y, filt.z };
                int32_t       tempMc;
                bool          tempOk = LM75_ReadMilliC(&tempMc);

                count = 0;

                lcd_clear_buffer(fbuffer);

                /* Rows 0-2: acceleration */
                for (uint8_t row = 0; row < 3; row++)
                {
                    snprintf(line, sizeof line, "%c: %5ld mg", name[row], (long)val[row]);
                    lcd_write_string((uint8_t *)line, fbuffer, 0, row);
                }

                /* Row 3: temperature */
                if (tempOk)
                    snprintf(line, sizeof line, "T: %c%ld.%03ld C",
                             tempMc < 0 ? '-' : '+',
                             labs(tempMc) / 1000, labs(tempMc) % 1000);
                else
                    snprintf(line, sizeof line, "T: no sensor");
                lcd_write_string((uint8_t *)line, fbuffer, 0, 3);

                lcd_push_buffer(fbuffer);

                printf("X: %5ld  Y: %5ld  Z: %5ld mg  T: %s%ld.%03ld C\r\n",
                       (long)filt.x, (long)filt.y, (long)filt.z,
                       tempMc < 0 ? "-" : "", labs(tempMc) / 1000, labs(tempMc) % 1000);
            }
        }
    }
}
