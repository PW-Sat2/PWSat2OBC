#ifndef LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_TIME_HPP_
#define LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_TIME_HPP_

#include "telecommunication/telecommand_handling.h"
#include "base/writer.h"
#include "base/reader.h"

#include "time/ICurrentTime.hpp"

namespace obc
{
    namespace telecommands
    {
        /**
         * @brief Telecommand for time manipulation
         * @ingroup telecommands
         * @telecommand
         *
         * Command code: 0xAB
         *
         * Parameters:
         *  - 8-bit - Specific option, see TimeOperations.
         *  - variable length field:
         *  	- TimeOperations::ReadOnly 	- none
         *  	- TimeOperations::Time  	- 64-bit LE - New mission time.
         *
         * Response: Full internal Time status
         *  - 64-bit LE - Current mission time.
         */
        class TimeTelecommand final : public telecommunication::uplink::IHandleTeleCommand
        {
          public:
            /**
             * @brief Ctor
             * @param fs File system
             */
        	TimeTelecommand(services::time::ICurrentTime& time);

            virtual void Handle(devices::comm::ITransmitFrame& transmitter, gsl::span<const std::uint8_t> parameters) override;

            virtual std::uint8_t CommandCode() const override;

            /** @brief Command code */
            static constexpr std::uint8_t Code = 0x54; // 'T'

            enum class TimeOperations : std::uint8_t
            {
            	ReadOnly = 0,
                Time = 1,        //!< New Mission Time is set
                TimeCorrection,  //!< Time Correction is set
                RTCCorrection,   //!< RTC Correction is set
            };

           private:

            /** @brief Current time provider */
            services::time::ICurrentTime& _time;

            void GenerateTimeStateResponse(Writer& responseWriter);

            void SetTime(Reader& reader, Writer& responseWriter);
            void SetTimeCorrectionFactor(Reader& reader, Writer& responseWriter);
            void SetRtcCorrectionFactor(Reader& reader, Writer& responseWriterr);
        };
    }
}

#endif /* LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_TIME_HPP_ */
