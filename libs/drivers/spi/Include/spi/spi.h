#ifndef LIBS_DRIVERS_SPI_INCLUDE_SPI_SPI_H_
#define LIBS_DRIVERS_SPI_INCLUDE_SPI_SPI_H_

#include <cstdint>
#include <gsl/span>
#include "utils.h"

#include "gpio/forward.h"

namespace drivers
{
    namespace spi
    {
        /**
         * @defgroup spi SPI driver wrapper
         * @ingroup perhipheral_drivers
         *
         * This module defines C++ friendly SPI interface
         *
         * @{
         */

        /**
         * @brief SPI interface
         */
        struct ISPIInterface
        {
            /** @brief Selects slave */
            virtual void Select() = 0;
            /** @brief Deselects slave */
            virtual void Deselect() = 0;
            /**
             * @brief Writes buffer to device
             * @param[in] buffer Input buffer
             */
            virtual void Write(gsl::span<const std::uint8_t> buffer) = 0;
            /**
             * @brief Reads from device
             * @param[in] buffer Output buffer
             */
            virtual void Read(gsl::span<std::uint8_t> buffer) = 0;
        };

        /**
         * @brief RAII-style selecting slave
         *
         * Example
         * @code
         * {
         * 		SPISelectSlave select(this->_spi); // slave is selected
         *
         * 		// some code interacting with slave
         * } // at the end of block slave is deselected
         * @endcode
         */
        class SPISelectSlave final : private NotCopyable, private NotMoveable
        {
          public:
            /**
             * @brief Selects slave using specified SPI interface
             * @param spi SPI interface
             */
            SPISelectSlave(ISPIInterface& spi);

            /**
             * @brief Deselects slave
             */
            ~SPISelectSlave();

          private:
            /** @brief Used SPI interface */
            ISPIInterface& _spi;
        };

        /** @} */
    }
}

#endif /* LIBS_DRIVERS_SPI_INCLUDE_SPI_SPI_H_ */
