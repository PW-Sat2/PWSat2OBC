#include "ImtqTelemetryCollector.hpp"

namespace devices
{
    namespace imtq
    {
        using namespace std::chrono_literals;

        ImtqTelemetryCollector::ImtqTelemetryCollector(IImtqDriver& driver) : next(driver), semaphore(nullptr)
        {
        }

        void ImtqTelemetryCollector::Initialize()
        {
            this->semaphore = System::CreateBinarySemaphore();
            System::GiveSemaphore(this->semaphore);
        }

        template <typename T> bool ImtqTelemetryCollector::Update(bool status, const T& source, T& target)
        {
            if (!status)
            {
                return status;
            }

            Lock lock(this->semaphore, 50ms);
            if (static_cast<bool>(lock))
            {
                target = source;
            }

            return status;
        }

        bool ImtqTelemetryCollector::PerformSelfTest(SelfTestResult& result, bool tryToFixIsisErrors)
        {
            const auto status = this->next.PerformSelfTest(result, tryToFixIsisErrors);
            return ProcessSelfTestResult(status, result);
        }

        bool ImtqTelemetryCollector::MeasureMagnetometer(Vector3<MagnetometerMeasurement>& result)
        {
            const auto status = this->next.MeasureMagnetometer(result);
            return Update(status, telemetry::ImtqMagnetometerMeasurements(result), this->magnetometers);
        }

        bool ImtqTelemetryCollector::SoftwareReset()
        {
            return this->next.SoftwareReset();
        }

        bool ImtqTelemetryCollector::SendNoOperation()
        {
            return this->next.SendNoOperation();
        }

        bool ImtqTelemetryCollector::CancelOperation()
        {
            return this->next.CancelOperation();
        }

        bool ImtqTelemetryCollector::StartMTMMeasurement()
        {
            return this->next.StartMTMMeasurement();
        }

        bool ImtqTelemetryCollector::StartActuationCurrent(const Vector3<Current>& current, std::chrono::milliseconds duration)
        {
            const auto status = this->next.StartActuationCurrent(current, duration);
#ifndef RAW_CURRENT
            return Update(status, telemetry::ImtqCoilCurrent{current}, this->coilCurrents);
#else
            return status;
#endif
        }

        bool ImtqTelemetryCollector::StartActuationDipole(Vector3<Dipole> dipole, std::chrono::milliseconds duration)
        {
            const auto status = this->next.StartActuationDipole(dipole, duration);
            return Update(status, telemetry::ImtqDipoles{dipole}, this->dipoles);
        }

        bool ImtqTelemetryCollector::StartAllAxisSelfTest()
        {
            return this->next.StartAllAxisSelfTest();
        }

        bool ImtqTelemetryCollector::StartBDotDetumbling(std::chrono::seconds duration)
        {
            return this->next.StartBDotDetumbling(duration);
        }

        bool ImtqTelemetryCollector::GetSystemState(State& state)
        {
            const auto status = this->next.GetSystemState(state);
            return Update(status,
                telemetry::ImtqState{0, state.mode, state.error.GetValue(), state.anyParameterUpdatedSinceStartup, state.uptime},
                this->imtqState);
        }

        bool ImtqTelemetryCollector::GetCalibratedMagnetometerData(MagnetometerMeasurementResult& result)
        {
            const auto status = this->next.GetCalibratedMagnetometerData(result);
            if (status)
            {
                Lock lock(this->semaphore, 50ms);
                if (static_cast<bool>(lock))
                {
                    this->magnetometers = telemetry::ImtqMagnetometerMeasurements{result.data};
                    this->coilsActive = telemetry::ImtqCoilsActive{result.coilActuationDuringMeasurement};
                }
            }

            return status;
        }

        bool ImtqTelemetryCollector::GetCoilCurrent(Vector3<Current>& result)
        {
            const auto status = this->next.GetCoilCurrent(result);
#ifndef RAW_CURRENT
            return Update(status, telemetry::ImtqCoilCurrent{result}, this->coilCurrents);
#else
            return status;
#endif
        }

