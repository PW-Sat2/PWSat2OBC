#include "boot.h"
#include <bitset>
#include <em_usart.h>
#include <em_wdog.h>
#include "boot/params.hpp"
#include "bsp/bsp_boot.h"
#include "bsp/bsp_uart.h"
#include "main.hpp"
#include "redundancy.hpp"

using program_flash::ProgramEntry;

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

    WDOG_Init_TypeDef init = WDOG_INIT_DEFAULT;
    init.debugRun = false;
    init.enable = true;
    init.perSel = io_map::Watchdog::BootTimeout;

    WDOGn_Init(WDOG, &init);

    BOOT_boot(baseAddress);

    while (1)
        ;
}

static void ProgramBlock(gsl::span<const std::uint32_t> data, std::uint32_t* base)
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

template <typename Input, std::size_t Count, typename Mapper, typename Output = decltype(std::declval<Mapper>()(std::declval<Input&>()))>
std::array<Output, Count> Map(Input (&input)[Count], Mapper map)
{
    std::array<Output, Count> result;
    for (std::size_t i = 0; i < Count; i++)
    {
        result[i] = map(input[i]);
    }

    return result;
}

template <typename Input, std::size_t Count, typename Mapper, typename Output = decltype(std::declval<Mapper>()(std::declval<Input&>()))>
std::array<Output, Count> Map(std::array<Input, Count>& input, Mapper map)
{
    std::array<Output, Count> result;
    for (std::size_t i = 0; i < Count; i++)
    {
        result[i] = map(input[i]);
    }

    return result;
}

void LoadApplicationTMR(std::array<std::uint8_t, 3> slots)
{
    BSP_UART_Printf<30>(BSP_UART_DEBUG, "\nTMR boot on slots %d, %d and %d", slots[0], slots[1], slots[2]);

    ProgramEntry entries[] = {
        Bootloader.BootTable.Entry(slots[0]), Bootloader.BootTable.Entry(slots[1]), Bootloader.BootTable.Entry(slots[2]),
    };

    auto lengths = Map(entries, [](ProgramEntry& entry) { return entry.Length(); });

    auto length = redundancy::Correct(lengths);

    BSP_UART_Printf<30>(BSP_UART_DEBUG, "\nProgram length: %ld", length);

    using ChunkType = uint32_t;
    constexpr std::size_t PageSize = 4_KB;
    constexpr std::size_t ChunksCount = PageSize / sizeof(ChunkType);

    auto programBases = Map(entries, [](ProgramEntry& entry) { return reinterpret_cast<const ChunkType*>(entry.Content()); });

    BSP_UART_Printf<30>(BSP_UART_DEBUG, "\nBase: %p", (void*)programBases[0]);
    BSP_UART_Printf<30>(BSP_UART_DEBUG, "\nBase: %p", (void*)programBases[1]);
    BSP_UART_Printf<30>(BSP_UART_DEBUG, "\nBase: %p", (void*)programBases[2]);

    auto internalFlashBase = reinterpret_cast<ChunkType*>(BOOT_APPLICATION_BASE);

    std::array<ChunkType, ChunksCount> pageBuffer;

    for (auto offset = 0U; offset < length / sizeof(ChunkType); offset += ChunksCount)
    {
        auto pageSpans = Map(programBases, [offset](const ChunkType*& entry) { return gsl::make_span(entry + offset, ChunksCount); });

        auto ptr = internalFlashBase + offset;

        auto internalFlashPage = gsl::make_span(ptr, ChunksCount);

        redundancy::CorrectBuffer<const ChunkType>(pageBuffer, pageSpans[0], pageSpans[1], pageSpans[2]);

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

static std::array<std::uint8_t, 3> ToSlotNumbers(std::uint8_t slotsMask)
{
    std::array<std::uint8_t, 3> slots{0, 0, 0};

    std::bitset<6> mask(slotsMask);

    for (auto i = 0, j = 0; i < 6; i++)
    {
        if (mask[i])
        {
            slots[j] = i;
            j++;
        }

        if (j == 3)
            break;
    }

    return slots;
}

static std::uint32_t LoadApplication(std::uint8_t slotsMask)
{
    if (slotsMask == boot::BootSettings::SafeModeBootSlot)
    {
        return BOOT_SAFEMODE_BASE_CODE;
    }

    auto slots = ToSlotNumbers(slotsMask);

    LoadApplicationTMR(slots);

    return BOOT_APPLICATION_BASE;
}

static void ResolveFailedBoot()
{
    auto currentSlots = Bootloader.Settings.BootSlots();

    if (currentSlots == boot::BootSettings::SafeModeBootSlot)
    {
        return;
    }

    if (currentSlots == boot::BootSettings::UpperBootSlot)
    {
        return;
    }

    if (!Bootloader.Settings.WasLastBootConfirmed())
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "\nLast boot not confirmed - switch to failsafe slots");

        auto failsafe = Bootloader.Settings.FailsafeBootSlots();
        Bootloader.Settings.BootSlots(failsafe);

        boot::BootReason = boot::Reason::BootNotConfirmed;
    }
}

bool IsBootSlotsValid(std::uint8_t mask)
{
    if (__builtin_popcount(mask) != 3)
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "\n\nThree boot slots must be selected");
        return false;
    }

    auto slots = ToSlotNumbers(mask);
    auto isValid = redundancy::Correct( //
        Bootloader.BootTable.Entry(slots[0]).IsValid(),
        Bootloader.BootTable.Entry(slots[1]).IsValid(),
        Bootloader.BootTable.Entry(slots[2]).IsValid() //
        );

    if (!isValid)
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "\n\nBoot slots are not valid");
        return false;
    }

    return true;
}

void ProceedWithBooting()
{
    BSP_UART_Puts(BSP_UART_DEBUG, "\nTimeout exceeded - booting");

    boot::BootReason = boot::Reason::PrimaryBootSlots;

    ResolveFailedBoot();

    auto slotsMask = Bootloader.Settings.BootSlots();

    if (slotsMask == boot::BootSettings::SafeModeBootSlot)
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "\n\nSafe Mode boot slot... Booting safe mode!");
    }
    else if (slotsMask == boot::BootSettings::UpperBootSlot)
    {
        boot::BootReason = boot::Reason::PrimaryBootSlots;
        BSP_UART_Puts(BSP_UART_DEBUG, "\n\nUpper boot slot... Booting to upper");
        BootToAddress(BOOT_APPLICATION_BASE);
    }
    else if (!IsBootSlotsValid(slotsMask))
    {
        boot::BootReason = boot::Reason::InvalidPrimaryBootSlots;
        slotsMask = Bootloader.Settings.FailsafeBootSlots();

        BSP_UART_Puts(BSP_UART_DEBUG, "\n\nFalling back to failsafe boot slots");

        if (IsBootSlotsValid(slotsMask))
        {
            Bootloader.Settings.BootSlots(slotsMask);
        }
        else
        {
            BSP_UART_Puts(BSP_UART_DEBUG, "\n\nFailsafe boot slots invalid...Falling back to safe mode");

            boot::BootReason = boot::Reason::InvalidFailsafeBootSlots;
            slotsMask = boot::BootSettings::SafeModeBootSlot;
            Bootloader.Settings.BootSlots(boot::BootSettings::SafeModeBootSlot);
        }
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

    boot::Index = slotsMask;

    auto baseAddress = LoadApplication(slotsMask);

    Bootloader.Settings.UnconfirmLastBoot();

    BootToAddress(baseAddress);
}
