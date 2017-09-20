#include "safe_mode.hpp"
#include <FreeRTOS.h>
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

        auto eepromStart = reinterpret_cast<std::uint8_t*>(0x80000000);

        auto eeprom = gsl::make_span(eepromStart, eepromStart + program_flash::SafeModeCopy::Size);

        for (std::size_t offset = 0; offset < program_flash::SafeModeCopy::Size; offset += 64)
        {
            auto isOk = memcmp(eeprom.data() + offset, this->_scrubBuffer.data() + offset, 64) == 0;

            if (isOk)
            {
                continue;
            }

            LOGF(LOG_LEVEL_WARNING, "[scrub] Rewriting safe mode in EEPROM at 0x%X", offset);

            auto correctPart = this->_scrubBuffer.subspan(offset, 64);

            portENTER_CRITICAL();

            *((volatile uint8_t*)(0x80000000 + 0x5555)) = 0xAA;
            *((volatile uint8_t*)(0x80000000 + 0x2AAA)) = 0x55;
            *((volatile uint8_t*)(0x80000000 + 0x5555)) = 0xA0;

            auto area = reinterpret_cast<volatile std::uint8_t*>(0x80000000 + offset);

            for (auto i = 0; i < 64; i++)
            {
                area[i] = correctPart[i];
            }

            portEXIT_CRITICAL();

            while ((area[63] & 0x80) != (correctPart[63] & 0x80))
            {
                System::SleepTask(1ms);
            }
        }

        this->_inProgress = false;

        LOG(LOG_LEVEL_INFO, "[scrub] Finished scrubbing safe mode");
    }
    SafeModeScrubbingStatus SafeModeScrubber::Status()
    {
        return SafeModeScrubbingStatus(this->_iterationsCount, this->_copiesCorrected, this->_mcuPagesCorrected);
    }
}
