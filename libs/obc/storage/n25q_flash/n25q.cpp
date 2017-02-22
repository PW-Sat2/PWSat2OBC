#include "n25q.h"
#include "fs/fs.h"
#include "gpio/gpio.h"

using obc::storage::N25QStorage;
using devices::n25q::OperationResult;
using services::fs::IYaffsDeviceOperations;
using drivers::gpio::OutputPin;
using obc::storage::SingleFlash;

SingleFlash::SingleFlash(const char* mountPoint,
    const drivers::gpio::Pin& slaveSelect,
    drivers::spi::EFMSPIInterface& spi,
    services::fs::IYaffsDeviceOperations& deviceOperations)
    :                                     //
      SPI(spi, slaveSelect),              //
      Driver(SPI),                        //
      Device(mountPoint, Driver),         //
      _deviceOperations(deviceOperations) //
{
}

N25QStorage::N25QStorage(drivers::spi::EFMSPIInterface& spi, IYaffsDeviceOperations& deviceOperations, obc::OBCGPIO& pins)
    : //
      _flashes{
          {"/a", pins.Flash1ChipSelect, spi, deviceOperations}, //
          {"/b", pins.Flash2ChipSelect, spi, deviceOperations}, //
          {"/c", pins.Flash3ChipSelect, spi, deviceOperations}  //
      }

{
}

OSResult SingleFlash::Initialize()
{
    if (this->Driver.Reset() != OperationResult::Success)
    {
        return OSResult::DeviceNotFound;
    }

    return this->Device.Mount(this->_deviceOperations);
}

OSResult SingleFlash::ClearStorage()
{
    return this->_deviceOperations.ClearDevice(this->Device.Device());
}

OSResult SingleFlash::Erase()
{
    auto r = this->Driver.EraseChip();
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

OSResult N25QStorage::Initialize()
{
    OSResult result = OSResult::Success;

    for (auto& f : this->_flashes)
    {
        auto r = f.Initialize();

        if (OS_RESULT_FAILED(r))
        {
            result = r;
        }
    }

    return result;
}

OSResult N25QStorage::ClearStorage()
{
    OSResult result = OSResult::Success;

    for (auto& f : this->_flashes)
    {
        auto r = f.ClearStorage();

        if (OS_RESULT_FAILED(r))
        {
            result = r;
        }
    }

    return result;
}

OSResult N25QStorage::Erase()
{
    OSResult result = OSResult::Success;

    for (auto& f : this->_flashes)
    {
        auto r = f.Erase();

        if (OS_RESULT_FAILED(r))
        {
            result = r;
        }
    }

    return result;
}
