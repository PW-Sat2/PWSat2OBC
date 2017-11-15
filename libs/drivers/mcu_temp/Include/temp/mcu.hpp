#ifndef LIBS_DRIVERS_MCU_TEMP_INCLUDE_TEMP_MCU_HPP_
#define LIBS_DRIVERS_MCU_TEMP_INCLUDE_TEMP_MCU_HPP_

#include <cstdint>
#include "utils.h"

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

        /**
         * @brief Reads raw temperature value.
         * @return Raw temperature value.
         */
        virtual BitValue<std::uint16_t, 12> ReadRaw() = 0;

        /**
         * @brief Reads calibration temperature. This value is hardcoded in MCU.
         * @return Calibration temperature in Celsius.
         *
         * @remark Calibration Temperature and ADC Result at Calibration Temperature values are used for converting raw ADC data to Celsius
         * degree.
         */
        virtual std::int16_t GetCalibrationTemperature() = 0;

        /**
         * @brief Reads ADC result at calibration temperature. This value is hardcoded in MCU.
         * @return ADC result at calibration temperature.
         *
         * @remark Calibration Temperature and ADC Result at Calibration Temperature values are used for converting raw ADC data to Celsius
         * degree.
         */
        virtual std::int16_t GetADCResultAtCalibrationTemperature() = 0;
    };

    /**@} */
}

#endif /* LIBS_DRIVERS_MCU_TEMP_INCLUDE_TEMP_MCU_HPP_ */
