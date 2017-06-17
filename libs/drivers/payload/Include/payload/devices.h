#ifndef LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_DEVICES_H_
#define LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_DEVICES_H_

#include <cstdint>
#include <gsl/span>
#include "base/os.h"

#include "interfaces.h"

namespace drivers
{
    namespace payload
    {
        /**
         * @defgroup payload_devices Payload Devices Driver
         * @ingroup payload
         *
         * @brief This module contains high level driver for Payload Devices.
         *
         * @{
         */

        /**
         * @brief Payload Device driver
         */
        class PayloadDeviceDriver : public IPayloadDeviceDriver
        {
          public:
            /**
             * @brief Ctor
             * @param driver Payload hardware driver
             */
            PayloadDeviceDriver(IPayloadDriver& driver);

            /**
             * @brief Gets Payload Who Am I flag.
             * @return Result status.
             */
            virtual OSResult GetWhoami() override;

            /**
              * @brief Starts measuring SunS reference voltages.
              * @return Result status.
              */
            virtual OSResult MeasureSunSRef() override;

            /**
              * @brief Starts measuring Temperature data.
              * @return Result status.
              */
            virtual OSResult MeasureTemperatures() override;

            /**
              * @brief Starts measuring Photodiodes data.
              * @return Result status.
              */
            virtual OSResult MeasurePhotodiodes() override;

            /**
              * @brief Starts measuring housekeeping data.
              * @return Result status.
              */
            virtual OSResult MeasureHousekeeping() override;

            /**
              * @brief Starts measuring RadFET data.
              * @return Result status.
              */
            virtual OSResult MeasureRadFET() override;

          private:
            IPayloadDriver& _driver;
        };

        /* @} */
    }
}

#endif /* LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_DEVICES_H_ */
