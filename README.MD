# Group 4 - Digital Instrumentation
## Assignment 4
**connections (using SPI2)**
| LSM9DS1             | STM32F302 | STM32F302 pin |
| :------------------ | :-------: | ------------: |
| GND                 | GND       | One of them   |
| VDD                 | 3v3       | 3v3           |
| SDA	              | MOSI      | D11 / PB15    |
| SCL                 | SCK       | D13 / PB13    |
| SDO M + SDO AG      | MISO      | D12 / PB14    |
| CS AG               | CS        | D10 / PB6     |
| CS M                | D6        | D6 / PB10     |