#ifndef CAMERA_TEST_COMMANDS_HPP_
#define CAMERA_TEST_COMMANDS_HPP_

#include <array>

#include <gsl/span>

#include "camera/camera_types.h"

using std::array;
using gsl::span;

namespace commands
{
    using namespace devices::camera;

    using Command = std::array<uint8_t, 6>;

    template <CameraCmd CommandID> static constexpr Command Ack{0xAA, 0x0E, static_cast<uint8_t>(CommandID), 0x00, 0x00, 0x00};

    template <CameraCmd CommandID, uint8_t PackageIDByte0, uint8_t PackageIDByte1>
    static constexpr Command AckPackage{0xAA, 0x0E, static_cast<uint8_t>(CommandID), 0x00, PackageIDByte0, PackageIDByte1};

    static constexpr Command Sync{0xAA, 0x0D, 0x00, 0x00, 0x00, 0x00};
    static constexpr Command Reset{0xAA, 0x08, 0x00, 0x00, 0x00, 0xFF};
    static constexpr Command Invalid{0xAA, 0xFF, 0x00, 0x00, 0x00, 0x00};

    template <CameraJPEGResolution resolution>
    static constexpr Command Init{0xAA, 0x01, 0x00, 0x07, 0x07, static_cast<uint8_t>(resolution)};

    template <CameraPictureType::Enum type> static constexpr Command GetPicture{0xAA, 0x04, static_cast<uint8_t>(type), 0x00, 0x00, 0x00};

    template <CameraPictureType::Enum type, uint8_t lengthLow, uint8_t lengthMid, uint8_t lengthHigh>
    static constexpr Command Data{0xAA, 0x0A, static_cast<uint8_t>(type), lengthLow, lengthMid, lengthHigh};

    template <CameraSnapshotType type> static constexpr Command Snapshot{0xAA, 0x05, static_cast<uint8_t>(type), 0x00, 0x00, 0x00};

    template <uint8_t packageSizeLow, uint8_t packageSizeHigh>
    static constexpr Command SetPackageSize{0xAA, 0x06, 0x08, packageSizeLow, packageSizeHigh, 0x00};

    template <uint8_t firstDivider, uint8_t secondDivider>
    static constexpr Command SetBaudRate{0xAA, 0x07, firstDivider, secondDivider, 0x00, 0x00};
}

#endif // CAMERA_TEST_COMMANDS_HPP_
