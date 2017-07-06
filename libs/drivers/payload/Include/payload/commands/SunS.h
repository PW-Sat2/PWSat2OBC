#ifndef LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_SUNS_H_
#define LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_SUNS_H_

#include "commands/base.h"
#include "telemetry.h"

namespace drivers
{
    namespace payload
    {
        namespace commands
        {
            /**
             * @brief Command for executing SunS Reference Voltages measurements and data retrieval
             */
            class SunSCommand : public PayloadCommand<0x80, PayloadTelemetry::SunsRef>
            {
              public:
                /**
                 * @brief Constructs @ref PayloadCommand object
                 * @param[in] driver A hardware driver
                 */
                SunSCommand(IPayloadDriver& driver);

              protected:
                virtual OSResult Save(const gsl::span<uint8_t>& buffer, PayloadTelemetry::SunsRef& output) override;
            };
        }
    }
}

#endif /* LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_SUNS_H_ */
