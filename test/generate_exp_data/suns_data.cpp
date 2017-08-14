#include "experiment/suns/suns.hpp"
#include "fs/fs.h"

using namespace services::fs;
using namespace std::chrono_literals;
using experiments::fs::ExperimentFile;

void GenerateSunSData(IFileSystem& fs)
{
    experiment::suns::DataPoint data;

    memset(&data, 0, sizeof(data));

    data.Timestamp = 1234ms;

    data.ExperimentalSunS.status = {0x3333, 0x1111, 0x2222};
    data.ExperimentalSunS.parameters = {0x44, 0x55};

    data.ExperimentalSunS.visible_light = {{
        {{0xA0A0, 0xA1A1, 0xA2A2, 0xA3A3}}, //
        {{0xB0B0, 0xB1B1, 0xB2B2, 0xB3B3}}, //
        {{0xC0C0, 0xC1C1, 0xC2C2, 0xC3C3}}, //
    }};

    data.ExperimentalSunS.temperature.structure = {0x6666};
    data.ExperimentalSunS.temperature.panels = {0x7777, 0x8888, 0x9999, 0xAAAA};

    data.ExperimentalSunS.infrared = {{
        {{0xAEAE, 0xAAAA, 0xABAB, 0xACAC}}, //
        {{0xBEBE, 0xBABA, 0xBBBB, 0xBCBC}}, //
        {{0xCECE, 0xCACA, 0xCBCB, 0xCCCC}}, //
    }};

    data.ReferenceSunS.voltages = {{0xE0E0, 0xE1E1, 0xE2E2, 0xE3E3, 0xE4E4}};
    data.Gyro = {0x0F1F, 0x2F3F, 0x4F5F, 0x6F7F};

    ExperimentFile primary;

    primary.Open(fs, "suns_primary", FileOpen::CreateAlways, FileAccess::ReadWrite);

    data.WritePrimaryDataSetTo(primary);

    primary.Close();

    ExperimentFile secondary;

    secondary.Open(fs, "suns_secondary", FileOpen::CreateAlways, FileAccess::ReadWrite);

    data.WriteSecondaryDataSetTo(secondary);

    secondary.Close();
}
