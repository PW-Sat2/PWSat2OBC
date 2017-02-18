#include "adcs.hpp"
#include "logger/logger.h"

using experiments::IterationResult;
using experiments::StartResult;

namespace experiment
{
    namespace adcs
    {
        DetumblingExperiment::DetumblingExperiment(::adcs::IAdcsCoordinator& adcs, services::time::ICurrentTime& time)
            : _adcs(adcs), _time(time)
        {
        }

        void DetumblingExperiment::Duration(std::chrono::seconds duration)
        {
            this->_duration = duration;
        }

        experiments::ExperimentCode DetumblingExperiment::Type()
        {
            return Code;
        }

        experiments::StartResult DetumblingExperiment::Start()
        {
            auto r = this->_adcs.Disable();

            if (OS_RESULT_FAILED(r))
            {
                LOGF(LOG_LEVEL_ERROR, "Failed to disable current ADCS mode (%d)", num(r));
                return StartResult::Failure;
            }

            r = this->_adcs.EnableExperimentalDetumbling();

            if (OS_RESULT_FAILED(r))
            {
                LOGF(LOG_LEVEL_ERROR, "Failed to enable experimental detumbling (%d)", num(r));

                r = this->_adcs.EnableBuiltinDetumbling();

                if (OS_RESULT_FAILED(r))
                {
                    LOGF(LOG_LEVEL_FATAL, "Failed to reenable builtin detumbling (%d)", num(r));
                }

                return StartResult::Failure;
            }

            auto start = this->_time.GetCurrentTime();

            if (!start.HasValue)
            {
                return StartResult::Failure;
            }

            this->_endAt = start.Value + this->_duration;

            return StartResult::Success;
        }

        IterationResult DetumblingExperiment::Iteration()
        {
            auto now = this->_time.GetCurrentTime();

            if (!now.HasValue)
            {
                return experiments::IterationResult::Failure;
            }

            if (now.Value >= this->_endAt)
            {
                return IterationResult::Finished;
            }

            return IterationResult::WaitForNextCycle;
        }

        void DetumblingExperiment::Stop(IterationResult /*lastResult*/)
        {
            auto r = this->_adcs.Disable();

            if (OS_RESULT_FAILED(r))
            {
                LOGF(LOG_LEVEL_FATAL, "Failed to reenable builtin detumbling (%d)", num(r));
                return;
            }

            this->_adcs.EnableBuiltinDetumbling();
        }
    }
}
