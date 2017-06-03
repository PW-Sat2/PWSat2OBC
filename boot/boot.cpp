#include "boot.h"
#include <bitset>
#include <em_usart.h>
#include "boot/params.hpp"
#include "bsp/bsp_boot.h"
#include "bsp/bsp_uart.h"
#include "main.hpp"
#include "redundancy.hpp"

static void resetPeripherals(void)
{
    SysTick->CTRL &= (~SysTick_CTRL_ENABLE_Msk);

    MSC_Deinit();
    DMA_Reset();
    USART_Reset(BSP_UART_DEBUG);
    USART_Reset(USART0);
}

static void resetClocks(void)
{
    CMU->HFCORECLKEN0 &= ~CMU_HFCORECLKEN0_DMA;
    CMU->HFPERCLKEN0 &= ~CMU_HFPERCLKEN0_UART1;
    CMU->HFPERCLKEN0 &= ~CMU_HFPERCLKEN0_USART0;
}

void BootToAddress(uint32_t baseAddress)
{
    resetPeripherals();
    resetClocks();

    boot::MagicNumber = boot::BootloaderMagicNumber;

    BOOT_boot(baseAddress);

    while (1)
        ;
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

void LoadApplicationTMR(std::array<std::uint8_t, 3> slots)
{
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
}

static std::uint32_t LoadApplication(std::uint8_t slotsMask)
{
    if (slotsMask == boot::BootSettings::SafeModeBootSlot)
    {
        return BOOT_SAFEMODE_BASE_CODE;
    }

    std::array<std::uint8_t, 3> slots{0, 0, 0};

    std::bitset<6> mask(slotsMask);

    for (auto i = 0, j = 0; i < 6; i++)
    {
        if (mask[i])
        {
            slots[j] = i + 1;
            j++;
        }

        if (j == 3)
            break;
    }

    LoadApplicationTMR(slots);

    return BOOT_APPLICATION_BASE;
}

void ProceedWithBooting()
{
    BSP_UART_Puts(BSP_UART_DEBUG, "\nTimeout exceeded - booting");

    auto slotsMask = Bootloader.Settings.BootSlots();

    boot::BootReason = boot::Reason::SelectedIndex;

    if (slotsMask == boot::BootSettings::SafeModeBootSlot)
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "\n\nSafe Mode boot index... Booting safe mode!");
    }
    else if (__builtin_popcount(slotsMask) != 3)
    {
        slotsMask = boot::BootSettings::SafeModeBootSlot;
        Bootloader.Settings.BootSlots(boot::BootSettings::SafeModeBootSlot);

        boot::BootReason = boot::Reason::InvalidBootIndex;

        BSP_UART_Puts(BSP_UART_DEBUG, "\n\nInvalid boot index... Booting safe mode!");
    }
    else if (Bootloader.Settings.BootCounter() == 0)
    {
        slotsMask = boot::BootSettings::SafeModeBootSlot;
        Bootloader.Settings.BootSlots(boot::BootSettings::SafeModeBootSlot);

        boot::BootReason = boot::Reason::CounterExpired;

        BSP_UART_Puts(BSP_UART_DEBUG, "\n\nBoot counter expired... Booting safe mode!");
    }
    else
    {
        auto counter = Bootloader.Settings.BootCounter();
        Bootloader.Settings.BootCounter(counter - 1);
    }

    auto baseAddress = LoadApplication(slotsMask);
    BootToAddress(baseAddress);
}
