#ifndef SRC_COMMUNICATION_H_
#define SRC_COMMUNICATION_H_

#include <gsl/span>
#include "comm/CommDriver.hpp"
#include "i2c/i2c.h"
#include "telecommunication/telecommand_handling.h"
#include "telecommunication/uplink.h"

namespace obc
{
    /**@brief Dummy ping telecommand that responds with pong */
    class PingTelecommand final : public telecommands::handling::IHandleTeleCommand
    {
      public:
        virtual void Handle(devices::comm::ITransmitFrame& transmitter, gsl::span<const std::uint8_t> parameters) override;

        virtual std::uint8_t CommandCode() const override;
    };

    /**
     * @brief Object aggregating all supported telecommands
     */
    class Telecommands final
    {
      public:
        /**
         * Initializes @ref Telecommands object
         */
        Telecommands();

        /**
         * Aggregates all telecommand handlers into single span
         * @return Span with all supported telecommands
         */
        gsl::span<telecommands::handling::IHandleTeleCommand*> AllTelecommands();

      private:
        /** @brief Ping telecommand */
        PingTelecommand _ping;

        /** @brief Array containg all telecommand handlers */
        telecommands::handling::IHandleTeleCommand* _telecommands[1];
    };

    /**
     * @brief OBC <-> Earth communication
     */
    struct OBCCommunication final
    {
        /**
         * @brief Initializes @ref OBCCommunication object
         * @param[in] i2cBus I2CBus used by low-level comm driver
         */
        OBCCommunication(drivers::i2c::II2CBus& i2cBus);

        /**
         * @brief Initializes all communication-related drivers and objects
         */
        void Initialize();

        /** @brief Uplink protocol decoder */
        telecommands::UplinkProtocol UplinkProtocolDecoder;

        /** @brief Object aggregating supported telecommand */
        Telecommands SupportedTelecommands;

        /** @brief Incoming telecommand handler */
        telecommands::handling::IncomingTelecommandHandler TelecommandHandler;

        /** @brief Low-level comm driver */
        devices::comm::CommObject CommDriver;
    };
}

#endif /* SRC_COMMUNICATION_H_ */
