#include "nand.h"

void NANDCalculateGeometry(NANDGeometry* geometry)
{
    geometry->chunkSize = geometry->pageSize * geometry->pagesPerChunk;
    geometry->blockSize = geometry->pageSize * geometry->pagesPerBlock;
    geometry->chunksPerBlock = geometry->pagesPerBlock / geometry->pagesPerChunk;
}

uint32_t NANDPageBaseAddressFromChunk(NANDGeometry* geometry, uint16_t chunkNo)
{
    return geometry->baseAddress + chunkNo * geometry->chunkSize;
}

uint32_t NANDBlockBaseAddress(NANDGeometry* geometry, uint16_t blockNo)
{
    return geometry->baseAddress + blockNo * geometry->blockSize;
}
