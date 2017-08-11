#include "camera_types.h"
#include "base/reader.h"

using namespace devices::camera;

PictureData::PictureData() : type(CameraPictureType::Enum::Invalid), dataLength(0)
{
}

bool PictureData::Parse(gsl::span<const uint8_t> command)
{
    Reader reader(command);

    auto prefix = reader.ReadByte();
    auto commandCode = (CameraCmd)reader.ReadByte();

    if (prefix != CommandPrefix || commandCode != CameraCmd::Data)
    {
        return false;
    }

    type = CameraPictureType::GetPictureType(reader.ReadByte());
    dataLength = reader.ReadWordAndHalfLE();

    return true;
}
