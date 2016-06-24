#ifndef LIBS_STORAGE_NAND_H_
#define LIBS_STORAGE_NAND_H_

#include "storage.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint8_t volatile* data8;
    uint16_t volatile* data16;
    uint32_t volatile* data32;
    uint8_t volatile* addr;
    uint8_t volatile* cmd;
} FlashNANDInterface;

void BuildNANDInterface(FlashInterface* flash, FlashNANDInterface* nand);

typedef struct
{
    uint16_t pageSize;
    uint8_t pagesPerBlock;
    uint8_t spareAreaPerPage;
    uint8_t pagesPerChunk;
    uint32_t chunkSize;
    uint32_t blockSize;
    uint32_t chunksPerBlock;
    uint32_t baseAddress;
} NANDGeometry;

void NANDCalculateGeometry(NANDGeometry* geometry);
uint32_t NANDPageBaseAddressFromChunk(NANDGeometry* geometry, uint16_t chunkNo);
uint32_t NANDBlockBaseAddress(NANDGeometry* geometry, uint16_t blockNo);

#ifdef __cplusplus
}
#endif

#endif /* LIBS_STORAGE_NAND_H_ */
