#ifndef LIBS_DRIVERS_MCU_TEMP_INCLUDE_TEMP_MCU_HPP_
#define LIBS_DRIVERS_MCU_TEMP_INCLUDE_TEMP_MCU_HPP_

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
         * @brief Reads temperature converted to Celsius scale
         * @return Temperature in Celsius scale
         *
         * @remark Measured temperature is multiplied by 10
         */
        virtual std::int16_t ReadCelsius() = 0;
    };

    /**@} */
}

#endif /* LIBS_DRIVERS_MCU_TEMP_INCLUDE_TEMP_MCU_HPP_ */
