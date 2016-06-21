#ifndef UNIT_TESTS_FILESYSTEM_MEMORYDRIVER_HPP_
#define UNIT_TESTS_FILESYSTEM_MEMORYDRIVER_HPP_

#include "yaffs_guts.h"

void InitializeYaffsDev(yaffs_dev* dev);
void CauseBadBlock(yaffs_dev* dev, int block);
bool IsBadBlock(yaffs_dev* dev, int block);

#endif /* UNIT_TESTS_FILESYSTEM_MEMORYDRIVER_HPP_ */
