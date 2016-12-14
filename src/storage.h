#ifndef SRC_STORAGE_H_
#define SRC_STORAGE_H_

#include "n25q/yaffs.h"
#include "spi/spi.h"
#include "storage/nand_driver.h"
#include "utils.h"
#include "yaffs_guts.h"

/**
 * @defgroup obc_storage OBC storage
 * @ingroup obc
 *
 * Hardware devices and file system drivers
 *
 * @{
 */

/**
 * @brief OBC storage
 */
struct OBCStorage final
{
    /**
     * @brief Constructs @ref OBCStorage instance
     * @param[in] spi SPI interface used by external memories
     */
    OBCStorage(drivers::spi::ISPIInterface& spi);

    /** @brief Initializes OBC storage */
    void Initialize();

#ifdef USE_EXTERNAL_FLASH
    /** @brief N25Q flash driver */
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

/** @} */

#endif /* SRC_STORAGE_H_ */
