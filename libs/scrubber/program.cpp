#include "program.hpp"
#include <algorithm>
#include <bitset>
#include <cstring>
#include "logger/logger.h"
#include "redundancy.hpp"

using namespace std::chrono_literals;

namespace scrubber
{
    static inline bool FastSpanCompare(const gsl::span<const uint8_t> a, const gsl::span<const uint8_t> b)
    {
        return memcmp(a.data(), b.data(), a.size()) == 0;
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

    ProgramScrubbingStatus::ProgramScrubbingStatus(std::uint32_t iterations, std::size_t offset, std::uint32_t slotsCorrected)
        : IterationsCount(iterations), Offset(offset), SlotsCorrected(slotsCorrected)
    {
    }

    ProgramScrubber::ProgramScrubber(
        ScrubBuffer& buffer, program_flash::BootTable& bootTable, program_flash::IFlashDriver& flashDriver, std::uint8_t slotsMask)
        : _buffer(buffer), _bootTable(bootTable), _flashDriver(flashDriver), _slotsMask(slotsMask), _offset(0), _iterationsCount(0),
          _slotsCorrected(0)
    {
    }

    void ProgramScrubber::ScrubSlots()
    {
        LOGF(LOG_LEVEL_INFO,
            "[scrub] Running program scrubbing on slots 0x%X at offset 0x%X",
            this->_slotsMask,
            static_cast<std::size_t>(this->_offset));

        std::array<uint8_t, 3> slots = DecodeSlotsMask(this->_slotsMask);

        UniqueLock<program_flash::BootTable> lock(this->_bootTable, 0s);

        if (!lock())
        {
            LOG(LOG_LEVEL_WARNING, "[scrub] Unable to take boot table lock - skipping program scrubbing");
            return;
        }

        program_flash::ProgramEntry entries[] = {
            this->_bootTable.Entry(slots[0]), this->_bootTable.Entry(slots[1]), this->_bootTable.Entry(slots[2]),
        };

        std::array<gsl::span<const std::uint8_t>, 3> scrubSpans;

        std::transform(std::begin(entries), std::end(entries), scrubSpans.begin(), [this](program_flash::ProgramEntry& entry) { //
            return entry.WholeEntry().subspan(this->_offset, ScrubSize);
        });

        redundancy::CorrectBuffer(this->_buffer, scrubSpans[0], scrubSpans[1], scrubSpans[2]);

        std::array<bool, 3> isCorrect;

        std::transform(scrubSpans.begin(), scrubSpans.end(), isCorrect.begin(), [this](const gsl::span<const uint8_t>& s) {
            return FastSpanCompare(s, this->_buffer);
        });

        LOGF(LOG_LEVEL_INFO, "[scrub] Check result: %d, %d, %d", isCorrect[0], isCorrect[1], isCorrect[2]);

        for (auto i = 0; i < 3; i++)
        {
            if (isCorrect[i])
            {
                continue;
            }

            LOGF(LOG_LEVEL_INFO, "[scrub] Rewriting slot %d", slots[i]);
            auto flashOffset = entries[i].InFlashOffset() + this->_offset;

            this->_flashDriver.EraseSector(flashOffset);

            this->_flashDriver.Program(flashOffset, this->_buffer);

            this->_slotsCorrected++;
        }

        this->_offset += ScrubSize;

        if (this->_offset >= ScrubAreaSize)
        {
            this->_offset = 0;
        }

        this->_iterationsCount++;

        LOG(LOG_LEVEL_MIN, "[scrub] Done");
    }

    ProgramScrubbingStatus ProgramScrubber::Status()
    {
        return ProgramScrubbingStatus(this->_iterationsCount, this->_offset, this->_slotsCorrected);
    }
}
