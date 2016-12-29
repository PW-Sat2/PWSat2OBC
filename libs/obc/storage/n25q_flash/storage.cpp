#include "obc/storage.h"
#include "fs/fs.h"

using obc::OBCStorage;
using devices::n25q::OperationResult;

OBCStorage::OBCStorage(drivers::spi::ISPIInterface& spi, FileSystem& fs)
    : ExternalFlashDriver(spi),                //
      ExternalFlash("/", ExternalFlashDriver), //
      _fs(fs)
{
}

void OBCStorage::Initialize()
{
    if (OS_RESULT_FAILED(this->ExternalFlash.Mount()))
    {
        return;
    }
}

OSResult OBCStorage::ClearStorage()
{
    return this->_fs.ClearDevice(&this->_fs, this->ExternalFlash.Device());
}

OSResult OBCStorage::Erase()
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
