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
        FileSender::FileSender(const char* path,
            telecommunication::downlink::DownlinkAPID apid,
            devices::comm::ITransmitFrame& transmitter,
            services::fs::IFileSystem& fs)
            : _file(fs, path, services::fs::FileOpen::Existing, services::fs::FileAccess::ReadOnly), _apid(apid), _transmitter(transmitter)
        {
            if (this->IsValid())
            {
                this->_fileSize = this->_file.Size();

                this->_lastSeq = static_cast<std::uint32_t>(std::ceil(this->_fileSize / static_cast<float>(DownlinkFrame::MaxPayloadSize)));
            }
        }

        bool FileSender::IsValid()
        {
            return this->_file;
        }

        bool FileSender::SendPart(std::uint32_t seq)
        {
            if (seq > this->_lastSeq)
            {
                return false;
            }

            DownlinkFrame response(static_cast<DownlinkAPID>(this->_apid), seq);

            if (OS_RESULT_FAILED(this->_file.Seek(SeekOrigin::Begin, seq * DownlinkFrame::MaxPayloadSize)))
            {
                return false;
            }

            auto segmentSize = std::min<std::size_t>(DownlinkFrame::MaxPayloadSize, this->_fileSize - seq * DownlinkFrame::MaxPayloadSize);

            auto buf = response.PayloadWriter().Reserve(segmentSize);

            this->_file.Read(buf);

            return this->_transmitter.SendFrame(response.Frame());
        }

        DownloadFileTelecommand::DownloadFileTelecommand(services::fs::IFileSystem& fs) : _fs(fs)
        {
        }

        std::uint8_t DownloadFileTelecommand::CommandCode() const
        {
            return 0xAB;
        }

        void DownloadFileTelecommand::Handle(devices::comm::ITransmitFrame& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);

            auto downlinkApid = r.ReadByte();
            auto pathLength = r.ReadByte();
            auto pathSpan = r.ReadArray(pathLength);
            auto path = reinterpret_cast<const char*>(pathSpan.data());
            r.ReadByte();

            LOGF(LOG_LEVEL_INFO, "Sending file %s", path);

            FileSender sender(path, static_cast<DownlinkAPID>(downlinkApid), transmitter, this->_fs);

            if (!sender.IsValid())
            {
                LOG(LOG_LEVEL_ERROR, "Unable to open requested file");
                DownlinkFrame errorResponse(DownlinkAPID::FileNotFound, 0);
                errorResponse.PayloadWriter().WriteArray(pathSpan);

                transmitter.SendFrame(errorResponse.Frame());

                return;
            }

            while (true)
            {
                auto seq = r.ReadDoubleWordLE();

                if (!r.Status())
                {
                    break;
                }

                sender.SendPart(seq);
            }
        }
    }
}
