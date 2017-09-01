#include "compile_info.hpp"
#include "base/reader.h"
#include "comm/ITransmitter.hpp"
#include "logger/logger.h"
#include "system.h"
#include "telecommunication/downlink.h"
#include "version.h"

using telecommunication::downlink::CorrelatedDownlinkFrame;
using telecommunication::downlink::DownlinkAPID;

namespace obc
{
    namespace telecommands
    {
        void CompileInfoTelecommand::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);

            auto correlationId = r.ReadByte();
            CorrelatedDownlinkFrame response(DownlinkAPID::Operation, 0, correlationId);
            auto& writer = response.PayloadWriter();
            if (!r.Status())
            {
                LOG(LOG_LEVEL_ERROR, "Malformed compile info request");
                writer.WriteByte(1);
            }
            else
            {
                constexpr std::uint8_t array[] = VERSION;
                static_assert(count_of(array) <= CorrelatedDownlinkFrame::MaxPayloadSize, "Version information is too long");
                writer.WriteByte(0);
                writer.WriteArray(array);
            }

            transmitter.SendFrame(response.Frame());
        }
    }
}
