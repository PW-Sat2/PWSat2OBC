#include "time.hpp"

#include "base/reader.h"
#include "logger/logger.h"
#include "telecommunication/downlink.h"

#include "comm/ITransmitFrame.hpp"

using std::uint8_t;
using gsl::span;
using devices::comm::ITransmitFrame;
using telecommunication::downlink::DownlinkFrame;
using telecommunication::downlink::DownlinkAPID;

namespace obc
{
    namespace telecommands
    {
		TimeTelecommand::TimeTelecommand(services::time::ICurrentTime& time)
		: _time (time)
		{
		}

        uint8_t TimeTelecommand::CommandCode() const
        {
            return Code;
        }

        void TimeTelecommand::Handle(ITransmitFrame& transmitter, span<const uint8_t> parameters)
        {
            Reader r(parameters);

            auto operationAsByte = r.ReadByte();
            auto operation = static_cast<TimeOperations>(operationAsByte);

            DownlinkFrame response(DownlinkAPID::TimeStatus, 0);
            auto payloadWriter = response.PayloadWriter();

            // write original operation code in response
            payloadWriter.WriteByte(operationAsByte);

            switch (operation)
            {
            case TimeOperations::ReadOnly:
            	GenerateReadbackResponse(payloadWriter, OSResult::Success, 0);
            	break;

            case TimeOperations::Time:
            	SetTime(r, payloadWriter);
            	break;

            case TimeOperations::TimeCorrection:
            	SetTimeCorrectionFactor(r, payloadWriter);
            	break;

            case TimeOperations::RTCCorrection:
            	SetRtcCorrectionFactor(r, payloadWriter);
            	break;

            default:
            	GenerateReadbackResponse(payloadWriter, OSResult::InvalidArgument, 0);
            	break;
            }

            GenerateTimeStateResponse(payloadWriter);
            transmitter.SendFrame(response.Frame());
        }

        void TimeTelecommand::SetTime(Reader& reader, Writer& responseWriter)
        {
        	auto newTime = reader.ReadQuadWordLE();
        	auto millisecons = std::chrono::milliseconds(newTime);
        	bool setTimeResult = _time.SetCurrentTime(millisecons);
        	auto result = setTimeResult ? OSResult::Success : OSResult::IOError;

        	GenerateReadbackResponse(responseWriter, result, newTime);
        }

        void TimeTelecommand::SetTimeCorrectionFactor(Reader& reader, Writer& responseWriter)
        {
        	UNREFERENCED_PARAMETER(reader);
        	GenerateReadbackResponse(responseWriter, OSResult::NotImplemented, -1);
        }

        void TimeTelecommand::SetRtcCorrectionFactor(Reader& reader, Writer& responseWriter)
        {
        	UNREFERENCED_PARAMETER(reader);
        	GenerateReadbackResponse(responseWriter, OSResult::NotImplemented, -1);
        }

        void TimeTelecommand::GenerateTimeStateResponse(Writer& payloadWriter)
        {
        	auto currentTime = _time.GetCurrentTime();
        	uint64_t milisecondsTime = currentTime.HasValue ? currentTime.Value.count() : 0;
            payloadWriter.WriteQuadWordLE(milisecondsTime);
        }

        void TimeTelecommand::GenerateReadbackResponse(Writer& responseWriter, OSResult result, uint64_t argument)
        {
        	responseWriter.WriteByte(num(result));
        	responseWriter.WriteQuadWordLE(argument);
        }
    }
}
