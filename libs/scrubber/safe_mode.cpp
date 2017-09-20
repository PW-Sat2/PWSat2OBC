#include "safe_mode.hpp"
#include <algorithm>
#include <cstring>
#include "logger/logger.h"
#include "msc/msc.hpp"
#include "redundancy.hpp"

using program_flash::SafeModeCopy;
using drivers::msc::MCUMemoryController;
using namespace std::chrono_literals;

namespace scrubber
{
    SafeModeScrubbingStatus::SafeModeScrubbingStatus(
        std::uint32_t iterationsCount, std::uint32_t copiesCorrected, std::uint32_t mcuPagesCorrected)
        : IterationsCount(iterationsCount), CopiesCorrected(copiesCorrected), MUCPagesCorrected(mcuPagesCorrected)
    {
    }

    SafeModeScrubber::SafeModeScrubber(ScrubBuffer& scrubBuffer, program_flash::BootTable& bootTable, drivers::msc::IMCUFlash& mcuFlash)
        : _scrubBuffer(gsl::make_span(scrubBuffer).subspan(0, program_flash::SafeModeCopy::Size)), _bootTable(bootTable),
          _mcuFlash(mcuFlash), _iterationsCount(0), _copiesCorrected(0), _mcuPagesCorrected(0)
    {
    }

    void SafeModeScrubber::Scrub()
    {
        LOG(LOG_LEVEL_INFO, "[scrub] Scrubbing safe mode copies");

        UniqueLock<program_flash::BootTable> lock(this->_bootTable, 0s);

        if (!lock())
        {
            LOG(LOG_LEVEL_WARNING, "[scrub] Unable to take boot table lock - skipping safe mode scrubbing");
            return;
        }

        this->_inProgress = true;

        SafeModeCopy copies[] = {
            _bootTable.GetSafeModeCopy(0),
            _bootTable.GetSafeModeCopy(1),
            _bootTable.GetSafeModeCopy(2),
            _bootTable.GetSafeModeCopy(3),
            _bootTable.GetSafeModeCopy(4),
        };

        std::array<gsl::span<const std::uint8_t>, 5> spans;

        std::transform(std::begin(copies), std::end(copies), spans.begin(), [](SafeModeCopy& copy) { return copy.Content(); });

        redundancy::CorrectBuffer(this->_scrubBuffer, spans);

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

            LOGF(LOG_LEVEL_WARNING, "[scrub] Rewriting safe mode copy %d", i);

            copies[i].Erase();
            copies[i].Write(0, this->_scrubBuffer);

            this->_copiesCorrected++;
        }

        LOG(LOG_LEVEL_INFO, "[scrub] Scrubbing safe mode in EEPROM");

        //        auto bootloaderStart = this->_mcuFlash.Begin();
        //
        //        for (std::size_t offset = 0; offset < program_flash::SafeModeCopy::Size; offset += MCUMemoryController::SectorSize)
        //        {
        //            auto isOk = memcmp(bootloaderStart + offset, this->_scrubBuffer.data() + offset, MCUMemoryController::SectorSize) ==
        //            0;
        //
        //            if (isOk)
        //            {
        //                continue;
        //            }
        //
        //            LOGF(LOG_LEVEL_WARNING, "[scrub] Rewriting MCU bootloader at offset 0x%X", offset);
        //            this->_mcuFlash.Erase(offset);
        //
        //            auto validPart = gsl::make_span(this->_scrubBuffer).subspan(offset, MCUMemoryController::SectorSize);
        //
        //            this->_mcuFlash.Program(offset, validPart);
        //            this->_mcuPagesCorrected++;
        //        }

        this->_inProgress = false;

        LOG(LOG_LEVEL_INFO, "[scrub] Finished scrubbing safe mode");
    }
    SafeModeScrubbingStatus SafeModeScrubber::Status()
    {
        return SafeModeScrubbingStatus(this->_iterationsCount, this->_copiesCorrected, this->_mcuPagesCorrected);
    }
}
