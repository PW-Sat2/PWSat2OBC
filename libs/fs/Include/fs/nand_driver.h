#ifndef LIBS_FS_INCLUDE_FS_NAND_DRIVER_H_
#define LIBS_FS_INCLUDE_FS_NAND_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <storage/nand.h>
#include <yaffs_guts.h>

/**
 * @defgroup yaffs-nand YAFFS driver for NAND flash.
 *
 * @brief This library provides YAFFS driver for NAND flash memory
 *
 * @{
 */

/**
 * @brief Description of YAFFS driver for NAND flash.
 */
typedef struct
{
    /**
     * @brief NAND flash interface
     */
    FlashNANDInterface flash;
    /**
     * @brief Description of geometry of NAND flash
     */
    NANDGeometry geometry;
} YaffsNANDDriver;

/**
 * Configures YAFFS device with given YAFFS NAND driver
 * @param[in] dev YAFFS device to configure
 * @param[in] driver Lower level driver for flash memory
 */
void SetupYaffsNANDDriver(struct yaffs_dev* dev, YaffsNANDDriver* driver);

/** @}*/

#ifdef __cplusplus
}
#endif

#endif /* LIBS_FS_INCLUDE_FS_NAND_DRIVER_H_ */
