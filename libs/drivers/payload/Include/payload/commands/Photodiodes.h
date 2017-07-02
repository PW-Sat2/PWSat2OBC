#ifndef LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_PHOTODIODES_H_
#define LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_PHOTODIODES_H_

#include "commands/base.h"
#include "telemetry.h"

namespace drivers
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
                virtual gsl::span<std::uint8_t> GetBuffer() override;
                virtual uint8_t GetDataAddress() const override;
                virtual OSResult Save(gsl::span<uint8_t>& buffer, PayloadTelemetry::Photodiodes& output) override;

              private:
                std::array<uint8_t, PayloadTelemetry::Photodiodes::DeviceDataLength> _buffer;
            };
        }
    }
}

#endif /* LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_PHOTODIODES_H_ */
