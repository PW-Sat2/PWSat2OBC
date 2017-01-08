#ifndef LIBS_OBC_HARDWARE_INCLUDE_OBC_GPIO_H_
#define LIBS_OBC_HARDWARE_INCLUDE_OBC_GPIO_H_
#include <type_traits>

#include <em_cmu.h>
#include <em_gpio.h>
#include "gpio-internal.h"
#include "gpio/gpio.h"
#include "io_map.h"

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

    /** @brief Composes all used GPIO pins together */
    template < //
        typename TSlaveSelectFlash1,
        typename TLed0,
        typename TLed1,
        typename TSysClear,
        typename TSPI,
        typename TLEUART,
        typename TI2C0,
        typename TI2C1>
    struct OBCGPIOBase
    {
        /** @brief Slave Select - Flash1 */
        const drivers::gpio::OutputPin<TSlaveSelectFlash1> Flash1ChipSelect;
        /** @brief LED0 */
        const drivers::gpio::OutputPin<TLed0> Led0;
        /** @brief LED1 */
        const drivers::gpio::OutputPin<TLed1> Led1;
        /** @brief SysClear */
        const drivers::gpio::InputPin<TSysClear> SysClear;
        /** @brief SPI */
        const SPIPins<TSPI> SPI;
        /** @brief LEUART */
        const LEUARTPins<TLEUART> LEUART;
        /** @brief I2C0 */
        const I2CPins<TI2C0> I2C_0;
        /** @brief I2C1 */
        const I2CPins<TI2C1> I2C_1;

        /** @brief Initializes GPIO pins */
        void Initialize() const
        {
            CMU_ClockEnable(cmuClock_GPIO, true);

            this->Flash1ChipSelect.Initialize();
            this->Led0.Initialize();
            this->Led1.Initialize();
            this->SysClear.Initialize();
            this->SPI.Initialize();
            this->LEUART.Initialize();
            this->I2C_0.Initialize();
            this->I2C_1.Initialize();
        }
    };

    /** @brief Connects GPIO pins to IO map */
    using OBCGPIO = gpio::VerifyPinsUniqueness<OBCGPIOBase, //
        io_map::SlaveSelectFlash1,                          //
        io_map::Led0,                                       //
        io_map::Led1,                                       //
        io_map::SysClear,                                   //
        io_map::SPI,                                        //
        io_map::LEUART,
        io_map::I2C_0,
        io_map::I2C_1>;

    /** @} */
}

#endif /* LIBS_OBC_HARDWARE_INCLUDE_OBC_GPIO_H_ */
