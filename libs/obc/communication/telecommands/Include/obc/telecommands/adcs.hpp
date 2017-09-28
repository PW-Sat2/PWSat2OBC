#ifndef LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_ADCS_HPP_
#define LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_ADCS_HPP_

#pragma once

#include "adcs/adcs.hpp"
#include "base/IHasState.hpp"
#include "base/fwd.hpp"
#include "mission/antenna_task.hpp"
#include "state/fwd.hpp"
#include "telecommunication/telecommand_handling.h"

namespace obc
{
    namespace telecommands
    {
        /**
         * @brief Disable imtq built in detumbling
         * @telecommand
         *
         * Code: 0x24
         * Parameters:
         *  - 8-bit - APID that will be used in response
         *  - 1-byte - new built-in detumbling mask value, true for disabled, false for enabled
         */
        class SetBuiltinDetumblingBlockMaskTelecommand : public telecommunication::uplink::Telecommand<0x24>
        {
          public:
            /**
             * @brief ctor.
             * @param stateContainer_ Reference to current obc state container
             * @param adcsCoordinator_ Reference to adcs subsystem controller
             */
            SetBuiltinDetumblingBlockMaskTelecommand(IHasState<SystemState>& stateContainer_, adcs::IAdcsCoordinator& adcsCoordinator_);

            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            IHasState<SystemState>& stateContainer;
            adcs::IAdcsCoordinator& adcsCoordinator;
        };

        /**
         * @brief Set new Adcs subystem operating mode.
         * @telecommand
         *
         * Code: 0x25
         * Parameters
         *  - 8-bit - APID that will be used in response:
         *  - 1-byte - new adcs subsystem mode
         */
        class SetAdcsModeTelecommand : public telecommunication::uplink::Telecommand<0x25>
        {
          public:
            /**
             * @brief ctor.
             * @param adcsCoordinator_ Reference to adcs subsystem controller
             */
            SetAdcsModeTelecommand(adcs::IAdcsCoordinator& adcsCoordinator_);

            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            void Finish(OSResult result, Writer& writer);

            adcs::IAdcsCoordinator& adcsCoordinator;
        };
    }
}

#endif /* LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_ADCS_HPP_ */
