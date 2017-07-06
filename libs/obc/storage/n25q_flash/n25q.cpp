#include "n25q.h"
#include "fs/fs.h"
#include "gpio/gpio.h"

using obc::storage::N25QStorage;
using devices::n25q::OperationResult;
using services::fs::IYaffsDeviceOperations;
using drivers::gpio::OutputPin;
using namespace obc::storage::error_counters;

N25QStorage::N25QStorage(                     //
    error_counter::ErrorCounting& errors,     //
    drivers::spi::EFMSPIInterface& spi,       //
    IYaffsDeviceOperations& deviceOperations, //
    obc::OBCGPIO& pins                        //
    )
    :                                                                    //
      _deviceOperations(deviceOperations),                               //
      _spiSlaves{                                                        //
          {spi, pins.Flash1ChipSelect},                                  //
          {spi, pins.Flash2ChipSelect},                                  //
          {spi, pins.Flash3ChipSelect}},                                 //
      _n25qDrivers{                                                      //
          {errors, N25QDriver1::ErrorCounter::DeviceId, _spiSlaves[0]},  //
          {errors, N25QDriver2::ErrorCounter::DeviceId, _spiSlaves[1]},  //
          {errors, N25QDriver3::ErrorCounter::DeviceId, _spiSlaves[2]}}, //
      _driver{{&_n25qDrivers[0], &_n25qDrivers[1], &_n25qDrivers[2]}},   //
      Device("/", _driver)                                               //
{
}

OSResult N25QStorage::Initialize()
{
    if (this->_driver.Reset() != OperationResult::Success)
    {
        return OSResult::DeviceNotFound;
    }

    return this->Device.Mount(this->_deviceOperations);
}

OSResult N25QStorage::ClearStorage()
{
    return _deviceOperations.ClearDevice(this->Device.Device());
}

OSResult N25QStorage::Erase()
{
    auto r = this->_driver.EraseChip();
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

devices::n25q::N25QDriver& N25QStorage::GetDriver(uint8_t index)
{
    return _n25qDrivers[index];
}
