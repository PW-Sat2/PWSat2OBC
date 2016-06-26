#ifndef LIBS_STORAGE_NAND_H_
#define LIBS_STORAGE_NAND_H_

#include "storage.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _FlashNANDInterface
{
    uint32_t baseAddress;
    uint8_t volatile* data8;
    uint16_t volatile* data16;
    uint32_t volatile* data32;
    uint8_t volatile* addr;
    uint8_t volatile* cmd;
    int (*initialize)(struct _FlashNANDInterface* flash);
    int (*check)(struct _FlashNANDInterface* flash);
    int (*status)(struct _FlashNANDInterface* flash);
    FlashStatus (*readPage)(struct _FlashNANDInterface* interface, uint32_t address, uint8_t* buffer, uint16_t len);
    FlashStatus (*eraseBlock)(struct _FlashNANDInterface* interface, uint32_t address);
    int (*writeBlock)(struct _FlashNANDInterface* interface, uint32_t address, uint8_t value);
    FlashStatus (*writePage)(
        struct _FlashNANDInterface* interface, uint8_t volatile* address, const uint8_t* buffer, uint32_t length);
    uint8_t (*isBadBlock)(const struct _FlashNANDInterface* flash, uint8_t volatile* address);
} FlashNANDInterface;

void BuildNANDInterface(FlashNANDInterface* flash);

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
