#include "program.hpp"
#include <bitset>
#include <cstring>
#include "base/array_map.hpp"
#include "logger/logger.h"
#include "redundancy.hpp"

namespace scrubber
{
    namespace
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
    }

    ProgramScrubber::ProgramScrubber(
        ScrubBuffer& buffer, program_flash::BootTable& bootTable, program_flash::IFlashDriver& flashDriver, std::uint8_t slotsMask)
        : _buffer(buffer), _bootTable(bootTable), _flashDriver(flashDriver), _slotsMask(slotsMask), _offset(0)
    {
    }

    void ProgramScrubber::ScrubSlots()
    {
        LOGF(LOG_LEVEL_INFO, "[scrub] Running program scrubbing on slots 0x%X at offset 0x%X", this->_slotsMask, this->_offset);

        std::array<uint8_t, 3> slots = DecodeSlotsMask(this->_slotsMask);

        // lock

        program_flash::ProgramEntry entries[] = {
            this->_bootTable.Entry(slots[0]), this->_bootTable.Entry(slots[1]), this->_bootTable.Entry(slots[2]),
        };

        auto scrubSpans = Map(entries, [this](program_flash::ProgramEntry& entry) { //
            return entry.WholeEntry().subspan(this->_offset, ScrubSize);
        });

        redundancy::CorrectBuffer(this->_buffer, scrubSpans[0], scrubSpans[1], scrubSpans[2]);

        auto isCorrect = Map(scrubSpans, [this](const gsl::span<const uint8_t>& s) { return FastSpanCompare(s, this->_buffer); });

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
        }

        // unlock

        this->_offset += ScrubSize;

        if (this->_offset >= ScrubAreaSize)
        {
            this->_offset = 0;
        }
        LOG(LOG_LEVEL_MIN, "[scrub] Done");
    }
}
