#ifndef LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_RADFET_H_
#define LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_RADFET_H_

#include "commands/base.h"
#include "telemetry.h"

namespace devices
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
                /**
                 * @brief Reads values from buffer into output object
                 * @param buffer Buffer with serialized data
                 * @param output Output object
                 * @return Operation status
                 */
                virtual OSResult Save(const gsl::span<uint8_t>& buffer, PayloadTelemetry::Radfet& output) override;
            };

            /**
             * @brief Command for enabling RadFET module
             */
            class RadFETOnCommand : public RadFETBaseCommand<0x84>
            {
              public:
                /**
                 * @brief Constructs @ref RadFETOnCommand object
                 * @param[in] driver A hardware driver
                 */
                RadFETOnCommand(IPayloadDriver& driver) : RadFETBaseCommand(driver){};
            };

            /**
             * @brief Command for execuring RadFET measurment
             */
            class RadFETMeasureCommand : public RadFETBaseCommand<0x85>
            {
              public:
                /**
                 * @brief Constructs @ref RadFETMeasureCommand object
                 * @param[in] driver A hardware driver
                 */
                RadFETMeasureCommand(IPayloadDriver& driver) : RadFETBaseCommand(driver){};
            };

            /**
             * @brief Command for disabling RadFET module
             */
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
