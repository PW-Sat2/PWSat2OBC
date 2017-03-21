#ifndef LIBS_DRIVERS_TEMP_INCLUDE_TEMP_TEMP_HPP_
#define LIBS_DRIVERS_TEMP_INCLUDE_TEMP_TEMP_HPP_

#include <cstdint>

namespace temp
{
    /**
     * @defgroup temp Temperature sensor
     * @ingroup perhipheral_drivers
     *
     * @{
     */

    /**
     * @brief Temperature sensor interface
     */
    struct ITemperatureReader
    {
        /**
         * @brief Reads raw value of temperature sensor
         * @return Raw sensor value
         */
        virtual std::uint16_t ReadRaw() = 0;
    };

    /**@} */
}

#endif /* LIBS_DRIVERS_TEMP_INCLUDE_TEMP_TEMP_HPP_ */
