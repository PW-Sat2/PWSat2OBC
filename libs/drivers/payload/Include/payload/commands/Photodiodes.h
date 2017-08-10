#ifndef LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_PHOTODIODES_H_
#define LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_PHOTODIODES_H_

#include "commands/base.h"
#include "telemetry.h"

namespace devices
{
    namespace payload
    {
        namespace commands
        {
            /**
             * @brief Command for executing Photodiodes measurements and data retrieval
             */
            class PhotodiodesCommand : public PayloadCommand<0x82, PayloadTelemetry::Photodiodes>
            {
              public:
                /**
                 * @brief Constructs @ref PayloadCommand object
                 * @param[in] driver A hardware driver
                 */
                PhotodiodesCommand(IPayloadDriver& driver);

              protected:
                virtual OSResult Save(const gsl::span<uint8_t>& buffer, PayloadTelemetry::Photodiodes& output) override;
            };
        }
    }
}

#endif /* LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_PHOTODIODES_H_ */
