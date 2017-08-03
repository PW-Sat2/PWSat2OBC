#include "suns.hpp"
#include "power/power.h"
#include "time/ICurrentTime.hpp"

namespace experiments
{
    namespace suns
    {
        SunSExperimentParams::SunSExperimentParams(std::uint8_t gain,
            std::uint8_t itime,
            std::uint8_t samplesCount,
            std::chrono::seconds shortDelay,
            std::uint8_t samplingSessionsCount,
            std::chrono::minutes longDelay)
            : _gain(gain), _itime(itime), _samplesCount(samplesCount), _shortDelay(shortDelay),
              _samplingSessionsCount(samplingSessionsCount), _longDelay(longDelay)
        {
        }

        void SunSExperiment::SetParameters(SunSExperimentParams parameters)
        {
            this->_parameters = parameters;
        }

        ExperimentCode SunSExperiment::Type()
        {
            return 0x04;
        }

        StartResult SunSExperiment::Start()
        {
            this->_powerControl.SensPower(true);
            this->_powerControl.SunSPower(true);

            this->_remainingSessions = this->_parameters.SamplingSessionsCount();

            return StartResult::Success;
        }

        IterationResult SunSExperiment::Iteration()
        {
            auto currentTime = this->_currentTime.GetCurrentTime();

            if (!currentTime.HasValue)
            {
                return IterationResult::WaitForNextCycle;
            }

            if (currentTime.Value < this->_nextSessionAt)
            {
                return IterationResult::WaitForNextCycle;
            }

            this->_remainingSessions--;

            if (this->_remainingSessions == 0)
            {
                return IterationResult::Finished;
            }

            this->_nextSessionAt = currentTime.Value + this->_parameters.LongDelay(); // TODO: use updated time value

            return IterationResult::WaitForNextCycle;
        }

        void SunSExperiment::Stop(IterationResult /*lastResult*/)
        {
            this->_powerControl.SensPower(false);
            this->_powerControl.SunSPower(false);
        }

        DataPoint SunSExperiment::GatherSingleMeasurement()
        {
            DataPoint point;

            point.Timestamp = this->_currentTime.GetCurrentTime().Value;

            this->_experimentalSunS.StartMeasurement(this->_parameters.Gain(), this->_parameters.ITime());

            this->_experimentalSunS.GetMeasuredData(point.ExperimentalSunS);

            return point;
        }
    }
}
