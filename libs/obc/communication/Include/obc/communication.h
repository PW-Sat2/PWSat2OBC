#ifndef SRC_COMMUNICATION_H_
#define SRC_COMMUNICATION_H_

#include <gsl/span>
#include "comm/CommDriver.hpp"
#include "i2c/i2c.h"
#include "obc/experiments.hpp"
#include "obc/telecommands/experiments.hpp"
#include "obc/fdir.hpp"
#include "obc/telecommands/file_system.hpp"
#include "obc/telecommands/ping.hpp"
#include "telecommunication/telecommand_handling.h"
#include "telecommunication/uplink.h"

namespace obc
{
    /**
     * @brief Object aggregating all supported telecommands
     */
    class Telecommands final
    {
      public:
        /**
         * @brief Initializes @ref Telecommands object
         * @param fs File system
         * @param experiments Experiments
         */
        Telecommands(services::fs::IFileSystem& fs, obc::OBCExperiments& experiments);

        /**
         * Aggregates all telecommand handlers into single span
         * @return Span with all supported telecommands
         */
        gsl::span<telecommunication::uplink::IHandleTeleCommand*> AllTelecommands();

      private:
        /** @brief Ping telecommand */
        obc::telecommands::PingTelecommand _ping;
        /** @brief Download file telecommand */
        obc::telecommands::DownloadFileTelecommand _downloadFileTelecommand;

        /** @brief Perform detumbling experiment */
        obc::telecommands::PerformDetumblingExperiment _performDetumblingExperiment;

        /** @brief Array containg all telecommand handlers */
        telecommunication::uplink::IHandleTeleCommand* _telecommands[3];
    };

    /**
     * @brief OBC <-> Earth communication
     */
    struct OBCCommunication final
    {
        /**
         * @brief Initializes @ref OBCCommunication object
         * @param[in] fdir FDIR mechanisms
         * @param[in] i2cBus I2CBus used by low-level comm driver
         * @param[in] fs File system
         * @param[in] experiments Experiments
         */
        OBCCommunication(obc::FDIR& fdir, drivers::i2c::II2CBus& i2cBus, services::fs::IFileSystem& fs, obc::OBCExperiments& experiments);

        /**
         * @brief Initializes all communication-related drivers and objects
         */
        void Initialize();

        /** @brief Uplink protocol decoder */
        telecommunication::uplink::UplinkProtocol UplinkProtocolDecoder;

        /** @brief Object aggregating supported telecommand */
        Telecommands SupportedTelecommands;

        /** @brief Incoming telecommand handler */
        telecommunication::uplink::IncomingTelecommandHandler TelecommandHandler;

        /** @brief Low-level comm driver */
        devices::comm::CommObject CommDriver;
    };
}

#endif /* SRC_COMMUNICATION_H_ */
