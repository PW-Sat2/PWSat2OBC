#include <chrono>
#include <cstdint>
#include "base/BitWriter.hpp"
#include "base/reader.h"
#include "base/writer.h"
#include "fs/ExperimentFile.hpp"
#include "fs/fs.h"
#include "payload/telemetry.h"
#include "suns/suns.hpp"

#include "telemetry/BasicTelemetry.hpp"
#include "telemetry/ErrorCounters.hpp"
#include "telemetry/Experiments.hpp"
#include "telemetry/SystemStartup.hpp"
#include "telemetry/TimeTelemetry.hpp"

#include "telemetry/Telemetry.hpp"

using namespace services::fs;
using namespace std::chrono_literals;
using experiments::fs::ExperimentFile;
using namespace devices::payload;

// ---------------------------------------

static void WriteTelemetry(ExperimentFile& file)
{
    typedef telemetry::Telemetry<telemetry::ErrorCountingTelemetry, //
        telemetry::ExperimentTelemetry,                             //
        telemetry::McuTemperature,                                  //
        devices::eps::hk::ControllerATelemetry,                     //
        devices::eps::hk::ControllerBTelemetry                      //
        >
        ManagedTelemetryForPayloadExperiment;

    ManagedTelemetryForPayloadExperiment telemetry;

    std::array<uint8_t, 232> buffer;
    buffer.fill(0xEE);
    BitWriter w(buffer);

    devices::eps::hk::ControllerATelemetry epsA;
    devices::eps::hk::ControllerBTelemetry epsB;

    std::array<std::uint8_t, 72> source;
    for (uint8_t i = 0; i < 72; ++i)
        source[i] = i;
    Reader r(source);
    epsA.ReadFrom(r);
    r.Reset();
    epsB.ReadFrom(r);

    telemetry.Set(devices::eps::hk::ControllerATelemetry(epsA));
    telemetry.Set(devices::eps::hk::ControllerBTelemetry(epsB));

    telemetry::ErrorCountingTelemetry::Container counters;
    for (size_t i = 0; i < counters.size(); ++i)
    {
        counters[i] = i;
    }

    telemetry.Set(telemetry::ErrorCountingTelemetry(counters));
    telemetry.Set(telemetry::McuTemperature(3660));
    telemetry.Set(telemetry::ExperimentTelemetry(4, experiments::StartResult::Success, experiments::IterationResult::LoopImmediately));

    telemetry.Write(w);
    file.Write(ExperimentFile::PID::PayloadExperimentMainTelemetry, w.Capture());
}

static void WriteRadFetTelemetry(ExperimentFile& file)
{
    PayloadTelemetry::Radfet telemetry;

    telemetry.status = 0xE0;
    telemetry.temperature = 0xE1111111;
    telemetry.vth = {0xE2223333lu, 0xE4445555lu, 0xE6667777lu};

    std::array<uint8_t, PayloadTelemetry::Radfet::DeviceDataLength> buffer;
    Writer w(buffer);
    telemetry.Write(w);
    file.Write(ExperimentFile::PID::PayloadRadFet, buffer);
}

static void WritePayloadTemperaturesTelemetry(ExperimentFile& file)
{
    PayloadTelemetry::Temperatures telemetry;

    telemetry.supply = 0xD111;
    telemetry.Xp = 0xD222;
    telemetry.Xn = 0xD333;
    telemetry.Yp = 0xD444;
    telemetry.Yn = 0xD555;
    telemetry.sads = 0xD666;
    telemetry.sail = 0xD777;
    telemetry.cam_nadir = 0xD888;
    telemetry.cam_wing = 0xD999;

    std::array<uint8_t, PayloadTelemetry::Temperatures::DeviceDataLength> buffer;
    Writer w(buffer);
    telemetry.Write(w);

    file.Write(ExperimentFile::PID::PayloadTemperatures, buffer);
}

static void WritePayloadStatusTelemetry(ExperimentFile& file)
{
    PayloadTelemetry::Status telemetry;
    telemetry.who_am_i = 0x53;
    file.Write(ExperimentFile::PID::PayloadWhoami, gsl::make_span(&telemetry.who_am_i, 1));
}

static void WritePayloadHousekeepingTelemetry(ExperimentFile& file)
{
    PayloadTelemetry::Housekeeping telemetry;
    telemetry.int_3v3d = 0xA111;
    telemetry.obc_3v3d = 0xA222;
    std::array<uint8_t, PayloadTelemetry::Housekeeping::DeviceDataLength> buffer;
    Writer w(buffer);
    telemetry.Write(w);
    file.Write(ExperimentFile::PID::PayloadHousekeeping, buffer);
}

