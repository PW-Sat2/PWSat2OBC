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
         *  - 8-bit - I2C address
         *  - Array of 8-bit - Data to send to I2C bus
         */
        class RawI2CTelecommand final : public telecommunication::uplink::Telecommand<0x80>
        {
          public:
            /**
             * @brief Ctor
             * @param i2cBus Reference to System I2C bus
             */
            RawI2CTelecommand(drivers::i2c::II2CBus& i2cBus);

            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            static constexpr uint8_t MaxDataSize = telecommunication::downlink::CorrelatedDownlinkFrame::MaxPayloadSize - 1;

            /** @brief File system */
            drivers::i2c::II2CBus& i2cBus;
        };
    }
}

#endif /* LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_I2C_HPP_ */
