#ifndef LIBS_EXPERIMENTS_SUNS_INCLUDE_EXPERIMENT_SUNS_SUNS_HPP_
#define LIBS_EXPERIMENTS_SUNS_INCLUDE_EXPERIMENT_SUNS_SUNS_HPP_

#include <chrono>
#include <cstdint>
#include "experiments/experiments.h"
#include "power/fwd.hpp"

#include "time/fwd.hpp"

namespace experiments
{
    namespace suns
    {
        class SunSExperimentParams
        {
          public:
            SunSExperimentParams() = default;

            SunSExperimentParams(std::uint8_t gain,
                std::uint8_t itime,
                std::uint8_t samplesCount,
                std::chrono::seconds shortDelay,
                std::uint8_t samplingSessionsCount,
                std::chrono::minutes longDelay);

            inline std::uint8_t Gain() const;
            inline std::uint8_t ITime() const;
            inline std::uint8_t SamplesCount() const;
            inline std::chrono::seconds ShortDelay() const;
            inline std::uint8_t SamplingSessionsCount() const;
            inline std::chrono::minutes LongDelay() const;

          private:
            std::uint8_t _gain;
            std::uint8_t _itime;
            std::uint8_t _samplesCount;
            std::chrono::seconds _shortDelay;
            std::uint8_t _samplingSessionsCount;
            std::chrono::minutes _longDelay;
        };

        inline std::uint8_t SunSExperimentParams::Gain() const
        {
            return this->_gain;
        }

        inline std::uint8_t SunSExperimentParams::ITime() const
        {
            return this->_itime;
        }

        inline std::uint8_t SunSExperimentParams::SamplesCount() const
        {
            return this->_samplesCount;
        }

        inline std::chrono::seconds SunSExperimentParams::ShortDelay() const
        {
            return this->_shortDelay;
        }

        inline std::uint8_t SunSExperimentParams::SamplingSessionsCount() const
        {
            return this->_samplingSessionsCount;
        }

        inline std::chrono::minutes SunSExperimentParams::LongDelay() const
        {
            return this->_longDelay;
        }

        class SunSExperiment : public IExperiment
        {
          public:
            SunSExperiment(services::power::IPowerControl& powerControl, services::time::ICurrentTime& currentTime)
                : _powerControl(powerControl), _currentTime(currentTime), _nextSessionAt(0)
            {
            }

            void SetParameters(SunSExperimentParams parameters);

            virtual ExperimentCode Type() override;
            virtual StartResult Start() override;
            virtual IterationResult Iteration() override;
            virtual void Stop(IterationResult lastResult) override;

          private:
            services::power::IPowerControl& _powerControl;
            services::time::ICurrentTime& _currentTime;

            SunSExperimentParams _parameters;

            std::uint8_t _remainingSessions;
            std::chrono::milliseconds _nextSessionAt;
        };
    }
}

#endif /* LIBS_EXPERIMENTS_SUNS_INCLUDE_EXPERIMENT_SUNS_SUNS_HPP_ */
