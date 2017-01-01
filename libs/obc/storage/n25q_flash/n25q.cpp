#include "n25q.h"
#include "fs/fs.h"

using obc::storage::N25QStorage;
using devices::n25q::OperationResult;
using services::fs::YaffsFileSystem;

N25QStorage::N25QStorage(drivers::spi::ISPIInterface& spi, YaffsFileSystem& fs)
    : ExternalFlashDriver(spi),                //
      ExternalFlash("/", ExternalFlashDriver), //
      _fs(fs)
{
}

void N25QStorage::Initialize()
{
    if (OS_RESULT_FAILED(this->ExternalFlash.Mount()))
    {
        return;
    }
}

OSResult N25QStorage::ClearStorage()
{
    return this->_fs.ClearDevice(this->ExternalFlash.Device());
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
