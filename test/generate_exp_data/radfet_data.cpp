#include <chrono>
#include <cstdint>
#include "base/writer.h"
#include "fs/ExperimentFile.hpp"
#include "fs/fs.h"
#include "payload/telemetry.h"

using namespace services::fs;
using namespace std::chrono_literals;
using experiments::fs::ExperimentFile;
using namespace devices::payload;

static void SaveTimestamp(experiments::fs::ExperimentFile& file, std::chrono::milliseconds timestamp)
{
    std::array<std::uint8_t, 8> buffer;
    Writer writer(buffer);
    writer.WriteQuadWordLE(timestamp.count());
    file.Write(ExperimentFile::PID::Timestamp, writer.Capture());
}

static void SaveRadFETRegisters(ExperimentFile& file, PayloadTelemetry::Radfet& telemetry)
{
    std::array<std::uint8_t, 17> buffer;
    Writer writer(buffer);
    telemetry.Write(writer);
    file.Write(ExperimentFile::PID::PayloadRadFet, writer.Capture());
}

static void SaveTemperatures(ExperimentFile& file, PayloadTelemetry::Temperatures& telemetry)
{
    std::array<std::uint8_t, 4> buffer;
    Writer writer(buffer);
    writer.WriteWordLE(telemetry.supply);
    writer.WriteWordLE(telemetry.sads);
    file.Write(ExperimentFile::PID::SupplySADSTemperature, writer.Capture());
}

static void SaveRadFETTelemetry(ExperimentFile& file, PayloadTelemetry::Radfet& telemetry, std::chrono::milliseconds timestamp)
{
    SaveTimestamp(file, timestamp);
    SaveRadFETRegisters(file, telemetry);
}

static void SaveLoopTelemetry(ExperimentFile& file,
    PayloadTelemetry::Radfet& radfetTelemetry,
    PayloadTelemetry::Temperatures& temeraturesTelemetry,
    std::chrono::milliseconds timestamp)
{
    SaveRadFETTelemetry(file, radfetTelemetry, timestamp);
    SaveTemperatures(file, temeraturesTelemetry);
}

static void RedfetStart(ExperimentFile& file)
{
    PayloadTelemetry::Radfet telemetry;
    telemetry.status = 0x10;
    telemetry.temperature = 0x11111111;
    telemetry.vth = {0x12223333lu, 0x14445555lu, 0x16667777lu};
    SaveRadFETTelemetry(file, telemetry, 0s);
}

static void RedfetIteration(ExperimentFile& file, int iteration)
{
    PayloadTelemetry::Radfet telemetry;
    telemetry.status = 0x20;
    telemetry.temperature = 0x21111111 + iteration;
    telemetry.vth = {0x22223333lu + iteration, 0x24445555lu + iteration, 0x26667777lu + iteration};

    PayloadTelemetry::Temperatures temperatures;
    temperatures.supply = 0xD111 + iteration;
    temperatures.sads = 0xD666 + iteration;

    SaveLoopTelemetry(file, telemetry, temperatures, iteration * 10s);
}

void RedfetStop(ExperimentFile& file)
{
    PayloadTelemetry::Radfet telemetry;
    telemetry.status = 0xF0;
    telemetry.temperature = 0xF1111111;
    telemetry.vth = {0xF2223333lu, 0xF4445555lu, 0xF6667777lu};
    SaveRadFETTelemetry(file, telemetry, 2min);
}

// --------------------------------------

void GenerateRadfetData(IFileSystem& fs)
{
    ExperimentFile file;

    file.Open(fs, "radfet", FileOpen::CreateAlways, FileAccess::ReadWrite);
    RedfetStart(file);

    for (int i = 0; i < 20; ++i)
    {
        RedfetIteration(file, i);
    }

    RedfetStop(file);
    file.Close();
}
