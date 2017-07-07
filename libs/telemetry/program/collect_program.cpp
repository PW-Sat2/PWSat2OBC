#include "collect_program.hpp"
#include "antenna/driver.h"
#include "antenna/telemetry.hpp"
#include "base/crc.h"
#include "boot/params.hpp"
#include "logger/logger.h"
#include "mcu/io_map.h"

namespace telemetry
{
    using namespace std::chrono_literals;

    ProgramCrcTelemetryAcquisition::ProgramCrcTelemetryAcquisition(program_flash::BootTable& bootTable) : _bootTable(bootTable)
    {
    }

    std::uint32_t ProgramCrcTelemetryAcquisition::GetLength(std::uint8_t index)
    {
        UniqueLock<program_flash::BootTable> lock(this->_bootTable, 10s);
        if (!lock())
        {
            return 0;
        }

        for (int i = 0; i < 8; ++i)
        {
            auto e = this->_bootTable.Entry(i);
            if ((index & (1 << i)) != 0 && e.IsValid())
            {
                return e.Length();
            }
        }

        return 0;
    }

    mission::UpdateDescriptor<telemetry::TelemetryState> ProgramCrcTelemetryAcquisition::BuildUpdate()
    {
        mission::UpdateDescriptor<telemetry::TelemetryState> descriptor;
        descriptor.name = "Program Crc Telemetry Acquisition";
        descriptor.updateProc = UpdateProc;
        descriptor.param = this;
        return descriptor;
    }

    mission::UpdateResult ProgramCrcTelemetryAcquisition::UpdateTelemetry(telemetry::TelemetryState& state)
    {
        const auto index = boot::Index;
        const auto length = GetLength(index);
        if (length == 0)
        {
            LOGF(LOG_LEVEL_ERROR, "Unable to get program length for index: %u. ", index);
            return mission::UpdateResult::Warning;
        }

        const auto result = CRC_calc(gsl::span<std::uint8_t>(io_map::ProgramFlash::ApplicatonBase, length));
        state.telemetry.Set(telemetry::ProgramState(result));
        return mission::UpdateResult::Ok;
    }

    mission::UpdateResult ProgramCrcTelemetryAcquisition::UpdateProc(telemetry::TelemetryState& state, void* param)
    {
        auto This = static_cast<ProgramCrcTelemetryAcquisition*>(param);
        return This->UpdateTelemetry(state);
    }
}
