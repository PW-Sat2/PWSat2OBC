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
            const auto count = reader.ReadByte();
            const auto path = reader.ReadString(30);

            CorrelatedDownlinkFrame frame{DownlinkAPID::Operation, 0, corelationId};
            auto& writer = frame.PayloadWriter();
            if (!reader.Status() || path.empty() || count >= 30)
            {
                writer.WriteByte(0x2);
            }
            else
            {
                this->_photoService.Reset();
                this->_photoService.EnableCamera(cameraId);
                for (std::uint8_t cx = 0; cx < count; ++cx)
                {
                    this->_photoService.TakePhoto(cameraId, resolution);
                    this->_photoService.DownloadPhoto(cameraId, 0);
                    this->_photoService.SavePhoto(0, "%.*s_%d", path.size(), path.data(), 0);
                }

                this->_photoService.DisableCamera(cameraId);
                this->_photoService.Reset();
                writer.WriteByte(0);
            }

            transmitter.SendFrame(frame.Frame());
        }
    }
}
