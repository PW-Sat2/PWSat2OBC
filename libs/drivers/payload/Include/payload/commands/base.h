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
            template <std::uint8_t TCommandCode, class TOutputDataType> class PayloadCommand
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
                  * @brief Provides buffer for data retrieval.
                  * @returns Buffer for data retrieval.
                  */
                virtual gsl::span<std::uint8_t> GetBuffer() = 0;

                /**
                  * @brief Returns address of telemetry data.
                  * @returns Address of data that will be retrieved by this command.
                  */
                virtual uint8_t GetDataAddress() const = 0;

                /**
                  * @brief The method saving retrieved data.
                  * @param output Data retrieved by command.
                  * @returns Result status.
                  */
                virtual OSResult Save(TOutputDataType& output) = 0;

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
                OSResult ExecuteCommand();
                OSResult ExecuteDataRead(uint8_t address, gsl::span<uint8_t> buffer);
            };
        }
    }
}

#endif /* LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_BASE_H_ */
