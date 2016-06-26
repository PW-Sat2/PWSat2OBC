#ifndef LIBS_FS_INCLUDE_FS_NAND_DRIVER_H_
#define LIBS_FS_INCLUDE_FS_NAND_DRIVER_H_

#include <storage/nand.h>
#include <yaffs_guts.h>

typedef struct
{
    FlashNANDInterface flash;
    NANDGeometry geometry;
} YaffsNANDDriver;

void SetupYaffsNANDDriver(struct yaffs_dev* dev, YaffsNANDDriver* driver);

#endif /* LIBS_FS_INCLUDE_FS_NAND_DRIVER_H_ */
