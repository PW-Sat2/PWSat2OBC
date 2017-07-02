#ifndef LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_HOUSEKEEPING_H_
#define LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_HOUSEKEEPING_H_

#include "commands/base.h"
#include "telemetry.h"

namespace drivers
{
    namespace payload
    {
        namespace commands
        {
            /**
             * @brief Command for executing Housekeeping measurements and data retrieval
             */
            class HousekeepingCommand : public PayloadCommand<0x83, PayloadTelemetry::Housekeeping>
            {
              public:
                /**
                 * @brief Constructs @ref PayloadCommand object
                 * @param[in] driver A hardware driver
                 */
                HousekeepingCommand(IPayloadDriver& driver);

              protected:
                virtual gsl::span<std::uint8_t> GetBuffer() override;
                virtual uint8_t GetDataAddress() const override;
                virtual OSResult Save(gsl::span<uint8_t>& buffer, PayloadTelemetry::Housekeeping& output) override;

              private:
                std::array<uint8_t, PayloadTelemetry::Housekeeping::DeviceDataLength> _buffer;
            };
        }
    }
}

#endif /* LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_HOUSEKEEPING_H_ */
