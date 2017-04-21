#include "time.hpp"

#include "base/reader.h"
#include "logger/logger.h"
#include "telecommunication/downlink.h"

#include "base/os.h"

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

            auto operation = static_cast<TimeOperations>(r.ReadByte());

            DownlinkFrame response(DownlinkAPID::TimeStatus, 0);
            auto payloadWriter = response.PayloadWriter();

            switch (operation)
            {
            case TimeOperations::ReadOnly:
            	payloadWriter.WriteByte(num(OSResult::Success));
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
            	payloadWriter.WriteByte(num(OSResult::InvalidArgument));
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

        	responseWriter.WriteByte(num(result));
        }

        void TimeTelecommand::SetTimeCorrectionFactor(Reader& reader, Writer& responseWriter)
        {
        	UNREFERENCED_PARAMETER(reader);
        	responseWriter.WriteByte(num(OSResult::NotImplemented));
        }

        void TimeTelecommand::SetRtcCorrectionFactor(Reader& reader, Writer& responseWriter)
        {
        	UNREFERENCED_PARAMETER(reader);
        	responseWriter.WriteByte(num(OSResult::NotImplemented));
        }

        void TimeTelecommand::GenerateTimeStateResponse(Writer& payloadWriter)
        {
        	auto currentTime = _time.GetCurrentTime();
        	uint64_t milisecondsTime = currentTime.HasValue ? currentTime.Value.count() : 0;
            payloadWriter.WriteQuadWordLE(milisecondsTime);
        }
    }
}
