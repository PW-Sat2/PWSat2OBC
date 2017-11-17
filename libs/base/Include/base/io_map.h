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

    /**
    * @brief Base type for type describing UART location
    *
    * Derived type must provide following inner types:
    *  - TX - valid @ref PinTag for TX pin
    *  - RX - valid @ref PinTag for RX pin
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
     * @brief Group of pins on the same port
     */
    template <GPIO_Port_TypeDef Port, std::uint32_t... PinNumbers> struct PortPins : public PinGroupTag
    {
        /** @brief Port pins group */
        struct Group
        {
            /** @brief Port pins group */
            using Pins = PinContainer<PinLocation<Port, PinNumbers>...>;
        };
    };

    /** @brief Helper class for flattening pin list */
    template <typename Acc, typename... Items> struct FlattenHelper;

    /** @brief Class that converts PinGroup into @ref io_map::PinContainer*/
    template <typename... Accs, typename Group, typename... Rest> struct FlattenHelper<io_map::PinContainer<Accs...>, Group, Rest...>
    {
        /** @brief @ref io_map::PinContainer with inner pins */
        using Inner = typename Group::Group::Pins;

        /** @brief Result */
        using Result = typename FlattenHelper<io_map::PinContainer<Accs...>, Inner, Rest...>::Result;
    };

    /** @brief Class that flattens @ref io_map::PinContainer */
    template <typename... Accs, typename... Pins, typename... Rest>
    struct FlattenHelper<io_map::PinContainer<Accs...>, io_map::PinContainer<Pins...>, Rest...>
    {
        /** @brief Result */
        using Result = typename FlattenHelper<io_map::PinContainer<Accs...>, Pins..., Rest...>::Result;
    };

    /** @brief Class that adds single @ref io_map::PinLocation into result accumulator */
    template <typename... Acc, GPIO_Port_TypeDef Port, std::uint16_t Pin, typename... Rest>
    struct FlattenHelper<io_map::PinContainer<Acc...>, io_map::PinLocation<Port, Pin>, Rest...>
    {
        /** @brief Result */
        using Result = typename FlattenHelper<io_map::PinContainer<Acc..., io_map::PinLocation<Port, Pin>>, Rest...>::Result;
    };

    /** @brief Empty list guard */
    template <typename... Accs> struct FlattenHelper<io_map::PinContainer<Accs...>>
    {
        /** @brief Final @ref io_map::PinContainer */
        using Result = io_map::PinContainer<Accs...>;
    };

    /** @brief Class that flattens pin list given as template parameter pack */
    template <typename... Pins> struct Flatten
    {
        /** @brief Type of @ref io_map::PinContainer with flattened pin locations */
        using Result = typename FlattenHelper<io_map::PinContainer<>, Pins...>::Result;
    };

    /** @} */
}

#endif /* LIBS_BASE_INCLUDE_BASE_IO_MAP_H_ */
