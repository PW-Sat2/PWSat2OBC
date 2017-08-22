#include "base/IHasState.hpp"
#include "comm/comm.hpp"
#include "state/fwd.hpp"
#include "telecommunication/downlink.h"
#include "telecommunication/telecommand_handling.h"

namespace obc
{
    namespace telecommands
    {
        /**
         * @brief Set periodic message telecommand
         * @ingroup obc_telecommands
         * @telecommand
         *
         * Parameters:
         *  - Correlation ID (8 bits)
         *  - Repeat interval (in minutes, 8 bits)
         *  - Repeat count (8 bits)
         */
        class SetPeriodicMessageTelecommand : public telecommunication::uplink::Telecommand<0x05>
        {
          public:
            /**
             * @brief Ctor
             * @param stateContainer System state accessor
             */
            SetPeriodicMessageTelecommand(IHasState<SystemState>& stateContainer);
            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            /** @brief System state accessor */
            IHasState<SystemState>& _stateContainer;
        };

        /**
         * @brief Sends periodic message immediately
         * @ingroup obc_telecommands
         * @telecommand
         *
         * Parameters: None
         */
        class SendPeriodicMessageTelecommand : public telecommunication::uplink::Telecommand<0x23>
        {
          public:
            /**
             * @brief Ctor
             * @param stateContainer System state accessor
             */
            SendPeriodicMessageTelecommand(IHasState<SystemState>& stateContainer);
            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            IHasState<SystemState>& _stateContainer;
        };
    }
}
