#include "devices.h"

#include "commands/RadFET.h"
#include "commands/base_code.hpp"

using namespace drivers::payload;

PayloadDeviceDriver::PayloadDeviceDriver(IPayloadDriver& driver) : _driver(driver)
{
}

OSResult PayloadDeviceDriver::MeasureSunSRef()
{
    return OSResult::NotImplemented;
}

OSResult PayloadDeviceDriver::MeasureTemperatures()
{
    return OSResult::NotImplemented;
}

OSResult PayloadDeviceDriver::MeasurePhotodiodes()
{
    return OSResult::NotImplemented;
}

OSResult PayloadDeviceDriver::MeasureHousekeeping()
{
    return OSResult::NotImplemented;
}

OSResult PayloadDeviceDriver::MeasureRadFET()
{
    commands::RadFETCommand command(_driver);
    return command.Execute();
}
