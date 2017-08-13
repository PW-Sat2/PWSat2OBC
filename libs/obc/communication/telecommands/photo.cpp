#include "photo.hpp"
#include "base/reader.h"
#include "comm/ITransmitter.hpp"
#include "photo/photo_service.hpp"

namespace obc
{
    namespace telecommands
    {
        using telecommunication::downlink::CorrelatedDownlinkFrame;
        using telecommunication::downlink::DownlinkAPID;

        TakePhoto::TakePhoto(services::photo::IPhotoService& photoService) : _photoService(photoService)
        {
        }

        void TakePhoto::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            char filePath[40];
            if (parameters.size() == 0)
            {
                CorrelatedDownlinkFrame response(DownlinkAPID::Operation, 0, 0);
                response.PayloadWriter().WriteByte(0x1);
                transmitter.SendFrame(response.Frame());
                return;
            }

            Reader reader(parameters);
            const auto corelationId = reader.ReadByte();
            const auto cameraId = static_cast<services::photo::Camera>(reader.ReadByte());
            const auto resolution = static_cast<services::photo::PhotoResolution>(reader.ReadByte());
            const auto path = reader.ReadString(count_of(filePath));

            CorrelatedDownlinkFrame frame{DownlinkAPID::Operation, 0, corelationId};
            auto& writer = frame.PayloadWriter();
            if (!reader.Status() || path.empty())
            {
                writer.WriteByte(0x2);
            }
            else
            {
                memcpy(filePath, path.data(), path.size());
                filePath[count_of(filePath) - 1] = '\0';
                this->_photoService.Schedule(services::photo::Reset());
                this->_photoService.Schedule(services::photo::EnableCamera(cameraId));
                this->_photoService.Schedule(services::photo::TakePhoto(cameraId, resolution));
                this->_photoService.Schedule(services::photo::DownloadPhoto(cameraId, 0));
                this->_photoService.Schedule(services::photo::SavePhoto(0, "%.*s", path.size(), path.data()));
                this->_photoService.Schedule(services::photo::DisableCamera(cameraId));
                this->_photoService.Schedule(services::photo::Reset());
                writer.WriteByte(0);
            }

            transmitter.SendFrame(frame.Frame());
        }
    }
}
