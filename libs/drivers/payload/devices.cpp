#include "devices.h"

#include "commands/Housekeeping.h"
#include "commands/Photodiodes.h"
#include "commands/RadFET.h"
#include "commands/RadFET_code.hpp"
#include "commands/SunS.h"
#include "commands/Temperatures.h"
#include "commands/Whoami.h"
#include "commands/base_code.hpp"

using namespace drivers::payload;

PayloadDeviceDriver::PayloadDeviceDriver(IPayloadDriver& driver) : _driver(driver)
{
}

OSResult PayloadDeviceDriver::GetWhoami(PayloadTelemetry::Status& output)
{
    commands::WhoamiCommand command(_driver);
    return command.Execute(output);
}

bool PayloadDeviceDriver::ValidateWhoami(const PayloadTelemetry::Status& data) const
{
    return commands::WhoamiCommand::Validate(data);
}

OSResult PayloadDeviceDriver::MeasureSunSRef(PayloadTelemetry::SunsRef& output)
{
    commands::SunSCommand command(_driver);
    return command.Execute(output);
}

OSResult PayloadDeviceDriver::MeasureTemperatures(PayloadTelemetry::Temperatures& output)
{
    commands::TemperaturesCommand command(_driver);
    return command.Execute(output);
}

OSResult PayloadDeviceDriver::MeasurePhotodiodes(PayloadTelemetry::Photodiodes& output)
{
    commands::PhotodiodesCommand command(_driver);
    return command.Execute(output);
}

OSResult PayloadDeviceDriver::MeasureHousekeeping(PayloadTelemetry::Housekeeping& output)
{
    commands::HousekeepingCommand command(_driver);
    return command.Execute(output);
}

OSResult PayloadDeviceDriver::RadFETOn(PayloadTelemetry::Radfet& output)
{
    commands::RadFETOnCommand command(_driver);
    return command.Execute(output);
}

OSResult PayloadDeviceDriver::MeasureRadFET(PayloadTelemetry::Radfet& output)
{
    commands::RadFETMeasureCommand command(_driver);
    return command.Execute(output);
}

OSResult PayloadDeviceDriver::RadFETOff(PayloadTelemetry::Radfet& output)
{
    commands::RadFETOffCommand command(_driver);
    return command.Execute(output);
}
