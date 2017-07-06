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
            template <std::uint8_t TCommandCode> class RadFETBaseCommand : public PayloadCommand<TCommandCode, PayloadTelemetry::Radfet>
            {
              public:
                /**
                 * @brief Constructs @ref RadFETBaseCommand object
                 * @param[in] driver A hardware driver
                 */
                RadFETBaseCommand(IPayloadDriver& driver);

              protected:
                virtual OSResult Save(const gsl::span<uint8_t>& buffer, PayloadTelemetry::Radfet& output) override;

              private:
            };

            class RadFETOnCommand : public RadFETBaseCommand<0x84>
            {
              public:
                /**
                 * @brief Constructs @ref RadFETOnCommand object
                 * @param[in] driver A hardware driver
                 */
                RadFETOnCommand(IPayloadDriver& driver) : RadFETBaseCommand(driver){};
            };

            class RadFETMeasureCommand : public RadFETBaseCommand<0x85>
            {
              public:
                /**
                 * @brief Constructs @ref RadFETMeasureCommand object
                 * @param[in] driver A hardware driver
                 */
                RadFETMeasureCommand(IPayloadDriver& driver) : RadFETBaseCommand(driver){};
            };

            class RadFETOffCommand : public RadFETBaseCommand<0x86>
            {
              public:
                /**
                 * @brief Constructs @ref RadFETOffCommand object
                 * @param[in] driver A hardware driver
                 */
                RadFETOffCommand(IPayloadDriver& driver) : RadFETBaseCommand(driver){};
            };
        }
    }
}

#endif /* LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_RADFET_H_ */
