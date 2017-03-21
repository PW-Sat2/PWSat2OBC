#ifndef LIBS_DRIVERS_TEMP_INCLUDE_TEMP_EFM_HPP_
#define LIBS_DRIVERS_TEMP_INCLUDE_TEMP_EFM_HPP_

#include "temp.hpp"

namespace temp
{
    /**
     * @brief ADC-based temperature sensor for EFM
     * @ingroup temp
     */
    class ADCTemperatureReader : public ITemperatureReader
    {
      public:
        virtual std::uint16_t ReadRaw() override;

        /**
         * @brief Reads temperature converted to Celsius scale
         * @return Temperature in Celsius scale
         */
        std::int16_t ReadCelsius();
    };
}

#endif /* LIBS_DRIVERS_TEMP_INCLUDE_TEMP_EFM_HPP_ */
