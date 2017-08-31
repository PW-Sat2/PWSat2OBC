#include "bootloader.hpp"
#include <algorithm>
#include <cstring>
#include "logger/logger.h"
#include "msc/msc.hpp"
#include "redundancy.hpp"

using program_flash::BootloaderCopy;
using drivers::msc::MCUMemoryController;
using namespace std::chrono_literals;

namespace scrubber
{
    BootloaderScrubbingStatus::BootloaderScrubbingStatus(
        std::uint32_t iterationsCount, std::uint32_t copiesCorrected, std::uint32_t mcuPagesCorrected)
        : IterationsCount(iterationsCount), CopiesCorrected(copiesCorrected), MUCPagesCorrected(mcuPagesCorrected)
    {
    }

    BootloaderScrubber::BootloaderScrubber(ScrubBuffer& scrubBuffer, program_flash::BootTable& bootTable, drivers::msc::IMCUFlash& mcuFlash)
        : _scrubBuffer(scrubBuffer), _bootTable(bootTable), _mcuFlash(mcuFlash), _iterationsCount(0), _copiesCorrected(0),
          _mcuPagesCorrected(0)
    {
    }

    void BootloaderScrubber::Scrub()
    {
        LOG(LOG_LEVEL_INFO, "[scrub] Scrubbing bootloader copies");

        UniqueLock<program_flash::BootTable> lock(this->_bootTable, 0s);

        if (!lock())
        {
            LOG(LOG_LEVEL_WARNING, "[scrub] Unable to take boot table lock - skipping bootloader scrubbing");
            return;
        }

        this->_inProgress = true;

        BootloaderCopy copies[] = {
            _bootTable.GetBootloaderCopy(0),
            _bootTable.GetBootloaderCopy(1),
            _bootTable.GetBootloaderCopy(2),
            _bootTable.GetBootloaderCopy(3),
            _bootTable.GetBootloaderCopy(4),
        };

        std::array<gsl::span<const std::uint8_t>, 5> spans;

        std::transform(std::begin(copies), std::end(copies), spans.begin(), [](BootloaderCopy& copy) { return copy.Content(); });

        redundancy::CorrectBuffer(gsl::make_span(this->_scrubBuffer), spans);

        std::array<bool, 5> isCorrect;

        std::transform(spans.begin(), spans.end(), isCorrect.begin(), [this](gsl::span<const std::uint8_t>& copy) { //
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

            this->_copiesCorrected++;
        }

        LOG(LOG_LEVEL_INFO, "[scrub] Scrubbing bootloader in MCU");

        auto bootloaderStart = this->_mcuFlash.Begin();

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
            this->_mcuPagesCorrected++;
        }

        this->_inProgress = false;

        LOG(LOG_LEVEL_INFO, "[scrub] Finished scrubbing bootloader");
    }
    BootloaderScrubbingStatus BootloaderScrubber::Status()
    {
        return BootloaderScrubbingStatus(this->_iterationsCount, this->_copiesCorrected, this->_mcuPagesCorrected);
    }
}
