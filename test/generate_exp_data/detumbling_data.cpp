#include "experiment/adcs/data_point.hpp"
#include "fs/fs.h"

using namespace services::fs;
using namespace std::chrono_literals;
using experiments::fs::ExperimentFile;
using experiment::adcs::DetumblingDataPoint;
using devices::gyro::GyroscopeTelemetry;
using devices::payload::PayloadTelemetry;

void GenerateDetumblingData(IFileSystem& fs)
{
    DetumblingDataPoint point;
    point.Timestamp = 1234ms;

    point.Temperatures.supply = 0x3101;
    point.Temperatures.Xp = 0x3102;
    point.Temperatures.Xn = 0x3103;
    point.Temperatures.Yp = 0x3104;
    point.Temperatures.Yn = 0x3105;
    point.Temperatures.sads = 0x3106;
    point.Temperatures.sail = 0x3107;
    point.Temperatures.cam_nadir = 0x3108;
    point.Temperatures.cam_wing = 0x3109;

    point.Photodiodes.Xp = 0x4101;
    point.Photodiodes.Xn = 0x4102;
    point.Photodiodes.Yp = 0x4103;
    point.Photodiodes.Yn = 0x4104;

    point.Gyro = {0x1101, 0x1102, 0x1103, 0x1104};

    point.ReferenceSunS.voltages = {0x2101, 0x2102, 0x2103, 0x2104, 0x2105};

    point.Magnetometer = std::array<devices::imtq::MagnetometerMeasurement, 3>{0x5101, 0x5102, 0x5103};

    point.Dipoles = std::array<devices::imtq::Dipole, 3>{0x6101, 0x6102, 0x6103};

    ExperimentFile dataFile;

    dataFile.Open(fs, "detumbling", FileOpen::CreateAlways, FileAccess::ReadWrite);

    point.WriteTo(dataFile);

    dataFile.Close();
}
