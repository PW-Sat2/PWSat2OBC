#ifndef LIBS_BASE_INCLUDE_BASE_IO_MAP_H_
#define LIBS_BASE_INCLUDE_BASE_IO_MAP_H_

#include <cstdint>

namespace io_map
{
    /**
     * @defgroup io_map_base I/O Map support utilities
     * @{
     */

    /**
     * @brief Tag for type describing pin location
     *
     * Derived type must provide following static members:
     *  - Port of type GPIO_Port_TypeDef
     *  - PinNumber of type std::uint16_t
     */
    struct PinTag
    {
    };

    /**
     * @brief Type describing pin location
     * @tparam TPort GPIO port
     * @tparam TPinNumber Pin number
     */
    template <GPIO_Port_TypeDef TPort, std::uint16_t TPinNumber> struct PinLocation : public PinTag
    {
        /** @brief Selected port */
        static constexpr auto Port = TPort;
        /** @brief Selected pin number */
        static constexpr auto PinNumber = TPinNumber;
    };

    /** @brief Packs many pins into single type */
    template <typename... T> struct PinContainer final
    {
    };

    /**
     * @brief Tag for type describing few pins grouped together (like peripheral pins)
     *
     * Derived type must provider inner type Group with typedef leading to @ref PinContainer
     */
    struct PinGroupTag
    {
    };

    /**
     * @brief Base type for type describing SPI location
     *
     * Derived type must provide following inner types:
     *  - MOSI - valid @ref PinTag for MOSI pin
     *  - MISO - valid @ref PinTag for MISO pin
     *  - CLK - valid @ref PinTag for CLK pin
     */
    template <typename Self> struct SPIPins : public PinGroupTag
    {
        /** @brief SPI pins group */
        struct Group
        {
            /** @brief SPI pins container */
            using Pins = PinContainer<typename Self::MOSI, typename Self::MISO, typename Self::CLK>;
        };
    };

    /**
         * @brief Base type for type describing UART location
         *
         * Derived type must provide following inner types:
         *  - TX - valid @ref PinTag for Transmit pin
         *  - RX - valid @ref PinTag for Receive pin
         */
        template <typename Self> struct UARTPins : public PinGroupTag
        {
            /** @brief UART pins group */
            struct Group
            {
                /** @brief UART pins container */
                using Pins = PinContainer<typename Self::RX, typename Self::TX>;
            };
        };

    /**
     * @brief Base type for type describing I2C location
     *
     * Derived type must provide following inner types:
     *  - SDA - valid @ref PinTag for SDA pin
     *  - SCL - valid @ref PinTag for SCL pin
     */
    template <typename Self> struct I2CPins : public PinGroupTag
    {
        /** @brief I2C pins group */
        struct Group
        {
            /** @brief I2C pins container */
            using Pins = PinContainer<typename Self::SDA, typename Self::SCL>;
        };
    };

    /**
     * @brief Base type for type describing LEUART location
     *
     * Derived type must provide following inner types:
     *  - TX - valid @ref PinTag for TX pin
     *  - RX - valid @ref PinTag for RX pin
     */
    template <typename Self> struct LEUARTPins : public PinGroupTag
    {
        /** @brief LEUART pins group */
        struct Group
        {
            /** @brief LEUART pins container */
            using Pins = PinContainer<typename Self::RX, typename Self::TX>;
        };
    };

    /** @} */
}

#endif /* LIBS_BASE_INCLUDE_BASE_IO_MAP_H_ */
