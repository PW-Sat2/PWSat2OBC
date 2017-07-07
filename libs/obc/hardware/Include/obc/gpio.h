#ifndef LIBS_OBC_HARDWARE_INCLUDE_OBC_GPIO_H_
#define LIBS_OBC_HARDWARE_INCLUDE_OBC_GPIO_H_
#include <type_traits>

#include <em_cmu.h>
#include <em_gpio.h>
#include "gpio-internal.h"
#include "gpio/gpio.h"
#include "mcu/io_map.h"
#include "payload/io_map.h"

namespace obc
{
    /**
     * @defgroup obc_hardware_gpio OBC GPIO
     * @ingroup obc_hardware
     *
     * @{
     */

    /** @brief Structure describing I2C pins */
    template <typename Location> struct I2CPins
    {
        /** @brief SDA */
        const drivers::gpio::OutputPin<typename Location::SDA> SDA;
        /** @brief SCL */
        const drivers::gpio::OutputPin<typename Location::SCL> SCL;

        /** @brief Initializes I2C pins */
        void Initialize() const
        {
            this->SDA.Initialize();
            this->SCL.Initialize();
        }
    };

    /** @brief Structure describing SPI pins */
    template <typename Location> struct SPIPins
    {
        /** @brief MOSI */
        const drivers::gpio::OutputPin<typename Location::MOSI> MOSI;
        /** @brief MISO */
        const drivers::gpio::InputPin<typename Location::MISO> MISO;
        /** @brief CLK */
        const drivers::gpio::OutputPin<typename Location::CLK> CLK;

        /** @brief Initializes SPI pins */
        void Initialize() const
        {
            this->MOSI.Initialize();
            this->MISO.Initialize();
            this->CLK.Initialize();
        }
    };

    /** @brief Structure describing LEUART pins */
    template <typename Location> struct LEUARTPins
    {
        /** @brief TX */
        const drivers::gpio::OutputPin<typename Location::TX> TX;
        /** @brief RX */
        const drivers::gpio::InputPin<typename Location::RX> RX;
        /** @brief Initializes LEUART pins */
        void Initialize() const
        {
            this->TX.Initialize();
            this->RX.Initialize();
        }
    };

    /** @brief Structure describing UART pins */
    template <typename Location> struct UARTPins
    {
        /** @brief TX */
        const drivers::gpio::OutputPin<typename Location::TX> TX;
        /** @brief RX */
        const drivers::gpio::InputPin<typename Location::RX> RX;
        /** @brief Initializes UART pins */
        void Initialize() const
        {
            this->TX.Initialize();
            this->RX.Initialize();
        }
    };

    /**
     * @brief Composes all used GPIO pins together
     *
     * @remark All used pin locations must derive from tags defined in base/io_map.h
     */
    template < //
        typename TSlaveSelectFlash1,
        typename TSlaveSelectFlash2,
        typename TSlaveSelectFlash3,
        typename TSlaveSelectFram1,
        typename TSlaveSelectFram2,
        typename TSlaveSelectFram3,
        typename TTimeIndicator,
        typename TBootIndicator,
        typename TSPI,
#ifdef USE_LEUART
        typename TLEUART,
#else
        typename TUART,
#endif
        typename TI2C0,
        typename TI2C1,
        typename TExternalWatchdogPin,
        typename TBSP,
        typename TMemoryModules,
        typename TSailState,
        typename PayloadInterrupt,
        typename SunsInterrupt>
    struct OBCGPIOBase
    {
        /** @brief Slave Select - Flash1 */
        const drivers::gpio::OutputPin<TSlaveSelectFlash1> Flash1ChipSelect;
        /** @brief Slave Select - Flash2 */
        const drivers::gpio::OutputPin<TSlaveSelectFlash2> Flash2ChipSelect;
        /** @brief Slave Select - Flash3 */
        const drivers::gpio::OutputPin<TSlaveSelectFlash3> Flash3ChipSelect;
        /** @brief Slave Select - Fram1 */
        const drivers::gpio::OutputPin<TSlaveSelectFram1> Fram1ChipSelect;
        /** @brief Slave Select - Fram2 */
        const drivers::gpio::OutputPin<TSlaveSelectFram2> Fram2ChipSelect;
        /** @brief Slave Select - Fram3 */
        const drivers::gpio::OutputPin<TSlaveSelectFram3> Fram3ChipSelect;
        /** @brief Time indicator. Blinks with passage of time */
        const drivers::gpio::OutputPin<TTimeIndicator> TimeIndicator;
        /** @brief Boot indicator. Enabled once the obc starts up */
        const drivers::gpio::OutputPin<TBootIndicator> BootIndicator;
        /** @brief SPI */
        const SPIPins<TSPI> SPI;
#ifdef USE_LEUART
        /** @brief LEUART */
        const LEUARTPins<TLEUART> LEUART;
#else
        /** @brief UART */
        const UARTPins<TUART> UART;
#endif

        /** @brief I2C0 */
        const I2CPins<TI2C0> I2C_0;
        /** @brief I2C1 */
        const I2CPins<TI2C1> I2C_1;

        /** @brief Initializes GPIO pins */
        void Initialize() const
        {
            CMU_ClockEnable(cmuClock_GPIO, true);

            this->Flash1ChipSelect.Initialize();
            this->Flash2ChipSelect.Initialize();
            this->Flash3ChipSelect.Initialize();
            this->Fram1ChipSelect.Initialize();
            this->Fram2ChipSelect.Initialize();
            this->Fram3ChipSelect.Initialize();
            this->TimeIndicator.Initialize();
            this->BootIndicator.Initialize();
            this->SPI.Initialize();
#ifdef USE_LEUART
            this->LEUART.Initialize();
#else
            this->UART.Initialize();
#endif
            this->I2C_0.Initialize();
            this->I2C_1.Initialize();
        }
    };

    /** @brief Connects GPIO pins to IO map */
    using OBCGPIO = gpio::VerifyPinsUniqueness<OBCGPIOBase, //
        io_map::SlaveSelectFlash1,                          //
        io_map::SlaveSelectFlash2,                          //
        io_map::SlaveSelectFlash3,                          //
        io_map::SlaveSelectFram1,                           //
        io_map::SlaveSelectFram2,                           //
        io_map::SlaveSelectFram3,                           //
        io_map::TimeIndicator,                              //
        io_map::BootIndicator,                              //
        io_map::SPI,                                        //
#ifdef USE_LEUART
        io_map::LEUART,
#else
        io_map::UART,
#endif
        io_map::I2C_0,
        io_map::I2C_1,
        io_map::Watchdog::ExternalWatchdogPin,
        io_map::BSP,
        io_map::MemoryModules,
        io_map::SailDeployed,
        io_map::PayloadInterrupt,
        io_map::SunsInterrupt>;

    /** @} */
}

#endif /* LIBS_OBC_HARDWARE_INCLUDE_OBC_GPIO_H_ */
