#ifndef LIBS_OBC_HARDWARE_INCLUDE_OBC_GPIO_INTERNAL_H_
#define LIBS_OBC_HARDWARE_INCLUDE_OBC_GPIO_INTERNAL_H_

#include <cstdint>
#include "io_map.h"

namespace obc
{
    namespace gpio
    {
        /**
        * @defgroup obc_hardware_gpio_interal OBC GPIO Support
        * @ingroup obc_hardware_gpio
        *
        * This module contains support structures for checking GPIO pin configuration
        *
        * @{
        */

        /** @brief Conflict detection */
        struct Conflicts
        {
            /** @brief Pin location types */
            enum class Type
            {
                Unknown,     //!< Unknown
                Pin,         //!< Pin
                I2C,         //!< I2C
                SPI,         //!< SPI
                LEUART,      //!< LEUART
                PinContainer //!< PinContainer
            };

            /** @brief Packs many pins into single type */
            template <typename... T> struct PinContainer
            {
            };

            /** @brief Detects pin location type */
            template <typename T> static constexpr Type GetType()
            {
                if (std::is_base_of<io_map::PinTag, T>::value)
                    return Type::Pin;
                if (std::is_base_of<io_map::I2CPinsTag, T>::value)
                    return Type::I2C;
                if (std::is_base_of<io_map::SPIPinsTag, T>::value)
                    return Type::SPI;
                if (std::is_base_of<io_map::LEUARTPinsTag, T>::value)
                    return Type::LEUART;

                return Type::Unknown;
            }

            /**
             * @brief Conflict detection entry-point
             *
             * When conflict is detected @ref value is set to true
             */
            template <Type LeftType, typename Left, Type RightType, typename Right> struct Conflict
            {
                /** @brief If no matching clause is found, swap arguments and try again */
                static constexpr bool value = Conflict<RightType, Right, LeftType, Left>::value;
            };

            /** @brief Detect conflicts between pins */
            template <typename Left, typename Right> struct Conflict<Type::Pin, Left, Type::Pin, Right>
            {
                static constexpr auto LeftPort = Left::PinLocation::Port;           //!< Left pin's port
                static constexpr auto LeftPinNumber = Left::PinLocation::PinNumber; //!< Left pin's number

                static constexpr auto RightPort = Right::PinLocation::Port;           //!< Right pin's port
                static constexpr auto RightPinNumber = Right::PinLocation::PinNumber; //!< Right pin's number

                /** @brief true if Left and Right points to the same pin */
                static constexpr bool value = (LeftPort == RightPort) && (LeftPinNumber == RightPinNumber);
            };

            /** @brief Detects conflicts between I2C pins and Right */
            template <typename Left, Type RightType, typename Right> struct Conflict<Type::I2C, Left, RightType, Right>
            {
                /** @brief Pin container: SDA, SCL */
                using C = PinContainer<typename Left::SDA, typename Left::SCL>;

                /** @brief true if any I2C pin conflicts with Right */
                static constexpr bool value = Conflict<Type::PinContainer, C, RightType, Right>::value;
            };

            /** @brief Detects conflicts between SPI pins and Right */
            template <typename Left, Type RightType, typename Right> struct Conflict<Type::SPI, Left, RightType, Right>
            {
                /** @brief Pin container: MOSI, MISO, CLK */
                using C = PinContainer<typename Left::MOSI, typename Left::MISO, typename Left::CLK>;

                /** @brief true if any SPI pin conflicts with Right */
                static constexpr bool value = Conflict<Type::PinContainer, C, RightType, Right>::value;
            };

            /** @brief Detects conflicts between LEUART pins and Right */
            template <typename Left, Type RightType, typename Right> struct Conflict<Type::LEUART, Left, RightType, Right>
            {
                /** @brief Pin container: TX, RX */
                using C = PinContainer<typename Left::TX, typename Left::RX>;

                /** @brief true if any LEUART pin conflicts with Right */
                static constexpr bool value = Conflict<Type::PinContainer, C, RightType, Right>::value;
            };

            /**
             * @brief Detects conflict between any Pin in container and Right
             */
            template <typename... Pins, Type RightType, typename Right>
            struct Conflict<Type::PinContainer, PinContainer<Pins...>, RightType, Right>
            {
                /**
                 * @brief Checks conflict for pin list
                 * @return bool if conflict detected
                 */
                template <std::uint8_t Tag, typename Head, typename... Rest> static constexpr bool Check()
                {
                    constexpr auto conflicts = Conflict<GetType<Head>(), Head, RightType, Right>::value;

                    if (conflicts)
                        return true;

                    return Check<0, Rest...>();
                }

                /**
                 * @brief Empty list guard
                 * @return Always false
                 */
                template <std::uint8_t Tag> static constexpr bool Check()
                {
                    return false;
                }

                /** @brief true if conflict detected */
                static constexpr bool value = Check<0, Pins...>();
            };

            /** @brief Entry point for conflict checking between two locations */
            template <typename Left, typename Right> static constexpr bool Is()
            {
                constexpr auto leftType = GetType<Left>();
                constexpr auto rightType = GetType<Right>();

                return Conflict<leftType, Left, rightType, Right>::value;
            }
        };

        /**
         * @brief Wrapper class checking that pin locations are not overlapping
         */
        template <template <typename...> class Base, typename... Pins> class VerifyPinsUniqueness final : public Base<Pins...>
        {
          private:
            /**
             * @brief Checks if Pins are not overlapping
             * @return true if conflict detected
             */
            template <std::uint8_t Tag, typename Pin, typename... Rest> static constexpr bool ArePinsNotOverlapping()
            {
                if (HasConflictWith<Pin, Rest...>())
                    return false;

                return ArePinsNotOverlapping<0, Rest...>();
            }

            /**
             * @brief Empty list guard
             * @return Always true
             */
            template <std::uint8_t Tag> static constexpr bool ArePinsNotOverlapping()
            {
                return true;
            }

            /**
             * @brief Checks if Pin has conflict with any other pin location
             * @return true if conflict detected
             */
            template <typename Pin, typename Other, typename... Rest> static constexpr bool HasConflictWith()
            {
                if (Conflicts::template Is<Pin, Other>())
                    return true;

                return HasConflictWith<Pin, Rest...>();
            }

            /**
             * @brief Empty list guard
             * @return Always true
             */
            template <typename Pin> static constexpr bool HasConflictWith()
            {
                return false;
            }

            static_assert(ArePinsNotOverlapping<0, Pins...>(), "Pins must not overlap");
        };

        /** @} */
    }
}

#endif /* LIBS_OBC_HARDWARE_INCLUDE_OBC_GPIO_INTERNAL_H_ */
