#ifndef LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_TIME_HPP_
#define LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_TIME_HPP_

#include "base/IHasState.hpp"
#include "mission/time.hpp"
#include "rtc/rtc.hpp"
#include "state/struct.h"
#include "telecommunication/downlink.h"
#include "telecommunication/telecommand_handling.h"
#include "time/ICurrentTime.hpp"

namespace obc
{
    namespace telecommands
    {
        /**
         * @brief Set time correction configuration
         * @ingroup telecommands
         * @telecommand
         *
         * Command code: 0x90
         *
         * Parameters:
         *  - 8-bit - Correlation id that will be used in response
         *  - 16-bit LE - Weight assigned to internal clock
         *  - 16-bit LE - Weight assigned to external clock
         */
        class SetTimeCorrectionConfigTelecommand final : public telecommunication::uplink::Telecommand<0x90>
        {
          public:
            /**
             * @brief Ctor
             * @param[in] stateContainer_ Container for OBC state
             */
            SetTimeCorrectionConfigTelecommand(IHasState<SystemState>& stateContainer_);

            /**
             * @brief Method called when telecommand is received.
             * @param[in] transmitter Reference to object that can be used to send response back
             * @param[in] parameters Parameters contained in telecommand frame
             */
            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            /** @brief Container for OBC state */
            IHasState<SystemState>& stateContainer;
        };

        /**
         * @brief Set time
         * @ingroup telecommands
         * @telecommand
         *
         * Command code: 0x91
         *
         * Parameters:
         *  - 8-bit - APID that will be used in response
         *  - 32-bit LE - New time
         */
        class SetTimeTelecommand final : public telecommunication::uplink::Telecommand<0x91>
        {
          public:
            /**
             * @brief Ctor
             * @param[in] stateContainer_ Container for OBC state
             * @param[in] timeProvider_ Current time provider
             * @param[in] rtc_ Real-time clock device
             * @param[in] timeSynchronization_ Time synchronization container.
             */
            SetTimeTelecommand(IHasState<SystemState>& stateContainer_,
                services::time::ICurrentTime& timeProvider_,
                devices::rtc::IRTC& rtc_,
                mission::ITimeSynchronization& timeSynchronization_);

            /**
             * @brief Method called when telecommand is received.
             * @param[in] transmitter Reference to object that can be used to send response back
             * @param[in] parameters Parameters contained in telecommand frame
             */
            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            /** @brief Container for OBC state */
            IHasState<SystemState>& stateContainer;

            /** @brief Current time provider */
            services::time::ICurrentTime& timeProvider;

            /** @brief Real-time clock device */
            devices::rtc::IRTC& rtc;

            /** @brief Time synchronization container. */
            mission::ITimeSynchronization& timeSynchronization;
        };
    }
}

#endif /* LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_TIME_HPP_ */