static void WritePayloadSunsTelemetry(ExperimentFile& file)
{
    PayloadTelemetry::SunsRef telemetry;
    telemetry.voltages = {0xB111, 0xB222, 0xB333, 0xB444, 0xB555};
    std::array<uint8_t, PayloadTelemetry::SunsRef::DeviceDataLength> buffer;
    Writer w(buffer);
    telemetry.Write(w);
    file.Write(ExperimentFile::PID::PayloadSunS, buffer);
}

static void WritePayloadPhotodiodesTelemetry(ExperimentFile& file)
{
    PayloadTelemetry::Photodiodes telemetry;

    telemetry.Xp = 0xC111;
    telemetry.Xn = 0xC222;
    telemetry.Yp = 0xC333;
    telemetry.Yn = 0xC444;

    std::array<uint8_t, PayloadTelemetry::Photodiodes::DeviceDataLength> buffer;
    Writer w(buffer);
    telemetry.Write(w);

    file.Write(ExperimentFile::PID::PayloadPhotodiodes, buffer);
}

static void WriteExperimentalSunsTelemetry(ExperimentFile& file)
{
    devices::suns::MeasurementData data;

    memset(&data, 0, sizeof(data));

    data.status = {0x3333, 0x1111, 0x2222};
    data.parameters = {0x44, 0x55};

    data.visible_light = {{
        {{0xA0A0, 0xA1A1, 0xA2A2, 0xA3A3}}, //
        {{0xB0B0, 0xB1B1, 0xB2B2, 0xB3B3}}, //
        {{0xC0C0, 0xC1C1, 0xC2C2, 0xC3C3}}, //
    }};

    data.temperature.structure = {0x6666};
    data.temperature.panels = {0x7777, 0x8888, 0x9999, 0xAAAA};

    data.infrared = {{
        {{0xAEAE, 0xAAAA, 0xABAB, 0xACAC}}, //
        {{0xBEBE, 0xBABA, 0xBBBB, 0xBCBC}}, //
        {{0xCECE, 0xCACA, 0xCBCB, 0xCCCC}}, //
    }};

    std::array<uint8_t, 67> buffer;

    Writer w(buffer);
    data.WritePrimaryData(w);
    file.Write(ExperimentFile::PID::ExperimentalSunSPrimary, w.Capture());

    w.Reset();
    data.WriteSecondaryData(w);
    file.Write(ExperimentFile::PID::ExperimentalSunSSecondary, w.Capture());
}

static void StartupStep(ExperimentFile& file)
{
    WriteTelemetry(file);
    WritePayloadStatusTelemetry(file);
    WritePayloadTemperaturesTelemetry(file);
    WritePayloadHousekeepingTelemetry(file);
    WritePayloadSunsTelemetry(file);
    WritePayloadPhotodiodesTelemetry(file);
    WriteTelemetry(file);
}

static void RadFETStep(ExperimentFile& file)
{
    WriteTelemetry(file);
    WritePayloadStatusTelemetry(file);
    WritePayloadTemperaturesTelemetry(file);
    WritePayloadHousekeepingTelemetry(file);
    WriteRadFetTelemetry(file);
    WriteRadFetTelemetry(file);
    WritePayloadStatusTelemetry(file);
    WritePayloadTemperaturesTelemetry(file);
    WritePayloadHousekeepingTelemetry(file);
    WriteRadFetTelemetry(file);
    WriteRadFetTelemetry(file);
    WriteTelemetry(file);
}

static void CamsStep(ExperimentFile& file)
{
    WriteTelemetry(file);
    WriteTelemetry(file);
    WritePayloadTemperaturesTelemetry(file);
    WriteTelemetry(file);
    WriteTelemetry(file);
    WritePayloadTemperaturesTelemetry(file);
}

static void CamsFullStep(ExperimentFile& file)
{
    for (uint8_t i = 0; i < 10; ++i)
    {
        file.Write(ExperimentFile::PID::CameraSyncCount, gsl::make_span(&i, 1));
    }

    for (uint8_t i = 0; i < 10; ++i)
    {
        file.Write(ExperimentFile::PID::CameraSyncCount, gsl::make_span(&i, 1));
    }
}

static void SunSStep(ExperimentFile& file)
{
    WriteTelemetry(file);
    WriteExperimentalSunsTelemetry(file);
    WriteTelemetry(file);
}

// --------------------------------------

void GeneratePayloadCommissioningData(IFileSystem& fs)
{
    ExperimentFile file;

    file.Open(fs, "payload", FileOpen::CreateAlways, FileAccess::ReadWrite);
    StartupStep(file);
    RadFETStep(file);
    CamsStep(file);
    CamsFullStep(file);
    SunSStep(file);
    file.Close();
}
