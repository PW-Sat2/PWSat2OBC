#ifndef LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_WHOAMI_H_
#define LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_WHOAMI_H_

#include "commands/base.h"
#include "telemetry.h"

namespace drivers
{
    namespace payload
    {
        namespace commands
        {
            /**
             * @brief Command for executing Who Am I data retrieval
             *
             * This command has overriden Execute method to exclude "measure" part of other commands.
             * Also the Command Code is ignored for this reason.
             */
            class WhoamiCommand : public PayloadCommand<0x00, PayloadTelemetry::Status>
            {
              public:
                /**
                 * @brief Constructs @ref PayloadCommand object
                 * @param[in] driver A hardware driver
                 */
                WhoamiCommand(IPayloadDriver& driver);
                virtual OSResult Execute(PayloadTelemetry::Status& output) override;

                /**
                 * @brief Method for validation of returned data.
                 * @param data The data to validate
                 * @returns True vhen data is valid, false when invalid.
                 */
                static bool Validate(const PayloadTelemetry::Status& data);

              protected:
                virtual OSResult Save(const gsl::span<uint8_t>& buffer, PayloadTelemetry::Status& output) override;

              private:
                static constexpr uint8_t ValidWhoAmIResponse = 0x53;
            };
        }
    }
}

#endif /* LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_WHOAMI_H_ */
