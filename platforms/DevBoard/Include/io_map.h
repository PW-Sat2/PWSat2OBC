#ifndef SRC_IO_MAP_H_
#define SRC_IO_MAP_H_

#include <cstdint>
#include <em_gpio.h>
#include <em_system.h>

namespace io_map
{
    template <GPIO_Port_TypeDef Port, std::uint16_t PinNumber> struct PinLocation
    {
        static constexpr auto UsePort = Port;
        static constexpr auto UsePinNumber = PinNumber;
    };

    using SlaveSelectFlash1 = PinLocation<gpioPortD, 3>;
}

#define SPI_USART USART1
#define SPI_LOCATION 1
#define SPI_PORT gpioPortD
#define SPI_MOSI 0
#define SPI_MISO 1
#define SPI_CLK 2

#define SYS_CLEAR_PORT gpioPortC
#define SYS_CLEAR_PIN 0

#define LED_PORT gpioPortE
#define LED0 2
#define LED1 3

// LEUART0
#define LEUART0_PORT gpioPortD
#define LEUART0_TX 4
#define LEUART0_RX 5
#define LEUART0_LOCATION LEUART_ROUTE_LOCATION_LOC0
#define LEUART0_BAUDRATE 115200
#define LEUART0_INT_PRIORITY 6

// I2C
#ifndef I2C_TIMEOUT
#define I2C_TIMEOUT 5 // in seconds
#endif

// I2C0
#define I2C0_BUS_PORT gpioPortD
#define I2C0_BUS_SDA_PIN 6
#define I2C0_BUS_SCL_PIN 7
#define I2C0_BUS_LOCATION I2C_ROUTE_LOCATION_LOC1

// I2C1
#define I2C1_BUS_PORT gpioPortC
#define I2C1_BUS_SDA_PIN 4
#define I2C1_BUS_SCL_PIN 5
#define I2C1_BUS_LOCATION I2C_ROUTE_LOCATION_LOC0

#define I2C_IRQ_PRIORITY 6

#ifdef I2C_SINGLE_BUS
#define I2C_SYSTEM_BUS 1
#define I2C_PAYLOAD_BUS 1
#else
#define I2C_SYSTEM_BUS 1
#define I2C_PAYLOAD_BUS 0
#endif

// NAND Flash
#define NAND_POWER_PORT gpioPortB
#define NAND_POWER_PIN 15
#define NAND_READY_PORT gpioPortD
#define NAND_READY_PIN 15
#define NAND_CE_PORT gpioPortD
#define NAND_CE_PIN 14
#define NAND_WP_PORT gpioPortD
#define NAND_WP_PIN 13
#define NAND_ALE_BIT 24
#define NAND_CLE_BIT 25

#define EBI_ALE_PORT gpioPortC
#define EBI_ALE_PIN 1
#define EBI_CLE_PORT gpioPortC
#define EBI_CLE_PIN 2
#define EBI_WE_PORT gpioPortF
#define EBI_WE_PIN 8
#define EBI_RE_PORT gpioPortF
#define EBI_RE_PIN 9

#define EBI_DATA_PORT gpioPortE
#define EBI_DATA_PIN0 8

#endif
