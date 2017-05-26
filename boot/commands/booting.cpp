#include <array>
#include <cstdint>
#include "boot.h"
#include "boot/params.hpp"
#include "bsp/bsp_boot.h"
#include "bsp/bsp_uart.h"
#include "redundancy.hpp"

using std::uint8_t;
using std::uint32_t;

void BootUpper()
{
    BSP_UART_Puts(BSP_UART_DEBUG, "\n\nBooting to upper half ");

    boot::BootReason = boot::Reason::BootToUpper;
    boot::Index = 8;

    BootToAddress(BOOT_APPLICATION_BASE);
}

void BootToSelected()
{
    auto index = USART_Rx(BSP_UART_DEBUG);
    if (index > BOOT_TABLE_SIZE)
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "\n\nError: Boot index out of bounds!");
        return;
    }

    BSP_UART_Printf<60>(BSP_UART_DEBUG, "\nBooting index: %d", index);

    boot::BootReason = boot::Reason::SelectedIndex;

    BootToAddress(LoadApplication(index));
}

static void ProgramBlock(gsl::span<std::uint32_t> data, std::uint32_t* base)
{
    MSC->LOCK = MSC_UNLOCK_CODE;
    MSC->WRITECTRL |= MSC_WRITECTRL_WREN | MSC_WRITECTRL_RWWEN;

    // erase page
    MSC->ADDRB = reinterpret_cast<std::uint32_t>(base);
    MSC->WRITECMD = MSC_WRITECMD_LADDRIM;
    MSC->WRITECMD = MSC_WRITECMD_ERASEPAGE;

    while ((MSC->STATUS & MSC_STATUS_BUSY))
        ;

    // program page
    MSC->ADDRB = reinterpret_cast<std::uint32_t>(base);
    MSC->WRITECMD = MSC_WRITECMD_LADDRIM;

    MSC->WDATA = data[0];
    MSC->WRITECMD = MSC_WRITECMD_WRITETRIG;

    for (auto i = data.begin() + 1; i != data.end(); i++)
    {
        while (!(MSC->STATUS & MSC_STATUS_WDATAREADY))
            ;

        MSC->WDATA = *i;
    }

    while (!(MSC->STATUS & MSC_STATUS_WDATAREADY))
        ;

    MSC->WRITECMD = MSC_WRITECMD_WRITEEND;
}

void TMRBoot()
{
    std::array<std::uint8_t, 3> slots{1, 2, 3};
    BSP_UART_Printf<30>(BSP_UART_DEBUG, "\nTMR boot on slots %d, %d and %d", slots[0], slots[1], slots[2]);

    std::array<uint32_t, 3> lengths{
        BOOT_getLen(slots[0]), BOOT_getLen(slots[1]), BOOT_getLen(slots[2]),
    };

    auto length = redundancy::Correct(lengths[0], lengths[1], lengths[2]);

    BSP_UART_Printf<30>(BSP_UART_DEBUG, "\nProgram length: %ld", length);

    using ChunkType = uint32_t;
    constexpr std::size_t PageSize = 4_KB;
    constexpr std::size_t ChunksCount = PageSize / sizeof(ChunkType);

    std::array<std::uint64_t, 3> programOffsets{
        BOOT_getOffsetProgram(slots[0]), BOOT_getOffsetProgram(slots[1]), BOOT_getOffsetProgram(slots[2]),
    };

    std::array<ChunkType*, 3> programBases{
        reinterpret_cast<ChunkType*>(BOOT_TABLE_BASE + programOffsets[0]),
        reinterpret_cast<ChunkType*>(BOOT_TABLE_BASE + programOffsets[1]),
        reinterpret_cast<ChunkType*>(BOOT_TABLE_BASE + programOffsets[2]),
    };

    BSP_UART_Printf<30>(BSP_UART_DEBUG, "\nBase: %p", (void*)programBases[0]);
    BSP_UART_Printf<30>(BSP_UART_DEBUG, "\nBase: %p", (void*)programBases[1]);
    BSP_UART_Printf<30>(BSP_UART_DEBUG, "\nBase: %p", (void*)programBases[2]);

    auto internalFlashBase = reinterpret_cast<ChunkType*>(BOOT_APPLICATION_BASE);

    std::array<ChunkType, ChunksCount> pageBuffer;

    for (auto offset = 0U; offset < length / sizeof(ChunkType); offset += ChunksCount)
    {
        std::array<gsl::span<ChunkType, ChunksCount>, 3> pageSpans{
            gsl::make_span(programBases[0] + offset, ChunksCount), //
            gsl::make_span(programBases[1] + offset, ChunksCount), //
            gsl::make_span(programBases[2] + offset, ChunksCount), //
        };

        auto ptr = internalFlashBase + offset;

        auto internalFlashPage = gsl::make_span(ptr, ChunksCount);

        redundancy::CorrectBuffer<ChunkType>(pageBuffer, pageSpans[0], pageSpans[1], pageSpans[2]);

        bool internalOk = internalFlashPage == gsl::make_span(pageBuffer);

        if (!internalOk)
        {
            BSP_UART_Printf<30>(BSP_UART_DEBUG, "\nPage %p: NOT OK", (void*)internalFlashPage.data());
        }

        if (!internalOk)
        {
            ProgramBlock(pageBuffer, internalFlashPage.data());
        }
    }

    BSP_UART_Puts(BSP_UART_DEBUG, "\nDone");

    BootToAddress(BOOT_APPLICATION_BASE);
}

void SetRunlevel()
{
    BSP_UART_Puts(BSP_UART_DEBUG, "\nRunlevel: ");

    std::uint8_t runlevel = BSP_UART_rxByte(BSP_UART_DEBUG);

    if (runlevel > 2)
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "Invalid runlevel\n");
        return;
    }

    boot::RequestedRunlevel = static_cast<boot::Runlevel>(runlevel);
    BSP_UART_Printf<20>(BSP_UART_DEBUG, "\nRunlevel set to %d\n", runlevel);
}