        bool ImtqTelemetryCollector::GetCoilTemperature(Vector3<TemperatureMeasurement>& result)
        {
            const auto status = this->next.GetCoilTemperature(result);
#ifndef RAW_TEMPERATURE
            return Update(status, telemetry::ImtqCoilTemperature{result}, this->coilTemperatures);
#else
            return status;
#endif
        }

        bool ImtqTelemetryCollector::GetSelfTestResult(SelfTestResult& result)
        {
            const auto status = this->next.GetSelfTestResult(result);
            return ProcessSelfTestResult(status, result);
        }

        bool ImtqTelemetryCollector::GetDetumbleData(DetumbleData& result)
        {
            const auto status = this->next.GetDetumbleData(result);
            if (status)
            {
                Lock lock(this->semaphore, 50ms);
                if (static_cast<bool>(lock))
                {
                    this->magnetometers = telemetry::ImtqMagnetometerMeasurements{result.calibratedMagnetometerMeasurement};
                    this->bdot = telemetry::ImtqBDotTelemetry{result.bDotData};
                    this->dipoles = telemetry::ImtqDipoles{result.commandedDipole};
                    this->coilCurrents = telemetry::ImtqCoilCurrent{result.measuredCurrent};
                }
            }

            return status;
        }

        bool ImtqTelemetryCollector::GetHouseKeepingRAW(HouseKeepingRAW& result)
        {
            const auto status = this->next.GetHouseKeepingRAW(result);
            if (status)
            {
                Lock lock(this->semaphore, 50ms);
                if (static_cast<bool>(lock))
                {
                    this->houseKeeping = telemetry::ImtqHousekeeping(result.digitalVoltage, //
                        result.analogVoltage,                                               //
                        result.digitalCurrent,                                              //
                        result.analogCurrent,                                               //
                        result.MCUtemperature                                               //
                        );
#ifdef RAW_CURRENT
                    this->coilCurrents = telemetry::ImtqCoilCurrent{result.coilCurrent};
#endif
#ifdef RAW_TEMPERATURE
                    this->coilTemperatures = telemetry::ImtqCoilTemperature{result.coilTemperature};
#endif
                }
            }

            return status;
        }

        bool ImtqTelemetryCollector::GetHouseKeepingEngineering(HouseKeepingEngineering& result)
        {
            const auto status = this->next.GetHouseKeepingEngineering(result);
#if !defined(RAW_CURRENT) && !defined(RAW_TEMPERATURE)
            if (status)
            {
                Lock lock(this->semaphore, 50ms);
                if (static_cast<bool>(lock))
                {
#ifndef RAW_CURRENT
                    this->coilCurrents = telemetry::ImtqCoilCurrent{result.coilCurrent};
#endif
#ifndef RAW_TEMPERATURE
                    this->coilTemperatures = telemetry::ImtqCoilTemperature{result.coilTemperature};
#endif
                }
            }
#endif

            return status;
        }

        bool ImtqTelemetryCollector::GetParameter(Parameter id, gsl::span<std::uint8_t> result)
        {
            return this->next.GetParameter(id, result);
        }

        bool ImtqTelemetryCollector::SetParameter(Parameter id, gsl::span<const std::uint8_t> value)
        {
            return this->next.SetParameter(id, value);
        }

        bool ImtqTelemetryCollector::ResetParameterAndGetDefault(Parameter id, gsl::span<std::uint8_t> result)
        {
            return this->next.ResetParameterAndGetDefault(id, result);
        }

        bool ImtqTelemetryCollector::ProcessSelfTestResult(bool status, const SelfTestResult& result)
        {
            if (status)
            {
                std::array<std::uint8_t, 8> testResult;
                for (size_t cx = 0; cx < result.stepResults.size(); ++cx)
                {
                    testResult[cx] = result.stepResults[cx].error.GetValue();
                }

                Update(status, telemetry::ImtqSelfTest(testResult), this->selfTest);
            }

            return status;
        }
    }
}
