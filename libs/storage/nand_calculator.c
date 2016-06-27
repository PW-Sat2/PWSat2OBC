#include "nand.h"
#include <math.h>

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

uint16_t NANDAffectedPagesCount(NANDGeometry* geometry, NANDOperation* operation)
{
    uint16_t dataPages = operation->dataSize / geometry->pageSize;
    if (geometry->pageSize * dataPages < operation->dataSize)
    {
        dataPages++;
    }

    uint16_t sparePages = operation->spareSize / geometry->spareAreaPerPage;
    if (geometry->spareAreaPerPage * sparePages < operation->spareSize)
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

NANDOperationSlice NANDGetOperationSlice(NANDGeometry* geometry, NANDOperation* operation, uint16_t pageNo)
{
    NANDOperationSlice slice;

    slice.baseAddress = operation->baseAddress + pageNo * geometry->pageSize;

    int32_t remaining = operation->dataSize - pageNo * geometry->pageSize;
    if (remaining > 0)
    {
        slice.dataBuffer = operation->dataBuffer + pageNo * geometry->pageSize;
        slice.dataSize = operation->dataSize - pageNo * geometry->pageSize;
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

    remaining = operation->spareSize - pageNo * geometry->spareAreaPerPage;

    if (remaining > 0)
    {
        slice.spareBuffer = operation->spareBuffer + pageNo * geometry->spareAreaPerPage;
        slice.spareSize = operation->spareSize - pageNo * geometry->spareAreaPerPage;
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
