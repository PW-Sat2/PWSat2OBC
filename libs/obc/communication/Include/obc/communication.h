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
#include "obc/telecommands/antenna.hpp"
#include "obc/telecommands/boot_settings.hpp"
#include "obc/telecommands/comm.hpp"
#include "obc/telecommands/experiments.hpp"
#include "obc/telecommands/file_system.hpp"
#include "obc/telecommands/ping.hpp"
#include "obc/telecommands/program_upload.hpp"
#include "obc/telecommands/time.hpp"
#include "program_flash/fwd.hpp"
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
        obc::telecommands::SetTimeCorrectionConfigTelecommand,
        obc::telecommands::SetTimeTelecommand,
        obc::telecommands::PerformDetumblingExperiment,
        obc::telecommands::AbortExperiment,         //
        obc::telecommands::ListFilesTelecommand,    //
        obc::telecommands::EraseBootTableEntry,     //
        obc::telecommands::WriteProgramPart,        //
        obc::telecommands::FinalizeProgramEntry,    //
        obc::telecommands::SetBootSlotsTelecommand, //
        obc::telecommands::SendBeaconTelecommand,   //
        obc::telecommands::StopAntennaDeployment    //
        >;

    /**
     * @brief OBC <-> Earth communication
     */
    struct OBCCommunication final
    {
        /**
         * @brief Initializes @ref OBCCommunication object
         * @param[in] fdir FDIR mechanisms
         * @param[in] commDriver Comm driver
         * @param[in] currentTime Current time
         * @param[in] rtc RTC device
         * @param[in] idleStateController Idle state controller
         * @param[in] disableAntennaDeployment Object responsible for disabling antenna deployment
         * @param[in] stateContainer Container for OBC state
         * @param[in] fs File system
         * @param[in] experiments Experiments
         * @param[in] bootTable Boot table
         * @param[in] bootSettings Boot settings
         * @param[in] telemetry Reference to object that contains current telemetry state.
         */
        OBCCommunication(obc::FDIR& fdir,
            devices::comm::CommObject& commDriver,
            services::time::ICurrentTime& currentTime,
            devices::rtc::IRTC& rtc,
            mission::IIdleStateController& idleStateController,
            mission::antenna::IDisableAntennaDeployment& disableAntennaDeployment,
            IHasState<SystemState>& stateContainer,
            services::fs::IFileSystem& fs,
            obc::OBCExperiments& experiments,
            program_flash::BootTable& bootTable,
            boot::BootSettings& bootSettings,
            IHasState<telemetry::TelemetryState>& telemetry);

        /**
         * @brief Initializes all communication at runlevel 1
         */
        void InitializeRunlevel1();

        /**
         * @brief Initializes all communication at runlevel 2
         */
        void InitializeRunlevel2();

        /** @brief Comm driver */
        devices::comm::CommObject& Comm;

        /** @brief Uplink protocol decoder */
        telecommunication::uplink::UplinkProtocol UplinkProtocolDecoder;

        /** @brief Object aggregating supported telecommands */
        Telecommands SupportedTelecommands;

        /** @brief Incoming telecommand handler */
        telecommunication::uplink::IncomingTelecommandHandler TelecommandHandler;
    };

    /** @} */
}

#endif /* SRC_COMMUNICATION_H_ */
