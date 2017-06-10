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

// static constexpr std::size_t ScrubSize = program_flash::IFlashDriver::LargeSectorSize;
//
// static std::array<uint8_t, ScrubSize> ScrubBuffer;
//
// static inline bool FastSpanCompare(const gsl::span<const uint8_t>& a, const gsl::span<const uint8_t>& b)
//{
//    return memcmp(a.data(), b.data(), a.size()) == 0;
//}

void ScrubProgram(std::uint16_t /*argc*/, char* /*argv*/ [])
{
    //    std::array<uint8_t, 3> slots{0, 1, 2};
    //
    //    program_flash::ProgramEntry entries[] = {
    //        Main.BootTable.Entry(slots[0]), Main.BootTable.Entry(slots[1]), Main.BootTable.Entry(slots[2])};
    //
    //    for (std::size_t scrubOffset = 0; scrubOffset < program_flash::ProgramEntry::Size; scrubOffset += ScrubSize)
    //    {
    //        auto scrubSpans =
    //            Map(entries, [scrubOffset](program_flash::ProgramEntry& entry) { return entry.WholeEntry().subspan(scrubOffset,
    //            ScrubSize); });
    //
    //        redundancy::CorrectBuffer(ScrubBuffer, scrubSpans[0], scrubSpans[1], scrubSpans[2]);
    //
    //        gsl::span<uint8_t> bufferSpan = ScrubBuffer;
    //
    //        auto isCorrect = Map(scrubSpans, [bufferSpan](const gsl::span<const uint8_t>& s) { return FastSpanCompare(s, bufferSpan); });
    //
    //        Main.terminal.Printf("Offset 0x%X:\n", scrubOffset);
    //        Main.terminal.Printf("Correct[0]=%s\n", isCorrect[0] ? "Yes" : "No");
    //        Main.terminal.Printf("Correct[1]=%s\n", isCorrect[1] ? "Yes" : "No");
    //        Main.terminal.Printf("Correct[2]=%s\n\n", isCorrect[2] ? "Yes" : "No");
    //
    //        for (auto i = 0; i < 3; i++)
    //        {
    //            if (isCorrect[i])
    //            {
    //                continue;
    //            }
    //
    //            // calc offset inside flash
    //            auto flashOffset = entries[i].InFlashOffset() + scrubOffset;
    //
    //            // erase
    //            Main.Hardware.FlashDriver.EraseSector(flashOffset);
    //
    //            // program correctBuffer
    //            Main.Hardware.FlashDriver.Program(flashOffset, ScrubBuffer);
    //        }
    //    }
}
