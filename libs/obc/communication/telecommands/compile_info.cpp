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
            constexpr std::uint8_t array[] = VERSION;
            DownlinkFrame response(DownlinkAPID::CompileInfo, 0);
            static_assert(count_of(array) <= decltype(response)::MaxPayloadSize, "Version information is too long");
            auto& writer = response.PayloadWriter();
            writer.WriteArray(array);
            transmitter.SendFrame(response.Frame());
        }
    }
}
