#include "mission/telemetry.hpp"
#include <cassert>
#include "logger/logger.h"
#include "state/struct.h"

namespace mission
{
    TelemetryTask::TelemetryTask(std::tuple<services::fs::IFileSystem&, TelemetryConfiguration> arguments)
        : provider(std::get<0>(arguments)), //
          configuration(std::get<1>(arguments))
    {
    }

    ActionDescriptor<SystemState> TelemetryTask::BuildAction()
    {
        ActionDescriptor<SystemState> descriptor;
        descriptor.name = "Save telemetry to file";
        descriptor.param = this;
        descriptor.condition = SaveCondition;
        descriptor.actionProc = SaveProxy;
        return descriptor;
    }

    bool TelemetryTask::SaveCondition(const SystemState& state, void* /*param*/)
    {
        return state.telemetry.IsModified();
    }

    void TelemetryTask::SaveProxy(const SystemState& state, void* param)
    {
        auto This = static_cast<TelemetryTask*>(param);
        This->Save(state);
    }

    void TelemetryTask::Save(const SystemState& state)
    {
        std::array<std::uint8_t, state::ManagedTelemetry::TotalSerializedSize> buffer;
        Writer writer(buffer);
        state.telemetry.WriteModified(writer);
        assert(writer.Status());
        if (!writer.Status())
        {
            LOGF(LOG_LEVEL_ERROR, "Insufficient buffer space for telemetry: '%d'.", static_cast<int>(writer.GetDataLength()));
            return;
        }

        auto content = writer.Capture();
        if (content.empty())
        {
            return;
        }

        if (SaveToFile(content))
        {
            state.telemetry.CommitCapture();
        }
    }

    bool TelemetryTask::SaveToFile(gsl::span<const std::uint8_t> frame)
    {
        services::fs::File file(this->provider, //
            this->configuration.currentFileName,
            services::fs::FileOpen::OpenAlways,
            services::fs::FileAccess::WriteOnly);
        if (!file)
        {
            LOGF(LOG_LEVEL_ERROR, "Unable to open telemetry file: '%s'.", this->configuration.currentFileName);
            return false;
        }

        auto size = file.Size();
        if (size >= this->configuration.maxFileSize)
        {
            file.Close();

            const auto status = this->provider.Move(this->configuration.currentFileName, this->configuration.previousFileName);
            if (OS_RESULT_FAILED(status))
            {
                LOGF(LOG_LEVEL_ERROR,
                    "Unable to archive telemetry file: '%s' as '%s'.",
                    this->configuration.currentFileName,
                    this->configuration.previousFileName);
                return false;
            }

            file = services::fs::File(this->provider,
                this->configuration.currentFileName,
                services::fs::FileOpen::OpenAlways,
                services::fs::FileAccess::WriteOnly);
            if (!file)
            {
                LOGF(LOG_LEVEL_ERROR, "Unable to open telemetry file: '%s'.", this->configuration.currentFileName);
                return false;
            }
        }

        return static_cast<bool>(file.Write(frame));
    }
}
