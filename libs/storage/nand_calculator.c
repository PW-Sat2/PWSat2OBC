#include "nand.h"
#include <math.h>

void NANDCalculateGeometry(NANDGeometry* geometry)
{
    geometry->chunkSize = geometry->pageSize * geometry->pagesPerChunk;
    geometry->blockSize = geometry->pageSize * geometry->pagesPerBlock;
    geometry->chunksPerBlock = geometry->pagesPerBlock / geometry->pagesPerChunk;
}

uint32_t NANDPageOffsetFromChunk(NANDGeometry* const geometry, uint16_t chunkNo)
{
    return geometry->baseOffset + chunkNo * geometry->chunkSize;
}

uint32_t NANDBlockOffset(NANDGeometry* const geometry, uint16_t blockNo)
{
    return geometry->baseOffset + blockNo * geometry->blockSize;
}

uint16_t NANDAffectedPagesCount(NANDGeometry* const geometry, NANDOperation* operation)
{
    uint32_t dataPages = operation->dataSize / geometry->pageSize;
    if ((uint32_t)(geometry->pageSize * dataPages) < operation->dataSize)
    {
        dataPages++;
    }

    uint32_t sparePages = operation->spareSize / geometry->spareAreaPerPage;
    if ((uint32_t)(geometry->spareAreaPerPage * sparePages) < operation->spareSize)
    {
        sparePages++;
    }

    if (dataPages > sparePages)
    {
        return dataPages;
    }
    else
    {
        return sparePages;
    }
}

NANDOperationSlice NANDGetOperationSlice(NANDGeometry* const geometry, NANDOperation* operation, uint16_t pageNo)
{
    NANDOperationSlice slice;

    const uint32_t pageOffset = pageNo * geometry->pageSize;

    slice.offset = operation->offset + pageOffset;

    int32_t remaining = operation->dataSize - pageOffset;
    if (remaining > 0)
    {
        slice.dataBuffer = operation->dataBuffer + pageOffset;
        slice.dataSize = operation->dataSize - pageOffset;
        if (slice.dataSize > geometry->pageSize)
        {
            slice.dataSize = geometry->pageSize;
        }
    }
    else
    {
        slice.dataBuffer = NULL;
        slice.dataSize = 0;
    }

    const uint32_t spareOffset = pageNo * geometry->spareAreaPerPage;

    remaining = operation->spareSize - spareOffset;

    if (remaining > 0)
    {
        slice.spareBuffer = operation->spareBuffer + spareOffset;
        slice.spareSize = operation->spareSize - spareOffset;
        if (slice.spareSize > geometry->spareAreaPerPage)
        {
            slice.spareSize = geometry->spareAreaPerPage;
        }
    }
    else
    {
        slice.spareBuffer = NULL;
        slice.spareSize = 0;
    }

    return slice;
}
