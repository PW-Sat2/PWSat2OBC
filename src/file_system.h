#ifndef SRC_FILE_SYSTEM_H_
#define SRC_FILE_SYSTEM_H_

#include "n25q/yaffs.h"
#include "spi/spi.h"
#include "utils.h"
#include "yaffs_guts.h"

struct OBCFileSystem final
{
    OBCFileSystem(drivers::spi::ISPIInterface& spi);

    void Initialize();

#ifdef USE_EXTERNAL_FLASH
    devices::n25q::N25QDriver ExternalFlashDriver;

    /** @brief N25Q Yaffs device */
    devices::n25q::N25QYaffsDevice<devices::n25q::BlockMapping::Sector, 512_Bytes, 16_MB> ExternalFlash;
#else
    /** Yaffs root device */
    struct yaffs_dev rootDevice;
    /** Driver for yaffs root device */
    YaffsNANDDriver rootDeviceDriver;
#endif
};

#endif /* SRC_FILE_SYSTEM_H_ */
