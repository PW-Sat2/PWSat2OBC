#include "n25q.h"
#include "fs/fs.h"
#include "gpio/gpio.h"

using obc::storage::N25QStorage;
using devices::n25q::OperationResult;
using drivers::gpio::OutputPin;

N25QStorage::N25QStorage(drivers::spi::EFMSPIInterface& spi, FileSystem& fs, obc::OBCGPIO& pins)
    : ExternalFlashDriverSPI(spi, pins.SlaveSelectFlash1), //
      ExternalFlashDriver(ExternalFlashDriverSPI),         //
      ExternalFlash("/", ExternalFlashDriver),             //
      _fs(fs)
{
}

void N25QStorage::Initialize()
{
    this->ExternalFlashDriver.Reset();

    if (OS_RESULT_FAILED(this->ExternalFlash.Mount()))
    {
        return;
    }
}

OSResult N25QStorage::ClearStorage()
{
    return this->_fs.ClearDevice(&this->_fs, this->ExternalFlash.Device());
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
