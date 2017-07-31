#ifndef LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_FDIR_HPP_
#define LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_FDIR_HPP_

#include "comm/comm.hpp"
#include "error_counter/error_counter.hpp"
#include "obc/fdir.hpp"
#include "telecommunication/uplink.h"

namespace obc
{
    namespace telecommands
    {
        /**
         * @brief Set error counter config telecommand
         * @ingroup telecommands
         * @telecommand
         *
         * Parameters:
         *  - Correlation ID (8 bits)
         *  - Device ID (8 bits)
         *  - Error limit (8 bits)
         *  - Increment (8 bits)
         *  - Decrement (8 bits)
         *  - Device ID (8 bits)
         *  - Error limit (8 bits)
         *  - Increment (8 bits)
         *  - Decrement (8 bits)
         *  - ... (up to frame size/max number of devices)
         */
        class SetErrorCounterConfig : public telecommunication::uplink::Telecommand<0x02>
        {
          public:
            /**
             * @brief Ctor
             * @param config Error counter config
             */
            SetErrorCounterConfig(ISetErrorCounterConfig& config);

            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            /** @brief Error counter config */
            ISetErrorCounterConfig& _config;
        };

        /**
         * @brief Get error counter config telecommand
         * @ingroup telecommands
         * @telecommand
         *
         * Parameters: None
         */
        class GetErrorCountersConfigTelecommand : public telecommunication::uplink::Telecommand<0x06>
        {
          public:
            /**
             * @brief Ctor
             * @param errorCounting Error counting service
             * @param config Error counting configuration
             */
            GetErrorCountersConfigTelecommand(
                error_counter::IErrorCounting& errorCounting, error_counter::IErrorCountingConfigration& config);

            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            /** @brief Error counting service */
            error_counter::IErrorCounting& _errorCounting;
            /** @brief Error counting configuration */
            error_counter::IErrorCountingConfigration& _config;
        };
    }
}

#endif /* LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_FDIR_HPP_ */
