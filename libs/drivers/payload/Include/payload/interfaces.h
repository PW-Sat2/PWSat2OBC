#ifndef LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_INTERFACES_H_
#define LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_INTERFACES_H_

#include <cstdint>
#include <gsl/span>
#include "base/os.h"

namespace drivers
{
    namespace payload
    {
        /**
         * @brief Payload hardware driver interface
         */
        struct IPayloadDriver
        {
          public:
            virtual bool IsBusy() const = 0;
            virtual OSResult PayloadRead(gsl::span<std::uint8_t> outData, gsl::span<std::uint8_t> inData) = 0;
            virtual OSResult PayloadWrite(gsl::span<std::uint8_t> outData) = 0;
            virtual OSResult WaitForData() = 0;
        };

        /**
         * @brief Payload device driver interface
         */
        struct IPayloadDeviceDriver
        {
            /**
             * @brief Starts measuring SunS reference voltages.
             */
            virtual OSResult MeasureSunSRef() = 0;

            /**
              * @brief Starts measuring Temperature data.
              */
            virtual OSResult MeasureTemperatures() = 0;

            /**
              * @brief Starts measuring Photodiodes data.
              */
            virtual OSResult MeasurePhotodiodes() = 0;

            /**
              * @brief Starts measuring housekeeping data.
              */
            virtual OSResult MeasureHousekeeping() = 0;

            /**
              * @brief Starts measuring RadFET data.
              */
            virtual OSResult MeasureRadFET() = 0;

            /**
             * @briesf Gets Payload Who Am I flag.
             */
            virtual OSResult GetWhoami() = 0;
        };
    }
}

#endif /* LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_INTERFACES_H_ */
