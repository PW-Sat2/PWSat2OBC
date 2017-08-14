#include <chrono>
#include <cstdint>
#include "base/BitWriter.hpp"
#include "base/writer.h"
#include "fs/ExperimentFile.hpp"
#include "fs/fs.h"
#include "gyro/telemetry.hpp"

using namespace services::fs;
using namespace std::chrono_literals;
using experiments::fs::ExperimentFile;

static bool Save(const std::chrono::milliseconds& time, ExperimentFile& file)
{
    std::array<std::uint8_t, 8> buffer;
    Writer writer{buffer};
    writer.WriteQuadWordLE(time.count());
    if (!writer.Status())
    {
        return false;
    }

    return OS_RESULT_SUCCEEDED(file.Write(experiments::fs::ExperimentFile::PID::Timestamp, writer.Capture()));
}

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

static bool Save(bool sailIndicator, std::uint16_t sailTemperature, ExperimentFile& file)
{
    std::array<std::uint8_t, 3> buffer;
    Writer writer{buffer};
    writer.WriteWordLE(sailTemperature);
    writer.WriteByte(sailIndicator ? 1 : 0);
    if (!writer.Status())
    {
        return false;
    }

    return OS_RESULT_SUCCEEDED(file.Write(experiments::fs::ExperimentFile::PID::Sail, writer.Capture()));
}

void GenerateSailData(IFileSystem& fs)
{
    ExperimentFile primary;

    primary.Open(fs, "sail", FileOpen::CreateAlways, FileAccess::ReadWrite);
    Save(11223344ms, primary);
    Save(true, 5566, primary);
    Save(devices::gyro::GyroscopeTelemetry{7788, 1234, 5678, 4321}, primary);
    primary.Close();
}
