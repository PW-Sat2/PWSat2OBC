#ifndef SRC_IO_MAP_H_
#define SRC_IO_MAP_H_

#include <cstdint>
#include <em_cmu.h>
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
    using PiggyBack21 = PinLocation<gpioPortA, 13>;
    using PiggyBack22 = PinLocation<gpioPortB, 12>;

    using SystickIndicator = PinLocation<gpioPortD, 1>;
    using BootIndicator = PinLocation<gpioPortD, 2>;

    using SailDeployed = PinLocation<gpioPortD, 4>;

    using CamSelect = PiggyBack22;

    using SunSInterrupt = PinLocation<gpioPortD, 5>;

    struct SPI : public SPIPins<SPI>
    {
        static constexpr std::uint32_t Baudrate = 20_MHz;
        static constexpr USART_TypeDef* Peripheral = USART0;
        static constexpr std::uint8_t Location = 1;
        using MOSI = PinLocation<gpioPortE, 7>;
        using MISO = PinLocation<gpioPortE, 6>;
        using CLK = PinLocation<gpioPortE, 5>;

        static constexpr auto DMATransferTimeout = std::chrono::seconds(1);
    };

    struct UART_0 : public UARTPins<UART_0>
    {
        static constexpr std::uint8_t Id = 0;
        static constexpr USART_TypeDef* Peripheral = UART0;

        static constexpr CMU_Clock_TypeDef Clock = cmuClock_UART0;
        static constexpr std::uint32_t Baudrate = 115200;
        static constexpr IRQn WakeUpInterrupt = IRQn::LESENSE_IRQn;
        static constexpr std::uint8_t WakeUpInterruptPriority = 5;

        static constexpr IRQn Interrupt = IRQn::UART0_RX_IRQn;
        static constexpr std::uint8_t InterruptPriority = 0;

        static constexpr std::uint32_t Location = UART_ROUTE_LOCATION_LOC0;
        using TX = PinLocation<gpioPortF, 6>;
        using RX = PinLocation<gpioPortF, 7>;
    };

    struct UART_1 : public UARTPins<UART_1>
    {
        static constexpr std::uint8_t Id = 1;
        static constexpr USART_TypeDef* Peripheral = UART1;

        static constexpr CMU_Clock_TypeDef Clock = cmuClock_UART1;
        static constexpr std::uint32_t Baudrate = 115200;
        static constexpr IRQn WakeUpInterrupt = IRQn::LESENSE_IRQn;
        static constexpr std::uint8_t WakeUpInterruptPriority = 5;

        static constexpr IRQn Interrupt = IRQn::UART1_RX_IRQn;
        static constexpr std::uint8_t InterruptPriority = 0;

        static constexpr std::uint32_t Location = UART_ROUTE_LOCATION_LOC2;
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
        static constexpr std::uint8_t* ApplicatonBase = reinterpret_cast<std::uint8_t*>(0x00080000);
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
        struct EDAC : public PinGroupTag
        {
            using ErrorPins = PortPins<gpioPortB, 0, 2>;

            struct Group
            {
                using Pins = PinContainer<ErrorPins>;
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

        struct Latchup
        {
            static constexpr auto HW = ACMP0;
            using SRAM1 = PinLocation<gpioPortC, 2>;
            using SRAM2 = PinLocation<gpioPortC, 8>;

            struct Group
            {
                using Pins = PinContainer<SRAM1, SRAM2>;
            };
        };

        struct Group
        {
            using Pins = PinContainer<EDAC, EBIConfig, Latchup>;
        };
    };

    template <std::uint8_t Memory> struct MemoryModule;

    template <> struct MemoryModule<1> : public PinGroupTag
    {
        using Control = PinLocation<gpioPortE, 2>;
        using Buffer = PinLocation<gpioPortC, 14>;
        using Power = PinLocation<gpioPortC, 0>;
        static constexpr decltype(auto) Comparator = ACMP0;

        struct Group
        {
            using Pins = PinContainer<Control, Buffer, Power>;
        };
    };

    template <> struct MemoryModule<2>
    {
        using Control = PinLocation<gpioPortE, 3>;
        using Buffer = PinLocation<gpioPortC, 15>;
        using Power = PinLocation<gpioPortC, 1>;
        static constexpr decltype(auto) Comparator = ACMP1;

        struct Group
        {
            using Pins = PinContainer<Control, Buffer, Power>;
        };
    };

    struct MemoryModules : public PinGroupTag
    {
        struct Group
        {
            using Pins = PinContainer<MemoryModule<1>, MemoryModule<2>>;
        };
    };
}

/** @endcond */

#endif
