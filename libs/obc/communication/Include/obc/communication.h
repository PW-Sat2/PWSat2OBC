#ifndef SRC_COMMUNICATION_H_
#define SRC_COMMUNICATION_H_

#include <array>
#include <gsl/span>
#include <tuple>
#include "comm/CommDriver.hpp"
#include "i2c/i2c.h"
#include "mission/comm.hpp"
#include "obc/experiments.hpp"
#include "obc/fdir.hpp"
#include "obc/telecommands/comm.hpp"
#include "obc/telecommands/experiments.hpp"
#include "obc/telecommands/file_system.hpp"
#include "obc/telecommands/ping.hpp"
#include "obc/telecommands/program_upload.hpp"
#include "telecommunication/telecommand_handling.h"
#include "telecommunication/uplink.h"
#include "time/ICurrentTime.hpp"

namespace obc
{
    /**
     * @defgroup obc_communication OBC Telecommunication
     * @ingroup obc
     *
     * @{
     */

    /**
     * @brief Helper type for holding many telecommands
     * @tparam Telecommands Types of telecommands
     */
    template <typename... Telecommands> class TelecommandsHolder final
    {
      public:
        /**
         * @brief Ctor
         * @param telecommands Telecommands instances
         */
        TelecommandsHolder(Telecommands&&... telecommands);

        /**
         * @brief Gets list of telecommands
         * @return List of telecommands
         */
        gsl::span<telecommunication::uplink::IHandleTeleCommand*> Get();

      private:
        /**
         * @brief Initialize pointers - single step
         */
        template <std::size_t N, typename Head, typename... Rest> void InitPtr()
        {
            auto& ref = std::get<N>(this->_telecommands);

            this->_pointers[N] = reinterpret_cast<telecommunication::uplink::IHandleTeleCommand*>(&ref);

            InitPtr<N + 1, Rest...>();
        }

        /**
         * @brief Initialize pointers - stop condition
         */
        template <std::size_t N> void InitPtr()
        {
        }

        /** @brief Telecommand instances */
        std::tuple<Telecommands...> _telecommands;
        /** @brief Pointers to telecommands */
        std::array<telecommunication::uplink::IHandleTeleCommand*, sizeof...(Telecommands)> _pointers;

        /**
         * @brief Checks if command codes are unique
         * @return true if command codes are unique
         */
        template <bool Tag, std::uint8_t Head, std::uint8_t... Rest> static constexpr bool AreCodesUnique()
        {
            if (IsValueInList<std::uint8_t>::IsInList<Head, Rest...>())
            {
                return false;
            }

            return AreCodesUnique<true, Rest...>();
        }

        /**
         * @brief Checks if command codes are unique (stop condition)
         * @return Always true
         */
        template <bool Tag> static constexpr bool AreCodesUnique()
        {
            return true;
        }

        static_assert(AreCodesUnique<true, Telecommands::Code...>(), "Telecommand codes must be unique");
    };

    template <typename... Telecommands>
    TelecommandsHolder<Telecommands...>::TelecommandsHolder(Telecommands&&... telecommands) : _telecommands{telecommands...}
    {
        InitPtr<0, Telecommands...>();
    }

    template <typename... Telecommands> gsl::span<telecommunication::uplink::IHandleTeleCommand*> TelecommandsHolder<Telecommands...>::Get()
    {
        return this->_pointers;
    }

    /** @brief Typedef with all supported telecommands */
    using Telecommands = TelecommandsHolder< //
        obc::telecommands::PingTelecommand,
        obc::telecommands::DownloadFileTelecommand,
        obc::telecommands::EnterIdleStateTelecommand,
        obc::telecommands::RemoveFileTelecommand,
        obc::telecommands::PerformDetumblingExperiment,
        obc::telecommands::AbortExperiment,     //
        obc::telecommands::EraseBootTableEntry, //
        obc::telecommands::WriteProgramPart,    //
        obc::telecommands::FinalizeProgramEntry //
        >;

    /**
     * @brief OBC <-> Earth communication
     */
    struct OBCCommunication final
    {
        /**
         * @brief Initializes @ref OBCCommunication object
         * @param[in] fdir FDIR mechanisms
         * @param[in] i2cBus I2CBus used by low-level comm driver
         * @param[in] currentTime Current time
         * @param[in] idleStateController Idle state controller
         * @param[in] fs File system
         * @param[in] experiments Experiments
         */
        OBCCommunication(obc::FDIR& fdir,
            drivers::i2c::II2CBus& i2cBus,
            services::time::ICurrentTime& currentTime,
            mission::IIdleStateController& idleStateController,
            services::fs::IFileSystem& fs,
            obc::OBCExperiments& experiments);

        /**
         * @brief Initializes all communication-related drivers and objects
         */
        void Initialize();

        /** @brief Uplink protocol decoder */
        telecommunication::uplink::UplinkProtocol UplinkProtocolDecoder;

        /** @brief Object aggregating supported telecommands */
        Telecommands SupportedTelecommands;

        /** @brief Incoming telecommand handler */
        telecommunication::uplink::IncomingTelecommandHandler TelecommandHandler;

        /** @brief Low-level comm driver */
        devices::comm::CommObject CommDriver;
    };

    /** @} */
}

#endif /* SRC_COMMUNICATION_H_ */
