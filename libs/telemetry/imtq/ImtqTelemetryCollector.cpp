#include "ImtqTelemetryCollector.hpp"
#include "telemetry/state.hpp"

namespace devices
{
    namespace imtq
    {
        using namespace std::chrono_literals;

        ImtqTelemetryCollector::ImtqTelemetryCollector(IImtqDriver& driver) : next(driver), semaphore(nullptr)
        {
            this->elementUpdated.fill(false);
        }

        void ImtqTelemetryCollector::Initialize()
        {
            this->semaphore = System::CreateBinarySemaphore();
            System::GiveSemaphore(this->semaphore);
        }

        template <typename T> bool ImtqTelemetryCollector::Update(bool status, ElementId element, const T& source, T& target)
        {
            if (!status)
            {
                return status;
            }

            Lock lock(this->semaphore, 50ms);
            if (static_cast<bool>(lock))
            {
                target = source;
                this->elementUpdated[num(element)] = true;
            }

            return status;
        }

        template <typename T> void ImtqTelemetryCollector::Save(const T& source, ElementId element, telemetry::ManagedTelemetry& target)
        {
            if (this->elementUpdated[num(element)])
            {
                target.Set(source);
                this->elementUpdated[num(element)] = false;
            }
        }

        bool ImtqTelemetryCollector::PerformSelfTest(SelfTestResult& result, bool tryToFixIsisErrors)
        {
            const auto status = this->next.PerformSelfTest(result, tryToFixIsisErrors);
            return ProcessSelfTestResult(status, result);
        }

        bool ImtqTelemetryCollector::MeasureMagnetometer(Vector3<MagnetometerMeasurement>& result)
        {
            const auto status = this->next.MeasureMagnetometer(result);
            return Update(status, ElementId::Magnetometer, telemetry::ImtqMagnetometerMeasurements(result), this->magnetometers);
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
            return this->next.StartActuationCurrent(current, duration);
        }

        bool ImtqTelemetryCollector::StartActuationDipole(Vector3<Dipole> dipole, std::chrono::milliseconds duration)
        {
            const auto status = this->next.StartActuationDipole(dipole, duration);
            return Update(status, ElementId::Dipoles, telemetry::ImtqDipoles{dipole}, this->dipoles);
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
            if (status)
            {
                Lock lock(this->semaphore, 50ms);
                if (static_cast<bool>(lock))
                {
                    this->imtqState =
                        telemetry::ImtqState{state.mode, state.error.GetValue(), state.anyParameterUpdatedSinceStartup, state.uptime};
                    this->imtqStatus = telemetry::ImtqStatus{state.status};
                    this->elementUpdated[num(ElementId::State)] = true;
                    this->elementUpdated[num(ElementId::Status)] = true;
                }
            }

            return status;
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
                    this->elementUpdated[num(ElementId::Magnetometer)] = true;
                    this->elementUpdated[num(ElementId::CoilsActive)] = true;
                }
            }

            return status;
        }

        bool ImtqTelemetryCollector::GetCoilCurrent(Vector3<Current>& result)
        {
            const auto status = this->next.GetCoilCurrent(result);
            return Update(status, ElementId::CoilCurrents, telemetry::ImtqCoilCurrent{result}, this->coilCurrents);
        }

        bool ImtqTelemetryCollector::GetCoilTemperature(Vector3<TemperatureMeasurement>& result)
        {
            const auto status = this->next.GetCoilTemperature(result);
            return Update(status, ElementId::CoilTemperatures, telemetry::ImtqCoilTemperature{result}, this->coilTemperatures);
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
                    this->elementUpdated[num(ElementId::Magnetometer)] = true;
                    this->elementUpdated[num(ElementId::Bdot)] = true;
                    this->elementUpdated[num(ElementId::Dipoles)] = true;
                    this->elementUpdated[num(ElementId::CoilCurrents)] = true;
                }
            }

            return status;
        }

        bool ImtqTelemetryCollector::GetHouseKeepingRAW(HouseKeepingRAW& result)
        {
            return this->next.GetHouseKeepingRAW(result);
        }

        bool ImtqTelemetryCollector::GetHouseKeepingEngineering(HouseKeepingEngineering& result)
        {
            const auto status = this->next.GetHouseKeepingEngineering(result);
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

                    this->imtqStatus = telemetry::ImtqStatus(result.status);
                    this->coilCurrents = telemetry::ImtqCoilCurrent{result.coilCurrent};
                    this->coilTemperatures = telemetry::ImtqCoilTemperature{result.coilTemperature};
                    this->elementUpdated[num(ElementId::HouseKeeping)] = true;
                    this->elementUpdated[num(ElementId::Status)] = true;
                    this->elementUpdated[num(ElementId::CoilCurrents)] = true;
                    this->elementUpdated[num(ElementId::CoilTemperatures)] = true;
                }
            }

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

                Update(status, ElementId::SelfTest, telemetry::ImtqSelfTest(testResult), this->selfTest);
            }

            return status;
        }

        bool ImtqTelemetryCollector::CaptureTelemetry(telemetry::ManagedTelemetry& target)
        {
            Lock lock(this->semaphore, 50ms);
            const auto status = static_cast<bool>(lock);
            if (status)
            {
                Save(this->magnetometers, ElementId::Magnetometer, target);
                Save(this->dipoles, ElementId::Dipoles, target);
                Save(this->bdot, ElementId::Bdot, target);
                Save(this->houseKeeping, ElementId::HouseKeeping, target);
                Save(this->coilCurrents, ElementId::CoilCurrents, target);
                Save(this->coilTemperatures, ElementId::CoilTemperatures, target);
                Save(this->imtqStatus, ElementId::Status, target);
                Save(this->imtqState, ElementId::State, target);
                Save(this->selfTest, ElementId::SelfTest, target);
                Save(this->coilsActive, ElementId::CoilsActive, target);
            }

            return status;
        }
    }
}
