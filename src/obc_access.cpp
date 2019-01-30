#include "obc_access.hpp"
#include "obc.h"

Terminal& GetTerminal()
{
    return Main.terminal;
}

// AntennaDriver& GetAntennaDriver()
// {
//     return Main.Hardware.antennaDriver;
// }

boot::BootSettings& GetBootSettings()
{
    return Main.BootSettings;
}

devices::comm::CommObject& GetCommDriver()
{
    return Main.Hardware.CommDriver;
}

obc::PersistentStorageAccess& GetPersistentStorageAccess()
{
    return Main.Hardware.PersistentStorage;
}

services::fs::YaffsFileSystem& GetFileSystem()
{
    return Main.fs;
}

obc::FDIR& GetFDIR()
{
    return Main.Fdir;
}

devices::eps::EPSDriver& GetEPS()
{
    return Main.Hardware.EPS;
}

devices::imtq::ImtqDriver& GetIMTQ()
{
    return Main.Hardware.Imtq;
}

devices::suns::SunSDriver& GetSUNS()
{
    return Main.Hardware.SunS;
}

obc::OBCExperiments& GetExperiments()
{
    return Main.Experiments;
}

obc::OBCMemory& GetMemory()
{
    return Main.Memory;
}

drivers::i2c::I2CInterface& GetI2C()
{
    return Main.Hardware.I2C.Buses;
}

devices::gyro::GyroDriver& GetGyro()
{
    return Main.Hardware.Gyro;
}

devices::rtc::RTCObject& GetRTC()
{
    return Main.Hardware.rtc;
}

services::time::TimeProvider& GetTimeProvider()
{
    return Main.timeProvider;
}

obc::OBCScrubbing& GetScrubbing()
{
    return Main.Scrubbing;
}
temp::ADCTemperatureReader& GetMCUTemperature()
{
    return Main.Hardware.MCUTemperature;
}

devices::camera::Camera& GetCamera()
{
    return Main.camera;
}

obc::Adcs& GetAdcs()
{
    return Main.adcs;
}
