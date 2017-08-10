#ifndef LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_DEVICES_H_
#define LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_DEVICES_H_

#include <cstdint>
#include <gsl/span>
#include "base/os.h"

#include "interfaces.h"

namespace devices
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
             * @param output Retrieved data.
             * @return Result status.
             */
            virtual OSResult GetWhoami(PayloadTelemetry::Status& output) override;

            /**
             * @brief Validates Payload Who Am I flag.
             * @param data Retrieved data.
             * @return True if data is correct, false otherwise.
             */
            virtual bool ValidateWhoami(const PayloadTelemetry::Status& data) const override;

            /**
              * @brief Starts measuring SunS reference voltages.
              * @param output Retrieved data.
              * @return Result status.
              */
            virtual OSResult MeasureSunSRef(PayloadTelemetry::SunsRef& output) override;

            /**
              * @brief Starts measuring Temperature data.
              * @param output Retrieved data.
              * @return Result status.
              */
            virtual OSResult MeasureTemperatures(PayloadTelemetry::Temperatures& output) override;

            /**
              * @brief Starts measuring Photodiodes data.
              * @param output Retrieved data.
              * @return Result status.
              */
            virtual OSResult MeasurePhotodiodes(PayloadTelemetry::Photodiodes& output) override;

            /**
              * @brief Starts measuring housekeeping data.
              * @param output Retrieved data.
              * @return Result status.
              */
            virtual OSResult MeasureHousekeeping(PayloadTelemetry::Housekeeping& output) override;

            /**
              * @brief Turns on RadFET.
              * @param output Retrieved data.
              * @return Result status.
              */
            virtual OSResult RadFETOn(PayloadTelemetry::Radfet& output) override;

            /**
              * @brief Starts measuring RadFET data.
              * @param output Retrieved data.
              * @return Result status.
              */
            virtual OSResult MeasureRadFET(PayloadTelemetry::Radfet& output) override;

            /**
              * @brief Turns off RadFET.
              * @param output Retrieved data.
              * @return Result status.
              */
            virtual OSResult RadFETOff(PayloadTelemetry::Radfet& output) override;

          private:
            IPayloadDriver& _driver;
        };

        /* @} */
    }
}

#endif /* LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_DEVICES_H_ */
