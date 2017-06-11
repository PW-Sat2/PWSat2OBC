#include <array>
#include <cstdint>
#include <em_cmu.h>
#include <em_gpio.h>
#include <em_int.h>
#include <em_timer.h>
#include <gsl/span>
#include "obc.h"
#include "redundancy.hpp"
#include "scrubber/ram.hpp"
#include "utils.h"

using std::uint8_t;
using std::uint32_t;

void ScrubRAM(std::uint16_t /*argc*/, char* /*argv*/ [])
{
    auto current = Scrubber::Current();

    Main.terminal.Printf("Current = 0x%08X\n", current);
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

static inline bool FastSpanCompare(const gsl::span<const uint8_t> a, const gsl::span<const uint8_t> b)
{
    return memcmp(a.data(), b.data(), a.size()) == 0;
}

static std::array<uint8_t, 64_KB> Scrub;

static constexpr std::size_t MCUFlashEraseSector = 4_KB;

void ScrubProgram(std::uint16_t /*argc*/, char* /*argv*/ [])
{
    program_flash::BootloaderCopy copies[] = {
        Main.BootTable.GetBootloaderCopy(0),
        Main.BootTable.GetBootloaderCopy(1),
        Main.BootTable.GetBootloaderCopy(2),
        Main.BootTable.GetBootloaderCopy(3),
        Main.BootTable.GetBootloaderCopy(4),
    };

    std::array<gsl::span<const uint8_t>, 5> spans = Map(copies, [](program_flash::BootloaderCopy& copy) { return copy.Content(); });

    redundancy::CorrectBuffer(gsl::make_span(Scrub), spans);

    for (std::uint8_t i = 0; i < 5; i++)
    {
        auto r = FastSpanCompare(Scrub, spans[i]);

        if (r)
        {
            Main.terminal.Printf("Slot %d: OK\n", i);
        }
        else
        {
            Main.terminal.Printf("Slot %d: NOT OK\n", i);
        }
    }

    auto bootloaderStart = reinterpret_cast<const uint8_t*>(0);

    auto& msc = Main.Hardware.MCUFlash;

    for (std::size_t offset = 0; offset < 64_KB; offset += MCUFlashEraseSector)
    {
        auto chunk = gsl::make_span(Scrub).subspan(offset, MCUFlashEraseSector);

        auto isOk = memcmp(bootloaderStart + offset, chunk.data(), MCUFlashEraseSector) == 0;

        if (isOk)
        {
            Main.terminal.Printf("Page 0x%X: OK\n", offset);
        }
        else
        {
            Main.terminal.Printf("Page 0x%X: NOT OK\n", offset);

            msc.Erase(offset);
            msc.Program(offset, chunk);
        }
    }
}
