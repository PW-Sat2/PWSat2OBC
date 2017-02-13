#ifndef LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_FILE_SYSTEM_HPP_
#define LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_FILE_SYSTEM_HPP_

#include "fs/fs.h"
#include "telecommunication/downlink.h"
#include "telecommunication/telecommand_handling.h"

namespace obc
{
    namespace telecommands
    {
        class FileSender final
        {
          public:
            FileSender(const char* path,
                telecommunication::downlink::DownlinkAPID apid,
                devices::comm::ITransmitFrame& transmitter,
                services::fs::IFileSystem& fs);
            bool IsValid();
            bool SendPart(std::uint32_t seq);

          private:
            services::fs::File _file;
            telecommunication::downlink::DownlinkAPID _apid;
            devices::comm::ITransmitFrame& _transmitter;
            services::fs::FileSize _fileSize;
            std::uint32_t _lastSeq;
        };

        class DownladFileTelecommand final : public telecommunication::uplink::IHandleTeleCommand
        {
          public:
            DownladFileTelecommand(services::fs::IFileSystem& fs) : _fs(fs)
            {
            }

            virtual std::uint8_t CommandCode() const override;

            virtual void Handle(devices::comm::ITransmitFrame& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            services::fs::IFileSystem& _fs;
        };
    }
}

#endif /* LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_FILE_SYSTEM_HPP_ */
