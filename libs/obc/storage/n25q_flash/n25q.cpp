#include "n25q.h"
#include "fs/fs.h"
#include "gpio/gpio.h"

using obc::storage::N25QStorage;
using devices::n25q::OperationResult;
using services::fs::IYaffsDeviceOperations;
using drivers::gpio::OutputPin;

N25QStorage::N25QStorage(drivers::spi::EFMSPIInterface& spi, IYaffsDeviceOperations& deviceOperations, obc::OBCGPIO& pins)
    : ExternalFlashDriverSPI(spi, pins.Flash1ChipSelect), //
      ExternalFlashDriver(ExternalFlashDriverSPI),            //
      ExternalFlash("/", ExternalFlashDriver),                //
      _deviceOperations(deviceOperations)
{
}

OSResult N25QStorage::Initialize()
{
    if (this->ExternalFlashDriver.Reset() != OperationResult::Success)
    {
        return OSResult::DeviceNotFound;
    }

    return this->ExternalFlash.Mount(this->_deviceOperations);
}

OSResult N25QStorage::ClearStorage()
{
    return this->_deviceOperations.ClearDevice(this->ExternalFlash.Device());
}

OSResult N25QStorage::Erase()
{
    auto r = this->ExternalFlashDriver.EraseChip();
    switch (r)
    {
        case OperationResult::Success:
            return OSResult::Success;
        case OperationResult::Timeout:
            return OSResult::Timeout;
        case OperationResult::Failure:
        default:
            return OSResult::IOError;
    }
}
