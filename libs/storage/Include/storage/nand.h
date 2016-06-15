#ifndef LIBS_STORAGE_NAND_H_
#define LIBS_STORAGE_NAND_H_

#include "storage.h"

typedef struct  {
	uint8_t volatile* data8;
	uint16_t volatile* data16;
	uint32_t volatile* data32;
	uint8_t volatile* addr;
	uint8_t volatile* cmd;
} FlashNANDInterface;

void BuildNANDInterface(FlashInterface* flash, FlashNANDInterface* nand);

#endif /* LIBS_STORAGE_NAND_H_ */
