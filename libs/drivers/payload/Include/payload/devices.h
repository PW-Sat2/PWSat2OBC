#ifndef LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_DEVICES_H_
#define LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_DEVICES_H_

#include <cstdint>
#include <gsl/span>
#include "base/os.h"
//#include "utils.h"

#include "interfaces.h"

namespace drivers
{
    namespace payload
    {
        /**
         * @brief Payload driver
         */
        class PayloadDeviceDriver : public IPayloadDeviceDriver
        {
          public:
            /**
             * @brief Ctor
             * @param driver Payload hardware driver
             */
            PayloadDeviceDriver(IPayloadDriver& driver);

            virtual OSResult MeasureSunSRef() override;
            virtual OSResult MeasureTemperatures() override;
            virtual OSResult MeasurePhotodiodes() override;
            virtual OSResult MeasureHousekeeping() override;
            virtual OSResult MeasureRadFET() override;

          private:
            IPayloadDriver& _driver;
        };

        /* @} */
    }
}

#endif /* LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_DEVICES_H_ */
