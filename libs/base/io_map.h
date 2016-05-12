#ifndef SRC_IO_MAP_H_
#define SRC_IO_MAP_H_

#include <em_system.h>

#define LED_PORT gpioPortE
#define LED0 2
#define LED1 3

// LEUART0
#define LEUART0_PORT gpioPortD
#define LEUART0_TX 4
#define LEUART0_RX 5
#define LEUART0_LOCATION LEUART_ROUTE_LOCATION_LOC0
#define LEUART0_BAUDRATE 9600

// I2C
#define I2C I2C1
#define I2C_PORT gpioPortC
#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 5
#define I2C_LOCATION I2C_ROUTE_LOCATION_LOC0

#endif
