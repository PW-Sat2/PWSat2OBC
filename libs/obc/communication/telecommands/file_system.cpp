#include "file_system.hpp"
#include <cmath>
#include "base/reader.h"
#include "comm/ITransmitFrame.hpp"
#include "fs/fs.h"
#include "logger/logger.h"
#include "system.h"
#include "telecommunication/downlink.h"

using telecommunication::downlink::DownlinkFrame;
using telecommunication::downlink::DownlinkAPID;
using services::fs::File;
using services::fs::SeekOrigin;

namespace obc
{
    namespace telecommands
    {
        std::uint8_t DownladFileTelecommand::CommandCode() const
        {
            return 0xAB;
        }

        void DownladFileTelecommand::Handle(devices::comm::ITransmitFrame& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);

            auto downlinkApid = r.ReadByte();
            auto pathLength = r.ReadByte();
            auto pathSpan = r.ReadArray(pathLength);
            auto path = reinterpret_cast<const char*>(pathSpan.data());
            r.ReadByte();

            LOGF(LOG_LEVEL_INFO, "Sending file %s", path);

            File f(this->_fs, path, services::fs::FileOpen::Existing, services::fs::FileAccess::ReadOnly);

            if (!f)
            {
                LOG(LOG_LEVEL_ERROR, "Unable to open requested file");
                DownlinkFrame errorResponse(DownlinkAPID::FileNotFound, 0);
                errorResponse.PayloadWriter().WriteArray(pathSpan);

                transmitter.SendFrame(errorResponse.Frame());

                return;
            }

            auto fileSize = f.Size();

            auto lastSeq = static_cast<std::uint32_t>(std::ceil(f.Size() / static_cast<float>(DownlinkFrame::MaxPayloadSize)));

            while (true)
            {
                auto seq = r.ReadDoubleWordLE();

                if (!r.Status())
                {
                    break;
                }

                if (seq > lastSeq)
                {
                    continue;
                }

                DownlinkFrame response(static_cast<DownlinkAPID>(downlinkApid), seq);

                if (OS_RESULT_FAILED(f.Seek(SeekOrigin::Begin, seq * DownlinkFrame::MaxPayloadSize)))
                {
                    continue;
                }

                auto segmentSize = std::min<std::size_t>(DownlinkFrame::MaxPayloadSize, fileSize - seq * DownlinkFrame::MaxPayloadSize);

                auto buf = response.PayloadWriter().Reserve(segmentSize);

                f.Read(buf);

                transmitter.SendFrame(response.Frame());
            }
        }
    }
}
