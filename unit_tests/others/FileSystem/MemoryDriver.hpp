#ifndef UNIT_TESTS_FILESYSTEM_MEMORYDRIVER_HPP_
#define UNIT_TESTS_FILESYSTEM_MEMORYDRIVER_HPP_

#include "storage/nand.h"
#include "yaffs_guts.h"

void InitializeMemoryNAND(FlashNANDInterface* flash);

void CauseBadBlock(FlashNANDInterface* flash, int block);
bool IsBadBlock(FlashNANDInterface* flash, int block);
void SwapBit(FlashNANDInterface* flash, uint32_t byteOffset, uint8_t bitsToSwap);

#endif /* UNIT_TESTS_FILESYSTEM_MEMORYDRIVER_HPP_ */
