#ifndef LIBS_DRIVERS_IMTQ_INCLUDE_IMTQ_TELEMETRY_HPP
#define LIBS_DRIVERS_IMTQ_INCLUDE_IMTQ_TELEMETRY_HPP

#pragma once

#include "base/os.h"
#include "imtq/IImtqDriver.hpp"
#include "telemetry/ImtqTelemetry.hpp"

namespace devices
{
    namespace imtq
    {
        class ImtqTelemetryCollector final : public IImtqDriver
        {
          public:
            ImtqTelemetryCollector(IImtqDriver& driver);

            void Initialize();

            virtual bool PerformSelfTest(SelfTestResult& result, bool tryToFixIsisErrors) final override;

            virtual bool MeasureMagnetometer(Vector3<MagnetometerMeasurement>& result) final override;

            virtual bool SoftwareReset() final override;

            virtual bool SendNoOperation() final override;

            virtual bool CancelOperation() final override;

            virtual bool StartMTMMeasurement() final override;

            virtual bool StartActuationCurrent(const Vector3<Current>& current, std::chrono::milliseconds duration) final override;

            virtual bool StartActuationDipole(Vector3<Dipole> dipole, std::chrono::milliseconds duration) final override;

            virtual bool StartAllAxisSelfTest() final override;

            virtual bool StartBDotDetumbling(std::chrono::seconds duration) final override;

            virtual bool GetSystemState(State& state) final override;

            virtual bool GetCalibratedMagnetometerData(MagnetometerMeasurementResult& result) final override;

            virtual bool GetCoilCurrent(Vector3<Current>& result) final override;

            virtual bool GetCoilTemperature(Vector3<TemperatureMeasurement>& result) final override;

            virtual bool GetSelfTestResult(SelfTestResult& result) final override;

            virtual bool GetDetumbleData(DetumbleData& result) final override;

            virtual bool GetHouseKeepingRAW(HouseKeepingRAW& result) final override;

            virtual bool GetHouseKeepingEngineering(HouseKeepingEngineering& result) final override;

            virtual bool GetParameter(Parameter id, gsl::span<std::uint8_t> result) final override;

            virtual bool SetParameter(Parameter id, gsl::span<const std::uint8_t> value) final override;

            virtual bool ResetParameterAndGetDefault(Parameter id, gsl::span<std::uint8_t> result) final override;

          private:
            template <typename T> bool Update(bool status, const T& source, T& target);

            bool ProcessSelfTestResult(bool stauts, const SelfTestResult& result);
            IImtqDriver& next;
            OSSemaphoreHandle semaphore;
            telemetry::ImtqMagnetometerMeasurements magnetometers;
            telemetry::ImtqDipoles dipoles;
            telemetry::ImtqBDotTelemetry bdot;
            telemetry::ImtqHousekeeping houseKeeping;
            telemetry::ImtqCoilCurrent coilCurrents;
            telemetry::ImtqCoilTemperature coilTemperatures;
            telemetry::ImtqState imtqState;
            telemetry::ImtqSelfTest selfTest;
            telemetry::ImtqCoilsActive coilsActive;
        };
    }
}

#endif
