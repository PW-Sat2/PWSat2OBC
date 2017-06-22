#ifndef SRC_IO_MAP_H_
#define SRC_IO_MAP_H_

#include <cstdint>
#include <em_device.h>
#include <em_gpio.h>
#include <em_timer.h>
#include <em_usart.h>
#include <em_wdog.h>
#include <em_system.h>
#include "base/io_map.h"
#include "utils.h"

/** @cond FALSE */

namespace io_map
{
    using SlaveSelectFlash1 = PinLocation<gpioPortA, 8>;
    using SlaveSelectFlash2 = PinLocation<gpioPortA, 9>;
    using SlaveSelectFlash3 = PinLocation<gpioPortA, 10>;

    using SlaveSelectFram1 = PinLocation<gpioPortA, 11>;
    using SlaveSelectFram2 = PinLocation<gpioPortA, 12>;
    using SlaveSelectFram3 = PinLocation<gpioPortA, 13>;

    using Led0 = PinLocation<gpioPortD, 1>;
    using Led1 = PinLocation<gpioPortD, 2>;

    using SysClear = PinLocation<gpioPortC, 0>;

    struct SPI : public SPIPins<SPI>
    {
        static constexpr std::uint32_t Baudrate = 20_MHz;
        static constexpr USART_TypeDef* Peripheral = USART0;
        static constexpr std::uint8_t Location = 1;
        using MOSI = PinLocation<gpioPortE, 7>;
        using MISO = PinLocation<gpioPortE, 6>;
        using CLK = PinLocation<gpioPortE, 5>;
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
        static constexpr std::uint8_t InterruptPriority = 0;
        static constexpr IRQn WakeUpInterrupt = IRQn::LESENSE_IRQn;
        static constexpr std::uint8_t WakeUpInterruptPriority = 5;

        using TX = PinLocation<gpioPortB, 9>;
        using RX = PinLocation<gpioPortB, 10>;
    };

    struct I2C_0 : public I2CPins<I2C_0>
    {
        static constexpr std::uint32_t Location = I2C_ROUTE_LOCATION_LOC2;

        using SDA = PinLocation<gpioPortC, 6>;
        using SCL = PinLocation<gpioPortC, 7>;
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
        static constexpr std::uint8_t SystemBus = 0;
        static constexpr std::uint8_t PayloadBus = 1;
#ifndef I2C_TIMEOUT
        static constexpr std::uint32_t Timeout = 5; // in seconds
#else
        static constexpr std::uint32_t Timeout = I2C_TIMEOUT; // in seconds
#endif
    };

    struct Watchdog
    {
        static constexpr WDOG_PeriodSel_TypeDef Period = wdogPeriod_64k;      // About 1 minute
        static constexpr WDOG_PeriodSel_TypeDef BootTimeout = wdogPeriod_16k; // About 16 second
        using ExternalWatchdogPin = PinLocation<gpioPortF, 9>;
    };

    struct ProgramFlash
    {
        static constexpr std::uint8_t* FlashBase = reinterpret_cast<std::uint8_t*>(0x84000000);
    };

    struct RAMScrubbing
    {
        static constexpr auto TimerHW = TIMER0;
        static constexpr auto Prescaler = timerPrescale1024;
        static constexpr auto TimerTop = 468;
        static constexpr auto IRQ = IRQn_Type::TIMER0_IRQn;
        static constexpr std::size_t MemoryStart = 0x88000000;
        static constexpr std::size_t MemorySize = 1_MB;
        static constexpr std::size_t CycleSize = 8;
    };

    struct BSP : public PinGroupTag
    {
        struct SRAMPower : public PinGroupTag
        {
            using SRAM1 = PinLocation<gpioPortC, 0>;
            using SRAM2 = PinLocation<gpioPortC, 1>;

            struct Group
            {
                using Pins = PinContainer<SRAM1, SRAM2>;
            };
        };

        struct SRAMBuffer : public PinGroupTag
        {
            using SRAM1 = PinLocation<gpioPortC, 14>;
            using SRAM2 = PinLocation<gpioPortC, 15>;

            struct Group
            {
                using Pins = PinContainer<SRAM1, SRAM2>;
            };
        };

        struct EDAC : public PinGroupTag
        {
            using Control1 = PinLocation<gpioPortE, 2>;
            using Control2 = PinLocation<gpioPortE, 3>;

            using ErrorPins = PortPins<gpioPortB, 0, 2>;

            struct Group
            {
                using Pins = PinContainer<Control1, Control2, ErrorPins>;
            };
        };

        struct EBIConfig : public PinGroupTag
        {
            using Address0__7 = PortPins<gpioPortE, 8, 9, 10, 11, 12, 13, 14, 15>;
            using Address9_15 = PortPins<gpioPortA, 0, 1, 2, 3, 4, 5, 6, 15>;
            using ChipSelects = PortPins<gpioPortD, 9, 10, 11, 12>;
            using ControlPins = PortPins<gpioPortF, 3, 4, 5>;

            struct Group
            {
                using Pins = PinContainer<Address0__7, Address9_15, ChipSelects, ControlPins>;
            };
        };

        struct Group
        {
            using Pins = PinContainer<SRAMBuffer, EDAC, EBIConfig>;
        };
    };
}

/** @endcond */

#endif
