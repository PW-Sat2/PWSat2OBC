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
         * @defgroup payload Payload Driver
         * @ingroup device_drivers
         *
         * @brief This module contains drivers for Payload devices, both hardware and high level driver.
         *
         * @{
         */

        /**
         * @brief Payload hardware driver interface
         */
        struct IPayloadDriver
        {
          public:
            /**
             * @brief Returns value indicating if payload is busy and performing measurements.
             * @return True if Payload is busy.
             */
            virtual bool IsBusy() const = 0;

            /**
             * @brief Executes read from Payload using hardware driver.
             * @param outData Buffer containing parameter - address of requested data.
             * @param inData Buffer for incoming data.
             * @return Result status.
             */
            virtual OSResult PayloadRead(gsl::span<std::uint8_t> outData, gsl::span<std::uint8_t> inData) = 0;

            /**
             * @brief Executes write to Payload using hardware driver.
             * @param outData Buffer containing parameter - address of measurement data.
             * @return Result status.
             */
            virtual OSResult PayloadWrite(gsl::span<std::uint8_t> outData) = 0;

            /**
             * @brief Method waiting for Payload be ready to data read.
             * @return Result status.
             */
            virtual OSResult WaitForData() = 0;

            /**
             * @brief Method executed when data from Payload are ready to read. Should only be called from interrupt service routine.
             * @return Result status.
             */
            virtual OSResult RaiseDataReadyISR() = 0;
        };

        /**
         * @brief Payload device driver interface
         */
        struct IPayloadDeviceDriver
        {
            /**
             * @brief Starts measuring SunS reference voltages.
             * @return Result status.
             */
            virtual OSResult MeasureSunSRef() = 0;

            /**
              * @brief Starts measuring Temperature data.
              * @return Result status.
              */
            virtual OSResult MeasureTemperatures() = 0;

            /**
              * @brief Starts measuring Photodiodes data.
              * @return Result status.
              */
            virtual OSResult MeasurePhotodiodes() = 0;

            /**
              * @brief Starts measuring housekeeping data.
              * @return Result status.
              */
            virtual OSResult MeasureHousekeeping() = 0;

            /**
              * @brief Starts measuring RadFET data.
              * @return Result status.
              */
            virtual OSResult MeasureRadFET() = 0;

            /**
             * @brief Gets Payload Who Am I flag.
             * @return Result status.
             */
            virtual OSResult GetWhoami() = 0;
        };

        /* @} */
    }
}

#endif /* LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_INTERFACES_H_ */
