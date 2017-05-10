#ifndef LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_TIMETELECOMMAND_HPP_
#define LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_TIMETELECOMMAND_HPP_

#include "base/os.h"
#include "base/reader.h"
#include "base/writer.h"
#include "telecommunication/telecommand_handling.h"

#include "state/time/ITimeCorrectionProvider.hpp"
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
         * Command code: 0x54
         *
         * Parameters:
         *  - 8-bit - Specific option, see TimeOperations.
         *  - variable length field:
         *  	- TimeOperations::ReadOnly 	- none
         *  	- TimeOperations::Time  	- 64-bit LE - New mission time.
         *
         * Response:
         * Part 1: Status and readback
         *  - 8-bit - Specific option, see TimeOperations.
         *  - 8-bit - status result
         *  - 64-bit LE - input argument expanded to 64-bit LE.
         * Part 2: Time telemetry
         *  - 64-bit LE - Current mission time.
         */
        class TimeTelecommand final : public telecommunication::uplink::Telecommand<0x54>
        {
          public:
            /**
             * @brief Ctor
             * @param fs File system
             */
            TimeTelecommand(services::time::ICurrentTime& time, state::ITimeCorrectionProvider& _timeCorrection);

            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

            /** @brief Operations allowed by Time Telecommand */
            enum class TimeOperations : std::uint8_t
            {
                ReadOnly = 0,
                Time = 1,       //!< New Mission Time is set
                TimeCorrection, //!< Time Correction is set
                RTCCorrection,  //!< RTC Correction is set
            };

          private:
            /** @brief Current time provider */
            services::time::ICurrentTime& _time;
            state::ITimeCorrectionProvider& _timeCorrection;

            void GenerateTimeStateResponse(Writer& responseWriter);
            void GenerateReadbackResponse(Writer& responseWriter, OSResult result, uint64_t argument);

            void SetTime(Reader& reader, Writer& responseWriter);
            void SetTimeCorrectionFactor(Reader& reader, Writer& responseWriter);
            void SetRtcCorrectionFactor(Reader& reader, Writer& responseWriterr);
        };
    }
}

#endif /* LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_TIMETELECOMMAND_HPP_ */
