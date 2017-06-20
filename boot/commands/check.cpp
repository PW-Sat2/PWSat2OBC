#include <algorithm>
#include <bitset>
#include "base/crc.h"
#include "bsp/bsp_uart.h"
#include "main.hpp"
#include "program_flash/boot_table.hpp"

static void CheckBootSettings()
{
    auto settingsValid = Bootloader.Settings.CheckMagicNumber();

    if (settingsValid)
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "[OK  ] Boot settings tagged with magic number\n");
    }
    else
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "[FAIL] Boot settings tagged with magic number\n");
    }

    auto primaryBootSlots = Bootloader.Settings.BootSlots();

    if (primaryBootSlots == 0b111)
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "[OK  ] Primary boot slots set to 0,1,2\n");
    }
    else
    {
        BSP_UART_Printf<40>(BSP_UART_DEBUG, "[FAIL] Primary boot slots set to 0,1,2 (actual: 0x%X)\n", primaryBootSlots);
    }

    auto failsafeBootSlots = Bootloader.Settings.FailsafeBootSlots();

    if (failsafeBootSlots == 0b111000)
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "[OK  ] Failsafe boot slots set to 3,4,5\n");
    }
    else
    {
        BSP_UART_Printf<40>(BSP_UART_DEBUG, "[FAIL] Failsafe boot slots set to 3,4,5 (actual: 0x%X)\n", failsafeBootSlots);
    }

    auto bootCounter = Bootloader.Settings.BootCounter();

    if (bootCounter == 0)
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "[OK  ] Boot counter set to 0\n");
    }
    else
    {
        BSP_UART_Printf<40>(BSP_UART_DEBUG, "[FAIL] Boot counter set to 0 (actual: %ld)\n", bootCounter);
    }

    auto lastConfirmedBootCounter = Bootloader.Settings.LastConfirmedBootCounter();

    if (lastConfirmedBootCounter == 0)
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "[OK  ] Last confirmed boot counter set to 0\n");
    }
    else
    {
        BSP_UART_Printf<40>(BSP_UART_DEBUG, "[FAIL] Last confirmed boot counter set to 0 (actual: %ld)\n", lastConfirmedBootCounter);
    }
}

static std::array<uint8_t, 3> DecodeSlotsMask(std::uint8_t mask)
{
    std::array<uint8_t, 3> result{0, 0, 0};
    std::bitset<program_flash::BootTable::EntriesCount> slots(mask);

    auto j = 0;
    for (std::uint8_t i = 0; i < program_flash::BootTable::EntriesCount; i++)
    {
        if (slots[i])
        {
            result[j] = i;
            j++;
        }
        if (j == 3)
            break;
    }

    return result;
}

static void CheckBootSlots()
{
    std::array<bool, 6> slotValid;
    std::array<std::uint16_t, 6> crc;

    for (auto i = 0; i < program_flash::BootTable::EntriesCount; i++)
    {
        slotValid[i] = false;
        auto entry = Bootloader.BootTable.Entry(i);

        if (!entry.IsValid())
        {
            BSP_UART_Printf<40>(BSP_UART_DEBUG, "[FAIL] Boot slot %d: Not valid\n", i);
            continue;
        }
        else
        {
            BSP_UART_Printf<40>(BSP_UART_DEBUG, "[OK  ] Boot slot %d: Valid\n", i);
        }

        auto expectedCrc = entry.Crc();
        auto actualCrc = entry.CalculateCrc();

        if (expectedCrc != actualCrc)
        {
            BSP_UART_Printf<40>(
                BSP_UART_DEBUG, "[FAIL] Boot slot %d: CRC mismatch (expected: 0x%X, actual: 0x%X)\n", i, expectedCrc, actualCrc);
            continue;
        }
        else
        {
            BSP_UART_Printf<40>(BSP_UART_DEBUG, "[OK  ] Boot slot %d: CRC match (0x%X)\n", i, expectedCrc);
        }

        crc[i] = actualCrc;
        slotValid[i] = true;
    }

    auto slots = DecodeSlotsMask(Bootloader.Settings.BootSlots());

    auto allValid = slotValid[slots[0]] && slotValid[slots[1]] && slotValid[slots[2]];
    auto crcMatch = (crc[slots[0]] == crc[slots[1]]) && (crc[slots[1]] == crc[slots[2]]);

    if (allValid && crcMatch)
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "[OK  ] Primary boot slots valid & CRC match\n");
    }
    else
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "[FAIL] Primary boot slots valid & CRC match\n");
    }

    slots = DecodeSlotsMask(Bootloader.Settings.FailsafeBootSlots());

    allValid = slotValid[slots[0]] && slotValid[slots[1]] && slotValid[slots[2]];
    crcMatch = (crc[slots[0]] == crc[slots[1]]) && (crc[slots[1]] == crc[slots[2]]);

    if (allValid && crcMatch)
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "[OK  ] Failsafe boot slots valid & CRC match\n");
    }
    else
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "[FAIL] Failsafe boot slots valid & CRC match\n");
    }
}

static void CheckBootloader()
{
    std::array<std::uint16_t, program_flash::BootTable::BootloaderCopies> crc;

    for (auto i = 0; i < program_flash::BootTable::BootloaderCopies; i++)
    {
        auto copy = Bootloader.BootTable.GetBootloaderCopy(i);
        crc[i] = copy.CalculateCrc();
    }

    auto r = std::minmax_element(crc.begin(), crc.end());

    if (*r.first == *r.second)
    {
        BSP_UART_Printf<50>(BSP_UART_DEBUG, "[OK  ] Bootloader copies all the same (0x%X)\n", *r.first);
    }
    else
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "[FAIL] Bootloader copies not the same (CRCs:");

        for (auto i = 0U; i < crc.size(); i++)
        {
            BSP_UART_Printf<10>(BSP_UART_DEBUG, " 0x%X", crc[i]);
        }

        BSP_UART_Puts(BSP_UART_DEBUG, ")\n");
    }

    auto mcuCrc = CRC_calc(reinterpret_cast<std::uint8_t*>(0), reinterpret_cast<std::uint8_t*>(program_flash::BootloaderCopy::Size));

    if (mcuCrc == *r.first)
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "[OK  ] Bootloader in MCU matches copies\n");
    }
    else
    {
        BSP_UART_Printf<80>(BSP_UART_DEBUG, "[FAIL] Bootloader in MCU does not match copies (MCU: 0x%X Copy: 0x%X)\n", mcuCrc, *r.first);
    }
}

void Check()
{
    BSP_UART_Puts(BSP_UART_DEBUG, "\nChecking OBC:\n");

    CheckBootSettings();

    CheckBootSlots();

    CheckBootloader();
}
