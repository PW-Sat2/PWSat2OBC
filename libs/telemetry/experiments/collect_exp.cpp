#include "collect_exp.hpp"
#include "logger/logger.h"

namespace telemetry
{
    ExperimentTelemetryAcquisition::ExperimentTelemetryAcquisition(experiments::IExperimentController& experiment) : provider(&experiment)
    {
    }

    mission::UpdateDescriptor<telemetry::TelemetryState> ExperimentTelemetryAcquisition::BuildUpdate()
    {
        mission::UpdateDescriptor<telemetry::TelemetryState> descriptor;
        descriptor.name = "Experiment Telemetry Acquisition";
        descriptor.updateProc = UpdateProc;
        descriptor.param = this;
        return descriptor;
    }

    mission::UpdateResult ExperimentTelemetryAcquisition::UpdateExperimentTelemetry(telemetry::TelemetryState& state)
    {
        state.telemetry.Set(ExperimentTelemetry(this->provider->CurrentState()));
        return mission::UpdateResult::Ok;
    }

    mission::UpdateResult ExperimentTelemetryAcquisition::UpdateProc(telemetry::TelemetryState& state, void* param)
    {
        auto This = static_cast<ExperimentTelemetryAcquisition*>(param);
        return This->UpdateExperimentTelemetry(state);
    }
}
