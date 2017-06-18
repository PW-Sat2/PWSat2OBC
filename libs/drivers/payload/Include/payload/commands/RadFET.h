#ifndef LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_RADFET_H_
#define LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_RADFET_H_

#include "commands/base.h"
#include "telemetry.h"

namespace drivers
{
    namespace payload
    {
        namespace commands
        {
            /**
             * @brief Command for executing RadFET measurements and data retrieval
             */
            class RadFETCommand : public PayloadCommand<0x84, PayloadTelemetry::Radfet>
            {
              public:
                /**
                 * @brief Constructs @ref PayloadCommand object
                 * @param[in] driver A hardware driver
                 */
                RadFETCommand(IPayloadDriver& driver);

              protected:
                virtual gsl::span<std::uint8_t> GetBuffer() override;
                virtual uint8_t GetDataAddress() const override;
                virtual OSResult Validate() const override;
                virtual OSResult Save(PayloadTelemetry::Radfet& output) override;

              private:
                union RadFETTelemetryBuffered {
                    PayloadTelemetry::Radfet data;
                    std::array<uint8_t, sizeof(PayloadTelemetry::Radfet)> buffer;
                    static_assert(sizeof(data) == sizeof(buffer), "Incorrect size buffered Telemetry");
                } _telemetry;
            };
        }
    }
}

#endif /* LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_RADFET_H_ */
