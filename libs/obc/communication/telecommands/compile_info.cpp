#include "compile_info.hpp"
#include "base/reader.h"
#include "comm/ITransmitter.hpp"
#include "logger/logger.h"
#include "system.h"
#include "telecommunication/downlink.h"
#include "version.h"

using telecommunication::downlink::DownlinkFrame;
using telecommunication::downlink::DownlinkAPID;

namespace obc
{
    namespace telecommands
    {
        void CompileInfoTelecommand::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> /*parameters*/)
        {
            const auto span = gsl::ensure_z(VERSION);
            Reader reader(gsl::make_span(reinterpret_cast<const std::uint8_t*>(span.data()), span.size()));
            int seq = 0;
            while (reader.RemainingSize() > 0)
            {
                DownlinkFrame response(DownlinkAPID::CompileInfo, seq);
                const auto size = std::min(reader.RemainingSize(), static_cast<std::int32_t>(decltype(response)::MaxPayloadSize));
                const auto span = reader.ReadArray(size);
                auto& writer = response.PayloadWriter();
                writer.WriteArray(span);
                transmitter.SendFrame(response.Frame());
                ++seq;
            }
        }
    }
}
