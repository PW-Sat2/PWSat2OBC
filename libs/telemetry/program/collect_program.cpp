#include "collect_program.hpp"
#include "antenna/driver.h"
#include "antenna/telemetry.hpp"
#include "base/crc.h"
#include "boot/params.hpp"
#include "io_map.h"
#include "logger/logger.h"

namespace telemetry
{
    ProgramCrcTelemetryAcquisition::ProgramCrcTelemetryAcquisition(program_flash::BootTable& bootTable) : _bootTable(bootTable)
    {
    }

    std::uint32_t ProgramCrcTelemetryAcquisition::GetLength(std::uint8_t index)
    {
        UniqueLock<program_flash::BootTable> lock(this->_bootTable, InfiniteTimeout);
        auto e = this->_bootTable.Entry(index);
        if (!e.IsValid())
        {
            return 0;
        }
        else
        {
            return e.Length();
        }
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
