#include <gsl/span>
#include "fs/ExperimentFile.hpp"
#include "fs/fs.h"

using namespace services::fs;
using namespace std::chrono_literals;
using experiments::fs::ExperimentFile;

void WriteCameraCommissioningData(ExperimentFile& file)
{
    uint8_t syncCount[] = {2,
        6,
        8,
        14,
        60,
        1,
        255,
        4,
        12,
        6,

        60,
        20,
        40,
        255,
        5,
        10,
        15,
        20,
        25,
        30};

    for (uint32_t i = 0; i < count_of(syncCount); ++i)
    {
        file.Write(ExperimentFile::PID::CameraSyncCount, gsl::make_span(&syncCount[i], 1));
    }
}

void GenerateCameraCommissioningData(IFileSystem& fs)
{
    ExperimentFile file;

    file.Open(fs, "camera", FileOpen::CreateAlways, FileAccess::ReadWrite);

    WriteCameraCommissioningData(file);

    file.Close();
}
