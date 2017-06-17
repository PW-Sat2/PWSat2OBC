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
            class WhoamiCommand : public PayloadCommand<0x00>
            {
              public:
                /**
                 * @brief Constructs @ref PayloadCommand object
                 * @param[in] driver A hardware driver
                 */
                WhoamiCommand(IPayloadDriver& driver);
                virtual OSResult Execute() override;

              protected:
                virtual gsl::span<std::uint8_t> GetBuffer() override;
                virtual uint8_t GetDataAddress() const override;
                virtual OSResult Validate() const override;
                virtual OSResult Save() override;

              private:
                std::array<uint8_t, 1> _telemetry;
            };
        }
    }
}

#endif /* LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_WHOAMI_H_ */
