#include <chrono>
#include <cstdint>
#include "base/BitWriter.hpp"
#include "fs/ExperimentFile.hpp"
#include "fs/fs.h"
#include "gyro/telemetry.hpp"

using namespace services::fs;
using namespace std::chrono_literals;
using experiments::fs::ExperimentFile;

static bool Save(const devices::gyro::GyroscopeTelemetry& gyroTelemetry, ExperimentFile& file)
{
    std::array<std::uint8_t, (devices::gyro::GyroscopeTelemetry::BitSize() + 7) / 8> buffer;
    BitWriter writer{buffer};
    gyroTelemetry.Write(writer);
    if (!writer.Status())
    {
        return false;
    }
    return OS_RESULT_SUCCEEDED(file.Write(experiments::fs::ExperimentFile::PID::Gyro, writer.Capture()));
}

void GenerateLeopData(IFileSystem& fs)
{
    ExperimentFile primary;

    primary.Open(fs, "leop", FileOpen::CreateAlways, FileAccess::ReadWrite);

    // 1h of gathering samples with 1 Hz is 3600 samples
    for (uint16_t i = 0; i < 3600; ++i)
    {
        Save(devices::gyro::GyroscopeTelemetry{static_cast<int16_t>(i * 2),
                 static_cast<int16_t>(-(i * 2)),
                 static_cast<int16_t>(i * 2 + 1),
                 static_cast<int16_t>(-(i * 2 + 1))},
            primary);
    }

    primary.Close();
}
