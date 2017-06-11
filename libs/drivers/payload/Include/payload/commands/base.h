#ifndef LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_BASE_H_
#define LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_BASE_H_

#include "interfaces.h"

namespace drivers
{
    namespace payload
    {
        namespace commands
        {
            template <std::uint8_t TCommandCode> class PayloadCommand
            {
              public:
                /**
                 * @brief Constructs @ref PayloadCommand object
                 * @param[in] driver A reference to hardware driver for payload.
                 */
                PayloadCommand(IPayloadDriver& driver) : _driver(driver)
                {
                }

                /** @brief Telecommand code */
                static constexpr auto CommandCode = TCommandCode;

                OSResult Execute();

              protected:
                virtual gsl::span<std::uint8_t> GetBuffer() = 0;
                virtual uint8_t GetDataAddress() const = 0;
                virtual OSResult Validate() const = 0;
                virtual OSResult Save() = 0;

              private:
                IPayloadDriver& _driver;

                OSResult ExecuteCommand();
                OSResult ExecuteDataRead(uint8_t address, gsl::span<uint8_t> buffer);
            };
        }
    }
}

#endif /* LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_BASE_H_ */
