#include <stddef.h>
#include <stdint.h>

#include "MemoryDriver.hpp"
#include "base/ecc.h"
#include "system.h"

#define MEMORY_SIZE (1 * 1024 * 1024)

struct DriverContext
{
    uint8_t memory[MEMORY_SIZE];
    uint8_t spare[MEMORY_SIZE / 512 * 16];
    uint8_t faultyBlocks;
};

static DriverContext context;

static inline DriverContext* Context(const FlashNANDInterface* interface)
{
    return (DriverContext*)(interface->baseAddress - offsetof(DriverContext, memory));
}

static FlashStatus ReadPage(FlashNANDInterface* interface, uint32_t address, uint8_t* buffer, uint16_t len)
{
    auto context = Context(interface);

    uint16_t blockNo = (address - (uint32_t)context->memory) / (512 * 32);

    if (context->faultyBlocks & (1 << blockNo))
    {
        return FlashStatusReadError;
    }

    memcpy(buffer, (void*)address, len);

    uint16_t pageNo = (address - (uint32_t)context->memory) / 512;

    uint8_t* spareBase = context->spare + pageNo * 16;

    uint32_t readEcc = spareBase[13] | (spareBase[14] << 8) | (spareBase[15] << 16);

    if (readEcc == 0x00FFFFFF)
    {
        return FlashStatusOK;
    }

    uint32_t generatedEcc = EccCalc(buffer, len);

    EccResult correctionResult = EccCorrect(generatedEcc, readEcc, buffer, len);

    switch (correctionResult)
    {
        case EccResultCorrected:
            return FlashStatusErrorCorrected;
        case EccResultNotCorrected:
            return FlashStatusErrorNotCorrected;
        case EccResultCorrupted:
            return FlashStatusChecksumCorrupted;
        case EccResultNoError:
        default:
            return FlashStatusOK;
    }
}

static FlashStatus ReadSpare(FlashNANDInterface* interface, uint32_t address, uint8_t* buffer, uint16_t length)
{
    auto context = Context(interface);
    uint16_t pageNo = (address - (uint32_t)context->memory) / 512;

    uint8_t* spareBase = context->spare + pageNo * 16;

    if (length <= 5)
    {
        memcpy(buffer, spareBase, length);
    }
    else
    {
        memcpy(buffer, spareBase, 5);
        memcpy(&buffer[5], &spareBase[6], length - 5);
    }

    return FlashStatusOK;
}

static FlashStatus WritePage(
    FlashNANDInterface* interface, uint8_t volatile* address, const uint8_t* buffer, uint32_t length)
{
    auto context = Context(interface);
    uint16_t pageNo = (address - context->memory) / 512;

    uint8_t* spareBase = context->spare + pageNo * 16;

    memcpy((void*)address, buffer, length);

    uint32_t ecc = EccCalc(buffer, length);

    spareBase[13] = (uint8_t)(ecc & 0xFF);
    spareBase[14] = (uint8_t)((ecc >> 8) & 0xFF);
    spareBase[15] = (uint8_t)((ecc >> 16) & 0xFF);

    return FlashStatusOK;
}

static FlashStatus WriteSpare(FlashNANDInterface* interface, uint32_t address, uint8_t* buffer, uint16_t length)
{
    auto context = Context(interface);
    uint16_t pageNo = (address - (uint32_t)context->memory) / 512;

    uint8_t* spareBase = context->spare + pageNo * 16;

    if (length <= 5)
    {
        memcpy(spareBase, buffer, length);
        return FlashStatusOK;
    }

    memcpy(spareBase, buffer, 5);
    memcpy(&spareBase[6], &buffer[5], length - 5);

    return FlashStatusOK;
}

static FlashStatus EraseBlock(FlashNANDInterface* interface, uint32_t address)
{
    auto context = Context(interface);

    uint16_t blockNo = (address - (uint32_t)context->memory) / (512 * 32);

    if (context->faultyBlocks & (1 << blockNo))
    {
        return FlashStatusWriteError;
    }

    uint32_t spareAddress = blockNo * 32 * 16;

    memset((void*)address, 0xFF, 32 * 512);
    memset((void*)spareAddress, 0xFF, 32 * 16);

    return FlashStatusOK;
}

static uint8_t CheckBadBlock(const FlashNANDInterface* interface, uint8_t volatile* address)
{
    auto context = Context(interface);

    uint16_t blockNo = (address - context->memory) / (32 * 512);

    uint8_t badBlockMark = context->spare[blockNo * 32 + 5];

    return badBlockMark != 0xFF;
}

static int Initialize(FlashNANDInterface* interface)
{
    UNREFERENCED_PARAMETER(interface);

    return FlashStatusOK;
}

static FlashStatus MarkBadBlock(const struct _FlashNANDInterface* interface, uint8_t* address)
{
    auto context = Context(interface);
    uint16_t blockNo = (address - context->memory) / (32 * 512);
    context->spare[blockNo * 32 + 5] = 0xAB;

    return FlashStatusOK;
}

void CauseBadBlock(FlashNANDInterface* interface, int block)
{
    auto context = Context(interface);
    context->faultyBlocks |= 1 << block;
}

bool IsBadBlock(FlashNANDInterface* interface, int block)
{
    auto context = Context(interface);
    uint8_t* address = &context->memory[block * 32 * 512];
    return CheckBadBlock(interface, address);
}

void SwapBit(FlashNANDInterface* interface, uint32_t byteOffset, uint8_t bitsToSwap)
{
    auto context = Context(interface);

    uint8_t b = context->memory[byteOffset];
    uint8_t c = b ^ bitsToSwap;
    context->memory[byteOffset] = c;
}

void InitializeMemoryNAND(FlashNANDInterface* flash)
{
    flash->initialize = Initialize;
    flash->writePage = WritePage;
    flash->writeSpare = WriteSpare;
    flash->readPage = ReadPage;
    flash->readSpare = ReadSpare;
    flash->eraseBlock = EraseBlock;
    flash->isBadBlock = CheckBadBlock;
    flash->markBadBlock = MarkBadBlock;
    flash->baseAddress = (uint32_t)context.memory;

    memset(context.memory, 0xFF, sizeof(context.memory));
    memset(context.spare, 0xFF, sizeof(context.spare));

    context.faultyBlocks = 0;
}
