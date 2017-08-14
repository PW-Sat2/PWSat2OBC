#include "adcs.hpp"
#include "data_point.hpp"
#include "gyro/driver.hpp"
#include "logger/logger.h"
#include "power/power.h"

using experiments::IterationResult;
using experiments::StartResult;

namespace experiment
{
    namespace adcs
    {
        DetumblingExperiment::DetumblingExperiment(::adcs::IAdcsCoordinator& adcs,
            services::time::ICurrentTime& time,
            services::power::IPowerControl& powerControl,
            devices::gyro::IGyroscopeDriver& gyro,
            devices::payload::IPayloadDeviceDriver& payload,
            telemetry::IImtqDataProvider& imtq)
            : _adcs(adcs), _time(time), _powerControl(powerControl), _gyro(gyro), _payload(payload), _imtq(imtq)
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

            auto revert = OnLeave([this]() {
                auto r = this->_adcs.EnableBuiltinDetumbling();

                if (OS_RESULT_FAILED(r))
                {
                    LOGF(LOG_LEVEL_FATAL, "Failed to reenable builtin detumbling (%d)", num(r));
                }
            });

            r = this->_adcs.EnableExperimentalDetumbling();

            if (OS_RESULT_FAILED(r))
            {
                LOGF(LOG_LEVEL_ERROR, "Failed to enable experimental detumbling (%d)", num(r));

                return StartResult::Failure;
            }

            auto start = this->_time.GetCurrentTime();

            if (!start.HasValue)
            {
                return StartResult::Failure;
            }

            if (!this->_powerControl.SensPower(true))
            {
                return StartResult::Failure;
            }

            this->_endAt = start.Value + this->_duration;

            revert.Skip();

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
            this->_powerControl.SensPower(false);

            auto r = this->_adcs.Disable();

            if (OS_RESULT_FAILED(r))
            {
                LOGF(LOG_LEVEL_FATAL, "Failed to reenable builtin detumbling (%d)", num(r));
                return;
            }

            this->_adcs.EnableBuiltinDetumbling();
        }

        DetumblingDataPoint DetumblingExperiment::GatherSingleMeasurement()
        {
            DetumblingDataPoint point;

            point.Timestamp = this->_time.GetCurrentTime().Value;
            point.Gyro = this->_gyro.read().Value;

            this->_payload.MeasureSunSRef(point.ReferenceSunS);
            this->_payload.MeasurePhotodiodes(point.Photodiodes);
            this->_payload.MeasureTemperatures(point.Temperatures);

            this->_imtq.GetLastMagnetometerMeasurement(point.Magnetometer);

            this->_imtq.GetLastDipoles(point.Dipoles);

            return point;
        }
    }
}
