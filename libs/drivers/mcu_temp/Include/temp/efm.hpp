#ifndef LIBS_DRIVERS_MCU_TEMP_INCLUDE_TEMP_EFM_HPP_
#define LIBS_DRIVERS_MCU_TEMP_INCLUDE_TEMP_EFM_HPP_

#include "mcu.hpp"

namespace temp
{
    /**
     * @brief ADC-based temperature sensor for EFM
     * @ingroup temp
     */
    class ADCTemperatureReader : public ITemperatureReader
    {
      public:
        virtual std::int16_t ReadCelsius() override;

      private:
        /**
         * @brief Reads raw value of temperature sensor
         * @return Raw value
         */
        std::uint16_t ReadRaw();
    };
}

#endif /* LIBS_DRIVERS_MCU_TEMP_INCLUDE_TEMP_EFM_HPP_ */
