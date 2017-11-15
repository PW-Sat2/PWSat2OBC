#include <cstdint>
#include <cstring>
#include "obc_access.hpp"
#include "temp/efm.hpp"
#include "terminal/terminal.h"

void GetTemperature()
{
    auto temp = GetMCUTemperature().ReadCelsius();

    GetTerminal().Printf("Temp: %d\n", temp);
}

void GetRawTemperature()
{
    auto temp = GetMCUTemperature().ReadRaw();

    GetTerminal().Printf("Temperature RAW: %d\n", temp.Value());
}

void GetCalibrationValues()
{
    auto calibration_temperature = GetMCUTemperature().GetCalibrationTemperature();
    auto calibration_result = GetMCUTemperature().GetADCResultAtCalibrationTemperature();

    GetTerminal().Printf("Calibration Temperature: %d\nCalibration Result: %d\n", calibration_temperature, calibration_result);
}

void Temp(std::uint16_t argc, char* argv[])
{
    if (argc == 1 && strcmp(argv[0], "calibration") == 0)
    {
        GetCalibrationValues();
    }
    else if (argc == 1 && strcmp(argv[0], "raw") == 0)
    {
        GetRawTemperature();
    }
    else
    {
        GetTemperature();
    }
}
