#ifndef LIBS_STORAGE_NAND_H_
#define LIBS_STORAGE_NAND_H_

#include "storage.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _FlashNANDInterface
{
    void* context;
    uint8_t volatile* data8;
    uint16_t volatile* data16;
    uint32_t volatile* data32;
    uint8_t volatile* addr;
    uint8_t volatile* cmd;
    int (*initialize)(struct _FlashNANDInterface* flash);
    int (*check)(struct _FlashNANDInterface* flash);
    int (*status)(struct _FlashNANDInterface* flash);

    FlashStatus (*eraseBlock)(struct _FlashNANDInterface* interface, uint32_t offset);

    FlashStatus (*readPage)(struct _FlashNANDInterface* interface, uint32_t offset, uint8_t* buffer, uint16_t length);

    FlashStatus (*readSpare)(struct _FlashNANDInterface* interface, uint32_t offset, uint8_t* buffer, uint16_t length);

    FlashStatus (*writePage)(
        struct _FlashNANDInterface* interface, uint32_t offset, const uint8_t* buffer, uint32_t length);

    FlashStatus (*writeSpare)(
        struct _FlashNANDInterface* interface, uint32_t offset, const uint8_t* buffer, uint16_t length);

    uint8_t (*isBadBlock)(struct _FlashNANDInterface* flash, uint32_t offset);
    FlashStatus (*markBadBlock)(struct _FlashNANDInterface* flash, uint32_t offset);
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
    uint32_t baseOffset;
} NANDGeometry;

typedef struct
{
    uint32_t offset;
    uint32_t dataSize;
    uint8_t* dataBuffer;
    uint32_t spareSize;
    uint8_t* spareBuffer;
} NANDOperationSlice;

typedef struct
{
    uint32_t offset;
    uint32_t dataSize;
    uint8_t* dataBuffer;
    uint32_t spareSize;
    uint8_t* spareBuffer;
} NANDOperation;

void NANDCalculateGeometry(NANDGeometry* geometry);
uint32_t NANDPageOffsetFromChunk(NANDGeometry* geometry, uint16_t chunkNo);
uint32_t NANDBlockOffset(NANDGeometry* geometry, uint16_t blockNo);
uint16_t NANDAffectedPagesCount(NANDGeometry* geometry, NANDOperation* operation);
NANDOperationSlice NANDGetOperationSlice(NANDGeometry* geometry, NANDOperation* operation, uint16_t pageNo);

#ifdef __cplusplus
}
#endif

#endif /* LIBS_STORAGE_NAND_H_ */
