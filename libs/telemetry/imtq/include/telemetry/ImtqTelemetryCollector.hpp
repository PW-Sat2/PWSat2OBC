#ifndef LIBS_DRIVERS_IMTQ_INCLUDE_IMTQ_TELEMETRY_HPP
#define LIBS_DRIVERS_IMTQ_INCLUDE_IMTQ_TELEMETRY_HPP

#pragma once

#include "IImtqTelemetryCollector.hpp"
#include "base/os.h"
#include "imtq/IImtqDriver.hpp"
#include "telemetry/ImtqTelemetry.hpp"
#include "telemetry/fwd.hpp"

namespace devices
{
    namespace imtq
    {
        /**
         * @brief This class is responsible for capturing imtq telemetry as it is extracted from the imtq hardware.
         *
         * This class is a wrapper for imtq driver, that should be used as proxy for all imtq requests.
         * All of the necessary telemetry elements are extracted as they are seen and saved in members
         * of this class. They can be later extracted using dedicated method.
         * @ingroup ImtqDriver
         */
        class ImtqTelemetryCollector final : public IImtqDriver, public telemetry::IImtqTelemetryCollector
        {
          public:
            /**
             * @brief Enumerator of all captured telemetry elements.
             */
            enum class ElementId
            {
                Magnetometer = 0, //!< Magnetometer
                Dipoles,          //!< Dipoles
                Bdot,             //!< Bdot
                HouseKeeping,     //!< HouseKeeping
                CoilCurrents,     //!< CoilCurrents
                CoilTemperatures, //!< CoilTemperatures
                Status,           //!< Status
                State,            //!< State
                SelfTest,         //!< SelfTest
                CoilsActive,      //!< CoilsActive
                Last,             //!< Last
            };

            /**
             * @brief ctor.
             * @param driver Reference to the actual imtq driver.
             */
            ImtqTelemetryCollector(IImtqDriver& driver);

            /**
             * @brief Initializes this object & prepares it to work.
             */
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

            virtual bool CaptureTelemetry(telemetry::ManagedTelemetry& target) final override;

            /**
             * @brief Verifies whether the requested telemetry element has been recently captured
             * @param id Telemetry element id.
             * @return True
             */
            bool IsNew(ElementId id) const;

          private:
            template <typename T> bool Update(bool status, ElementId element, const T& source, T& target);

            template <typename T> void Save(const T& source, ElementId element, telemetry::ManagedTelemetry& target);

            bool ProcessSelfTestResult(bool stauts, const SelfTestResult& result);

            IImtqDriver& next;
            OSSemaphoreHandle semaphore;
            telemetry::ImtqMagnetometerMeasurements magnetometers;
            telemetry::ImtqDipoles dipoles;
            telemetry::ImtqBDotTelemetry bdot;
            telemetry::ImtqHousekeeping houseKeeping;
            telemetry::ImtqCoilCurrent coilCurrents;
            telemetry::ImtqCoilTemperature coilTemperatures;
            telemetry::ImtqStatus imtqStatus;
            telemetry::ImtqState imtqState;
            telemetry::ImtqSelfTest selfTest;
            telemetry::ImtqCoilsActive coilsActive;
            std::array<bool, num(ElementId::Last)> elementUpdated;
        };

        inline bool ImtqTelemetryCollector::IsNew(ElementId id) const
        {
            const auto index = num(id);
            if (index < 0 || index >= num(ElementId::Last))
            {
                return false;
            }

            return this->elementUpdated[index];
        }
    }
}

#endif
