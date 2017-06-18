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
                virtual OSResult Validate() const override;
                virtual OSResult Save(PayloadTelemetry::Photodiodes& output) override;

              private:
                union PhotodiodesTelemetryBuffered {
                    PayloadTelemetry::Photodiodes data;
                    std::array<uint8_t, sizeof(PayloadTelemetry::Photodiodes)> buffer;
                    static_assert(sizeof(data) == sizeof(buffer), "Incorrect size buffered Telemetry");
                } _telemetry;
            };
        }
    }
}

#endif /* LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_PHOTODIODES_H_ */
