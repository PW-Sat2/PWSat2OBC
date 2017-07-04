#ifndef LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_BASE_H_
#define LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_BASE_H_

#include "interfaces.h"

namespace drivers
{
    namespace payload
    {
        namespace commands
        {
            /**
             * @brief Base class for all Commands for Payload manipulation.
             *
             * Base class for all Commands for Payload manipulation.
             * The template argument is command code.
             */
            template <std::uint8_t TCommandCode, class TOutputDataType, const uint8_t TDeviceDataAddress, const uint8_t TDeviceDataLength>
            class PayloadCommand
            {
              public:
                /**
                 * @brief Constructs @ref PayloadCommand object
                 * @param[in] driver A reference to hardware driver for payload.
                 */
                PayloadCommand(IPayloadDriver& driver);

                /** @brief Telecommand code */
                static constexpr auto CommandCode = TCommandCode;

                /**
                 * @brief Executes command.
                 * @returns Result status.
                 */
                virtual OSResult Execute(TOutputDataType& output);

              protected:
                /**
                  * @brief The method saving retrieved data.
                  * @param buffer The buffer of data retrieved from device.
                  * @param output Data retrieved by command.
                  * @returns Result status.
                  */
                virtual OSResult Save(const gsl::span<uint8_t>& buffer, TOutputDataType& output) = 0;

                /**
                 * @brief The method performing full data request command - read, validate and save.
                 * @param output Data retrieved by command.
                 * @returns Result status.
                 */
                OSResult ExecuteDataCommand(TOutputDataType& output);

                /**
                 * @brief The hardware driver.
                 */
                IPayloadDriver& _driver;

              private:
                static constexpr uint8_t DeviceDataAddress = TDeviceDataAddress;
                static constexpr uint8_t DeviceDataLength = TDeviceDataLength;
                std::array<uint8_t, DeviceDataLength> _buffer;

                OSResult ExecuteCommand();
                OSResult ExecuteDataRead(uint8_t address, gsl::span<uint8_t> buffer);
            };
        }
    }
}

#endif /* LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_BASE_H_ */
