#ifndef LIBS_FS_INCLUDE_FS_NAND_DRIVER_H_
#define LIBS_FS_INCLUDE_FS_NAND_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <storage/nand.h>
#include <yaffs_guts.h>

typedef struct
{
    FlashNANDInterface flash;
    NANDGeometry geometry;
} YaffsNANDDriver;

void SetupYaffsNANDDriver(struct yaffs_dev* dev, YaffsNANDDriver* driver);

#ifdef __cplusplus
}
#endif

#endif /* LIBS_FS_INCLUDE_FS_NAND_DRIVER_H_ */
