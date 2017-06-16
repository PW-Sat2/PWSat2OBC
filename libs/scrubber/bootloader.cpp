#include "bootloader.hpp"
#include <cstring>
#include "base/array_map.hpp"
#include "logger/logger.h"
#include "msc/msc.hpp"
#include "redundancy.hpp"

using program_flash::BootloaderCopy;
using drivers::msc::MCUMemoryController;

namespace scrubber
{
    BootloaderScrubbingStatus::BootloaderScrubbingStatus(
        std::uint32_t iterationsCount, std::uint32_t copiesCorrected, std::uint32_t mcuPagesCorrected)
        : IterationsCount(iterationsCount), CopiesCorrected(copiesCorrected), MUCPagesCorrected(mcuPagesCorrected)
    {
    }

    BootloaderScrubber::BootloaderScrubber(
        ScrubBuffer& scrubBuffer, program_flash::BootTable& bootTable, drivers::msc::MCUMemoryController& mcuFlash)
        : _scrubBuffer(scrubBuffer), _bootTable(bootTable), _mcuFlash(mcuFlash), _iterationsCount(0), _copiesCorrected(0),
          _mcuPagesCorrected(0)
    {
    }

    void BootloaderScrubber::Scrub()
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

        LOG(LOG_LEVEL_INFO, "[scrub] Scrubbing bootloader in MCU");

        auto bootloaderStart = reinterpret_cast<std::uint8_t*>(0);

        for (std::size_t offset = 0; offset < program_flash::BootloaderCopy::Size; offset += MCUMemoryController::SectorSize)
        {
            auto isOk = memcmp(bootloaderStart + offset, this->_scrubBuffer.data() + offset, MCUMemoryController::SectorSize) == 0;

            if (isOk)
            {
                continue;
            }

            LOGF(LOG_LEVEL_WARNING, "[scrub] Rewriting MCU bootloader at offset 0x%X", offset);
            this->_mcuFlash.Erase(offset);

            auto validPart = gsl::make_span(this->_scrubBuffer).subspan(offset, MCUMemoryController::SectorSize);

            this->_mcuFlash.Program(offset, validPart);
        }

        LOG(LOG_LEVEL_INFO, "[scrub] Finished scrubbing bootloader");
    }
    BootloaderScrubbingStatus BootloaderScrubber::Status()
    {
        return BootloaderScrubbingStatus(this->_iterationsCount, this->_copiesCorrected, this->_mcuPagesCorrected);
    }
}
