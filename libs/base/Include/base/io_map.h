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
     * @brief Tag for type describing I2C location
     *
     * Derived type must provide following inner types:
     *  - SDA - valid @ref PinTag for SDA pin
     *  - SCL - valid @ref PinTag for SCL pin
     */
    struct I2CPinsTag
    {
    };

    /**
     * @brief Tag for type describing SPI location
     *
     * Derived type must provide following inner types:
     *  - MOSI - valid @ref PinTag for MOSI pin
     *  - MISO - valid @ref PinTag for MISO pin
     *  - CLK - valid @ref PinTag for CLK pin
     */
    struct SPIPinsTag
    {
    };

    /**
     * @brief Tag for type describing LEUART location
     *
     * Derived type must provide following inner types:
     *  - TX - valid @ref PinTag for TX pin
     *  - RX - valid @ref PinTag for RX pin
     */
    struct LEUARTPinsTag
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

    /** @} */
}

#endif /* LIBS_BASE_INCLUDE_BASE_IO_MAP_H_ */
