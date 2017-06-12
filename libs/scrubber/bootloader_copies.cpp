#include "bootloader_copies.hpp"
#include <cstring>
#include "base/array_map.hpp"
#include "logger/logger.h"
#include "redundancy.hpp"

using program_flash::BootloaderCopy;

namespace scrubber
{
    BootloaderCopiesScrubber::BootloaderCopiesScrubber(ScrubBuffer& scrubBuffer, program_flash::BootTable& bootTable)
        : _scrubBuffer(scrubBuffer), _bootTable(bootTable)
    {
    }

    void BootloaderCopiesScrubber::Scrub()
    {
        LOG(LOG_LEVEL_INFO, "[scrub] Scrubbing bootloader copies");

        BootloaderCopy copies[] = {
            _bootTable.GetBootloaderCopy(0),
            _bootTable.GetBootloaderCopy(1),
            _bootTable.GetBootloaderCopy(2),
            _bootTable.GetBootloaderCopy(3),
            _bootTable.GetBootloaderCopy(4),
        };

        std::array<gsl::span<const std::uint8_t>, 5> spans = Map(copies, [](BootloaderCopy& copy) { return copy.Content(); });

        redundancy::CorrectBuffer(gsl::make_span(this->_scrubBuffer), spans);

        auto isCorrect = Map(spans,
            [this](gsl::span<const std::uint8_t>& copy) { //
                return memcmp(this->_scrubBuffer.data(), copy.data(), copy.size()) == 0;
            });

        for (std::uint8_t i = 0; i < count_of(copies); i++)
        {
            if (isCorrect[i])
            {
                continue;
            }

            LOGF(LOG_LEVEL_WARNING, "[scrub] Rewriting bootloader copy %d", i);

            copies[i].Erase();
            copies[i].Write(0, this->_scrubBuffer);
        }

        LOG(LOG_LEVEL_INFO, "[scrub] Fnished scrubbing bootloader copies");
    }
}
