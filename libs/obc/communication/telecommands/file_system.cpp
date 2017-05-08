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
        FileSender::FileSender(
            const char* path, uint8_t correlationId, devices::comm::ITransmitFrame& transmitter, services::fs::IFileSystem& fs)
            : _file(fs, path, services::fs::FileOpen::Existing, services::fs::FileAccess::ReadOnly), _correlationId(correlationId),
              _transmitter(transmitter)
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
            constexpr uint8_t maxFileDataSize = DownlinkFrame::MaxPayloadSize - 2;

            if (seq > this->_lastSeq)
            {
                return false;
            }

            DownlinkFrame response(DownlinkAPID::Operation, seq);

            if (OS_RESULT_FAILED(this->_file.Seek(SeekOrigin::Begin, seq * maxFileDataSize)))
            {
                return false;
            }

            response.PayloadWriter().WriteByte(_correlationId);
            response.PayloadWriter().WriteByte(static_cast<uint8_t>(DownloadFileTelecommand::ErrorCode::Success));

            auto segmentSize = std::min<std::size_t>(maxFileDataSize, this->_fileSize - seq * maxFileDataSize);

            auto buf = response.PayloadWriter().Reserve(segmentSize);

            this->_file.Read(buf);

            return this->_transmitter.SendFrame(response.Frame());
        }

        DownloadFileTelecommand::DownloadFileTelecommand(services::fs::IFileSystem& fs) : _fs(fs)
        {
        }

        void DownloadFileTelecommand::Handle(devices::comm::ITransmitFrame& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);

            auto correlationId = r.ReadByte();
            auto pathLength = r.ReadByte();
            auto pathSpan = r.ReadArray(pathLength);
            auto path = reinterpret_cast<const char*>(pathSpan.data());
            auto terminationByte = r.ReadByte();

            if (!r.Status() || terminationByte != 0)
            {
                LOG(LOG_LEVEL_ERROR, "Malformed request");
                DownlinkFrame errorResponse(DownlinkAPID::Operation, 0);
                errorResponse.PayloadWriter().WriteByte(correlationId);
                errorResponse.PayloadWriter().WriteByte(static_cast<uint8_t>(DownloadFileTelecommand::ErrorCode::MalformedRequest));
                errorResponse.PayloadWriter().WriteByte(0);

                transmitter.SendFrame(errorResponse.Frame());

                return;
            }

            LOGF(LOG_LEVEL_INFO, "Sending file %s", path);

            FileSender sender(path, correlationId, transmitter, this->_fs);

            if (!sender.IsValid())
            {
                LOG(LOG_LEVEL_ERROR, "Unable to open requested file");
                DownlinkFrame errorResponse(DownlinkAPID::Operation, 0);
                errorResponse.PayloadWriter().WriteByte(correlationId);
                errorResponse.PayloadWriter().WriteByte(static_cast<uint8_t>(DownloadFileTelecommand::ErrorCode::FileNotFound));
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

        RemoveFileTelecommand::RemoveFileTelecommand(services::fs::IFileSystem& fs) : _fs(fs)
        {
        }

        void RemoveFileTelecommand::Handle(devices::comm::ITransmitFrame& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);

            auto correlationId = r.ReadByte();
            auto pathLength = r.ReadByte();
            auto pathSpan = r.ReadArray(pathLength);
            auto path = reinterpret_cast<const char*>(pathSpan.data());
            auto terminationByte = r.ReadByte();

            DownlinkFrame response(DownlinkAPID::Operation, 0);
            response.PayloadWriter().WriteByte(correlationId);

            if (!r.Status() || terminationByte != 0)
            {
                LOG(LOG_LEVEL_ERROR, "Malformed request");
                response.PayloadWriter().WriteByte(static_cast<uint8_t>(OSResult::InvalidArgument));
                response.PayloadWriter().WriteByte(0);
                transmitter.SendFrame(response.Frame());
                return;
            }

            LOGF(LOG_LEVEL_INFO, "Removing file %s", path);

            auto unlinkResult = this->_fs.Unlink(path);

            if (OS_RESULT_FAILED(unlinkResult))
            {
                LOGF(LOG_LEVEL_ERROR, "Unable to unlink file %s, error %d", path, static_cast<int>(unlinkResult));
            }
            else
            {
                LOGF(LOG_LEVEL_INFO, "File unlinked %s", path);
            }

            response.PayloadWriter().WriteByte(static_cast<uint8_t>(unlinkResult));
            response.PayloadWriter().WriteArray(pathSpan);

            transmitter.SendFrame(response.Frame());
        }
    }
}
