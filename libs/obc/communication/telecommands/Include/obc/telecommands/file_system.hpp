#ifndef LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_FILE_SYSTEM_HPP_
#define LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_FILE_SYSTEM_HPP_

#include "fs/fs.h"
#include "telecommunication/telecommand_handling.h"

namespace obc
{
    namespace telecommands
    {
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
