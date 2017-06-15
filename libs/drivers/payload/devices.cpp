#include "devices.h"

#include "commands/Housekeeping.h"
#include "commands/Photodiodes.h"
#include "commands/RadFET.h"
#include "commands/SunS.h"
#include "commands/Temperatures.h"
#include "commands/Whoami.h"
#include "commands/base_code.hpp"

using namespace drivers::payload;

PayloadDeviceDriver::PayloadDeviceDriver(IPayloadDriver& driver) : _driver(driver)
{
}

OSResult PayloadDeviceDriver::GetWhoami()
{
    commands::WhoamiCommand command(_driver);
    return command.Execute();
}

OSResult PayloadDeviceDriver::MeasureSunSRef()
{
    commands::SunSCommand command(_driver);
    return command.Execute();
}

OSResult PayloadDeviceDriver::MeasureTemperatures()
{
    commands::TemperaturesCommand command(_driver);
    return command.Execute();
}

OSResult PayloadDeviceDriver::MeasurePhotodiodes()
{
    commands::PhotodiodesCommand command(_driver);
    return command.Execute();
}

OSResult PayloadDeviceDriver::MeasureHousekeeping()
{
    commands::HousekeepingCommand command(_driver);
    return command.Execute();
}

OSResult PayloadDeviceDriver::MeasureRadFET()
{
    commands::RadFETCommand command(_driver);
    return command.Execute();
}
