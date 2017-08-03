#ifndef LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_TEMPERATURES_H_
#define LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_TEMPERATURES_H_

#include "commands/base.h"
#include "telemetry.h"

namespace devices
{
    namespace payload
    {
        namespace commands
        {
            /**
             * @brief Command for executing Temperatures measurements and data retrieval
             */
            class TemperaturesCommand : public PayloadCommand<0x81, PayloadTelemetry::Temperatures>
            {
              public:
                /**
                 * @brief Constructs @ref PayloadCommand object
                 * @param[in] driver A hardware driver
                 */
                TemperaturesCommand(IPayloadDriver& driver);

              protected:
                virtual OSResult Save(const gsl::span<uint8_t>& buffer, PayloadTelemetry::Temperatures& output) override;
            };
        }
    }
}

#endif /* LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_TEMPERATURES_H_ */
