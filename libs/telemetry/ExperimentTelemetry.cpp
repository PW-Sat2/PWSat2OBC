#include "base/BitWriter.hpp"
#include "telemetry/Experiments.hpp"

namespace telemetry
{
    static experiments::ExperimentCode GetCode(const experiments::ExperimentState& state)
    {
        if (state.CurrentExperiment.HasValue)
        {
            return state.CurrentExperiment.Value;
        }
        else
        {
            return 0;
        }
    }

    static experiments::StartResult GetStartResult(const experiments::ExperimentState& state)
    {
        if (state.LastStartResult.HasValue)
        {
            return state.LastStartResult.Value;
        }
        else
        {
            return experiments::StartResult::Success;
        }
    }

    static experiments::IterationResult GetIterationResult(const experiments::ExperimentState& state)
    {
        if (state.LastIterationResult.HasValue)
        {
            return state.LastIterationResult.Value;
        }
        else
        {
            return experiments::IterationResult::None;
        }
    }

    ExperimentTelemetry::ExperimentTelemetry()
        : currentExperiment(0),                                     //
          experimentStartResult(experiments::StartResult::Success), //
          experimentIterationResult(experiments::IterationResult::None)
    {
    }

    ExperimentTelemetry::ExperimentTelemetry(experiments::ExperimentCode code, //
        experiments::StartResult startResult,                                  //
        experiments::IterationResult iterationResult)                          //
        : currentExperiment(code),                                             //
          experimentStartResult(startResult),                                  //
          experimentIterationResult(iterationResult)
    {
    }

    ExperimentTelemetry::ExperimentTelemetry(const experiments::ExperimentState& state)
        : currentExperiment(GetCode(state)),            //
          experimentStartResult(GetStartResult(state)), //
          experimentIterationResult(GetIterationResult(state))
    {
    }

    void ExperimentTelemetry::Write(BitWriter& writer) const
    {
        writer.Write(this->currentExperiment);
        writer.Write(this->experimentStartResult);
        writer.Write(this->experimentIterationResult);
    }
}
