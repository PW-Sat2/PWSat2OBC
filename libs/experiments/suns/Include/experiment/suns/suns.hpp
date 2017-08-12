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

namespace experiment
{
    namespace suns
    {
        /**
         * @ingroup experiments
         * @{
         */

        /**
         * @brief SunS experiment parameters
         */
        class SunSExperimentParams
        {
          public:
            SunSExperimentParams();

            /**
             * @brief Ctor
             * @param gain Gain
             * @param itime ITime
             * @param samplesCount Samples count
             * @param shortDelay Short delay
             * @param samplingSessionsCount Sampling sessions count
             * @param longDelay Long delay
             */
            SunSExperimentParams(std::uint8_t gain,
                std::uint8_t itime,
                std::uint8_t samplesCount,
                std::chrono::seconds shortDelay,
                std::uint8_t samplingSessionsCount,
                std::chrono::minutes longDelay);

            /**
             * @brief Returns gain value
             * @return Gain value
             */
            inline std::uint8_t Gain() const;
            /**
             * @brief Returns itime value
             * @return ITime value
             */
            inline std::uint8_t ITime() const;
            /**
             * @brief Returns samples count
             * @return Samples count
             */
            inline std::uint8_t SamplesCount() const;
            /**
             * @brief Returns short delay duration
             * @return Short delau duration
             */
            inline std::chrono::seconds ShortDelay() const;
            /**
             * @brief Returns sampling sessions count
             * @return Sampling sessions count
             */
            inline std::uint8_t SamplingSessionsCount() const;
            /**
             * @brief Returns long delay duration
             * @return Long delay durations
             */
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

        /**
         * @brief SunS experiment data point
         */
        class DataPoint final
        {
          public:
            /** @brief Timestamp */
            std::chrono::milliseconds Timestamp;
            /** @brief Data from experimental SunS */
            devices::suns::MeasurementData ExperimentalSunS;
            /** @brief Data from reference SunS */
            devices::payload::PayloadTelemetry::SunsRef ReferenceSunS;
            /** @brief Data from gyroscope */
            devices::gyro::GyroscopeTelemetry Gyro;

            /**
             * @brief Writes data point to primary data set file
             * @param file File for primary data sets
             */
            void WritePrimaryDataSetTo(experiments::fs::ExperimentFile& file);
            /**
             * @brief Writes data point to secondary data set file
             * @param file File for secondary data sets
             */
            void WriteSecondaryDataSetTo(experiments::fs::ExperimentFile& file);

            /**
             * @brief Writes time stamp to writer
             * @param writer Writer to hold the written data
             */
            void WriteTimeStamp(Writer& writer);

            /**
             * @brief Writes primary experimental suns data to writer
             * @param writer Writer to hold the written data
             */
            void WritePrimaryExperimentalSunS(Writer& writer);

            /**
             * @brief Writes secondary experimental suns data to writer
             * @param writer Writer to hold the written data
             */
            void WriteSecondaryExperimentalSunS(Writer& writer);

            /**
             * @brief Writes reference suns data to writer
             * @param writer Writer to hold the written data
             */
            void WriteReferenceSunS(Writer& writer);

            /**
             * @brief Writes gyro data to writer
             * @param writer Writer to hold the written data
             */
            void WriteGyro(Writer& writer);
        };

        /**
         * @brief Interface for setting-up SunS experiment parameters
         */
        struct ISetupSunSExperiment
        {
            /**
             * @brief Sets experiment parameters
             * @param parameters Parameters
             */
            virtual void SetParameters(SunSExperimentParams parameters) = 0;

            /**
             * @brief Sets base name for output files
             * @param baseName Base name for output files
             *
             * @remark String is copied to internal buffer
             * @remark If string is longer than internal buffer size, it is trimmed to maximum size
             */
            virtual void SetOutputFiles(const char* baseName) = 0;
        };

        /**
         * @brief SunS experiment
         */
        class SunSExperiment : public experiments::IExperiment, public ISetupSunSExperiment
        {
          public:
            /**
             * @brief Ctor
             * @param powerControl Power control interface
             * @param currentTime Current time provider
             * @param experimentalSunS Experimental SunS driver
             * @param payload Payload (reference SunS) driver
             * @param gyro Gyroscope driver
             * @param fileSystem File system
             */
            SunSExperiment(services::power::IPowerControl& powerControl,
                services::time::ICurrentTime& currentTime,
                devices::suns::ISunSDriver& experimentalSunS,
                devices::payload::IPayloadDeviceDriver& payload,
                devices::gyro::IGyroscopeDriver& gyro,
                services::fs::IFileSystem& fileSystem);

            virtual void SetParameters(SunSExperimentParams parameters) override;
            virtual void SetOutputFiles(const char* baseName) override;

            virtual experiments::ExperimentCode Type() override;
            virtual experiments::StartResult Start() override;
            virtual experiments::IterationResult Iteration() override;
            virtual void Stop(experiments::IterationResult lastResult) override;

            /**
             * @brief Gathers single data point from all sensors
             * @return Data point
             */
            DataPoint GatherSingleMeasurement();

            /** @brief Experiment code */
            static constexpr experiments::ExperimentCode Code = 0x3;

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

            experiments::fs::ExperimentFile _primaryDataSet;
            experiments::fs::ExperimentFile _secondaryDataSet;
        };

        /** @} */
    }
}

#endif /* LIBS_EXPERIMENTS_SUNS_INCLUDE_EXPERIMENT_SUNS_SUNS_HPP_ */
