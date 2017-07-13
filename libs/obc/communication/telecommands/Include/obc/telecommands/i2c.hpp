#ifndef LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_I2C_HPP_
#define LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_I2C_HPP_

#include "i2c/i2c.h"
#include "telecommunication/downlink.h"
#include "telecommunication/telecommand_handling.h"

namespace obc
{
    namespace telecommands
    {
        /**
         * @brief Download any file
         * @ingroup telecommands
         * @telecommand
         *
         * Command code: 0x80
         *
         * Parameters:
         *  - 8-bit - Operation correlation id that will be used in response
         *  - 8-bit - Bus select: 0 - system bus, 1 - payload
         *  - 8-bit - I2C address
         *  - 16-bit - Delay between write and read in milliseconds
         *  - Array of 8-bit - Data to send to I2C bus
         */
        class RawI2CTelecommand final : public telecommunication::uplink::Telecommand<0x80>
        {
          public:
            /**
             * @brief Ctor
             * @param systemBus Reference to System I2C bus
             * @param payload Reference to I2C payload
             */
            RawI2CTelecommand(drivers::i2c::II2CBus& systemBus, drivers::i2c::II2CBus& payload);

            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            /** @brief System bus. */
            drivers::i2c::II2CBus& systemBus;

            /** @brief Payload. */
            drivers::i2c::II2CBus& payload;
        };
    }
}

#endif /* LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_I2C_HPP_ */
