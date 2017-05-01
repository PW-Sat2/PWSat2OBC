#ifndef SRC_IO_MAP_H_
#define SRC_IO_MAP_H_

#include <cstdint>
#include <em_device.h>
#include <em_gpio.h>
#include <em_usart.h>
#include <em_wdog.h>
#include <em_system.h>
#include "base/io_map.h"
#include "utils.h"

/** @cond FALSE */

namespace io_map
{
    using SlaveSelectFlash1 = PinLocation<gpioPortD, 3>;
    using SlaveSelectFlash2 = PinLocation<gpioPortB, 11>;
    using SlaveSelectFlash3 = PinLocation<gpioPortB, 12>;

    using SlaveSelectFram1 = PinLocation<gpioPortA, 11>;
    using SlaveSelectFram2 = PinLocation<gpioPortA, 12>;
    using SlaveSelectFram3 = PinLocation<gpioPortA, 13>;

    using Led0 = PinLocation<gpioPortE, 2>;
    using Led1 = PinLocation<gpioPortE, 3>;

    using SysClear = PinLocation<gpioPortC, 0>;

    struct SPI : public SPIPins<SPI>
    {
        static constexpr std::uint32_t Baudrate = 7_MHz;
        static constexpr USART_TypeDef* Peripheral = USART1;
        static constexpr std::uint8_t Location = 1;
        using MOSI = PinLocation<gpioPortD, 0>;
        using MISO = PinLocation<gpioPortD, 1>;
        using CLK = PinLocation<gpioPortD, 2>;
    };

    struct LEUART : public LEUARTPins<LEUART>
    {
        static constexpr std::uint8_t Location = LEUART_ROUTE_LOCATION_LOC0;
        static constexpr std::uint32_t Baudrate = 115200;
        static constexpr std::uint8_t InterruptPriority = 6;

        using TX = PinLocation<gpioPortD, 4>;
        using RX = PinLocation<gpioPortD, 5>;
    };

    struct UART : public UARTPins<UART>
    {
        static constexpr USART_TypeDef* Peripheral = UART1;
        static constexpr std::uint32_t Location = UART_ROUTE_LOCATION_LOC2;
        static constexpr std::uint32_t Baudrate = 115200;
        static constexpr std::uint8_t InterruptPriority = 6;
        static constexpr IRQn WakeUpInterrupt = IRQn::LESENSE_IRQn;
        static constexpr std::uint8_t WakeUpInterruptPriority = 5;

        using TX = PinLocation<gpioPortB, 9>;
        using RX = PinLocation<gpioPortB, 10>;
    };

    struct I2C_0 : public I2CPins<I2C_0>
    {
        static constexpr std::uint32_t Location = I2C_ROUTE_LOCATION_LOC1;

        using SDA = PinLocation<gpioPortD, 6>;
        using SCL = PinLocation<gpioPortD, 7>;
    };

    struct I2C_1 : public I2CPins<I2C_1>
    {
        static constexpr std::uint32_t Location = I2C_ROUTE_LOCATION_LOC0;

        using SDA = PinLocation<gpioPortC, 4>;
        using SCL = PinLocation<gpioPortC, 5>;
    };

    struct I2C
    {
        static constexpr std::uint8_t InterruptPriority = 6;

        static constexpr std::uint8_t SystemBus = 1;
        static constexpr std::uint8_t PayloadBus = 0;
#ifndef I2C_TIMEOUT
        static constexpr std::uint32_t Timeout = 5; // in seconds
#else
        static constexpr std::uint32_t Timeout = I2C_TIMEOUT; // in seconds
#endif
    };

    struct Watchdog
    {
        static constexpr WDOG_PeriodSel_TypeDef Period = wdogPeriod_1k; // About 1 second
    };
}

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

/** @endcond */

#endif
