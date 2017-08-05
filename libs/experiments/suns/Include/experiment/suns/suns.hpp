#ifndef LIBS_EXPERIMENTS_SUNS_INCLUDE_EXPERIMENT_SUNS_SUNS_HPP_
#define LIBS_EXPERIMENTS_SUNS_INCLUDE_EXPERIMENT_SUNS_SUNS_HPP_

#include <chrono>
#include <cstdint>
#include <cstring>
#include "experiments/experiments.h"
#include "fs/ExperimentFile.hpp"
#include "fs/fwd.hpp"
#include "gyro/gyro.h"
#include "gyro/telemetry.hpp"
#include "payload/interfaces.h"
#include "power/fwd.hpp"
#include "suns/suns.hpp"
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

        class DataPoint
        {
          public:
            std::chrono::milliseconds Timestamp;
            devices::suns::MeasurementData ExperimentalSunS;
            devices::payload::PayloadTelemetry::SunsRef ReferenceSunS;
            devices::gyro::GyroscopeTelemetry Gyro;

            void WritePrimaryDataSetTo(experiments::fs::ExperimentFile& file);
            void WriteSecondaryDataSetTo(experiments::fs::ExperimentFile& file);
        };

        struct ISetupSunSExperiment
        {
            virtual void SetParameters(SunSExperimentParams parameters) = 0;
            virtual void SetOutputFiles(const char* baseName) = 0;
        };

        class SunSExperiment : public IExperiment, public ISetupSunSExperiment
        {
          public:
            SunSExperiment(services::power::IPowerControl& powerControl,
                services::time::ICurrentTime& currentTime,
                devices::suns::ISunSDriver& experimentalSunS,
                devices::payload::IPayloadDeviceDriver& payload,
                devices::gyro::IGyroscopeDriver& gyro,
                services::fs::IFileSystem& fileSystem)
                : _powerControl(powerControl), _currentTime(currentTime), _experimentalSunS(experimentalSunS), _payload(payload),
                  _gyro(gyro), _fs(fileSystem), _nextSessionAt(0)
            {
                std::strncpy(_primaryFileName, "/suns", 30);
                std::strncpy(_secondaryFileName, "/suns.sec", 30);
            }

            virtual void SetParameters(SunSExperimentParams parameters) override;
            virtual void SetOutputFiles(const char* baseName) override;

            virtual ExperimentCode Type() override;
            virtual StartResult Start() override;
            virtual IterationResult Iteration() override;
            virtual void Stop(IterationResult lastResult) override;

            DataPoint GatherSingleMeasurement();

            static constexpr experiments::ExperimentCode Code = 0x04;

          private:
            services::power::IPowerControl& _powerControl;
            services::time::ICurrentTime& _currentTime;
            devices::suns::ISunSDriver& _experimentalSunS;
            devices::payload::IPayloadDeviceDriver& _payload;
            devices::gyro::IGyroscopeDriver& _gyro;
            services::fs::IFileSystem& _fs;

            SunSExperimentParams _parameters;

            std::uint8_t _remainingSessions;
            std::chrono::milliseconds _nextSessionAt;

            char _primaryFileName[30];
            char _secondaryFileName[sizeof(_primaryFileName) + 4];

            fs::ExperimentFile _primaryDataSet;
            fs::ExperimentFile _secondaryDataSet;
        };
    }
}

#endif /* LIBS_EXPERIMENTS_SUNS_INCLUDE_EXPERIMENT_SUNS_SUNS_HPP_ */
