#include <stddef.h>
#include <stdint.h>

#include "MemoryDriver.hpp"
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

    return FlashStatusOK;
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
    //    if (pageNo == 32)
    {
        //        printf("Diff=0x%X\n", address - (uint32_t)context->memory);
        //        printf("RD[%d]=", pageNo);
        //        for (uint8_t i = 0; i < length; i++)
        //        {
        //            printf("%.2X", buffer[i]);
        //        }
        //        printf("\n");
    }
    return FlashStatusOK;
}

static FlashStatus WritePage(
    FlashNANDInterface* interface, uint8_t volatile* address, const uint8_t* buffer, uint32_t length)
{
    UNREFERENCED_PARAMETER(interface);

    memcpy((void*)address, buffer, length);

    return FlashStatusOK;
}

static FlashStatus WriteSpare(FlashNANDInterface* interface, uint32_t address, uint8_t* buffer, uint16_t length)
{
    auto context = Context(interface);
    uint16_t pageNo = (address - (uint32_t)context->memory) / 512;

    uint8_t* spareBase = context->spare + pageNo * 16;
    //    if (pageNo == 32)
    {
        //        printf("WR[%d]=", pageNo);
        //        for (uint8_t i = 0; i < length; i++)
        //        {
        //            printf("%.2X", buffer[i]);
        //        }
        //        printf("\n");
    }
    uint8_t b = spareBase[16];

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

    printf("Erasing block %d\n", blockNo);

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

    printf("Swaping bit in page %d\n", byteOffset / 512);

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
