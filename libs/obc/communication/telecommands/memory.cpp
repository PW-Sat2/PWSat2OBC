#include "memory.hpp"
#include <algorithm>
#include "base/gcc_workaround.hpp"
#include "base/reader.h"
#include "comm/ITransmitter.hpp"
#include "telecommunication/downlink.h"
#include "telecommunication/telecommand_handling.h"

namespace obc
{
    namespace telecommands
    {
        using telecommunication::downlink::CorrelatedDownlinkFrame;
        using telecommunication::downlink::DownlinkAPID;

        void ReadMemoryTelecommand::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);
            auto correlationId = r.ReadByte();
            auto offset = r.ReadDoubleWordLE();
            auto size = r.ReadDoubleWordLE();

            size = std::min(size, std::numeric_limits<std::size_t>::max() - offset);

            auto memoryToRead = reinterpret_cast<std::uint8_t*>(offset);

            if (!r.Status())
            {
                CorrelatedDownlinkFrame frame(DownlinkAPID::Operation, 0, correlationId);
                frame.PayloadWriter().WriteByte(1);

                transmitter.SendFrame(frame.Frame());
                return;
            }

            auto seq = 0;

            while (size > 0)
            {
                auto partSize = std::min<std::size_t>(size, CorrelatedDownlinkFrame::MaxPayloadSize);
                CorrelatedDownlinkFrame frame(DownlinkAPID::MemoryContent, seq, correlationId);

                if (memoryToRead == nullptr)
                {
                    frame.PayloadWriter().WriteByte(gcc_workaround::ReadByte0());
                    memoryToRead++;
                    partSize--;
                    size--;
                }
                auto part = gsl::make_span(memoryToRead, memoryToRead + partSize);

                frame.PayloadWriter().WriteArray(part);

                transmitter.SendFrame(frame.Frame());

                size -= partSize;
                memoryToRead += partSize;
                seq++;
            }
        }
    }
}
