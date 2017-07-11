#ifndef LIBS_MISSION_INCLUDE_MISSION_ANTENNA_H_
#define LIBS_MISSION_INCLUDE_MISSION_ANTENNA_H_

#include <cstdint>
#include "antenna/antenna.h"
#include "antenna/telemetry.hpp"
#include "base/os.h"
#include "mission/base.hpp"
#include "power/power.h"
#include "state/struct.h"

namespace mission
{
    namespace antenna
    {
        /**
         * @defgroup mission_atenna Antenna Deployment
         * @ingroup mission
         * @brief Module that contains logic related to antenna deployment just after  the initial mission silent period.
         *
         * This module contains only the logic that is related to the antenna deployment with various retry mechanisms.
         * However it does not contain any logic related to communication initialization or setting up the system for further
         * mission operations.
         *
         * @{
         */

        /**
         * @brief Class that contains the logic related to keeping track of the antenna deployment process progress.
         *
         * The antenna deployment process is composed of several parts. They are run sequentially from the beginning
         * until one of the following conditions are true:
         * - At least one antenna controller says that all antennas are deployed,
         * - There are no more steps to performs.
         *
         */
        class AntennaMissionState final
        {
          public:
            /**
             * @brief ctor.
             * @param[in] antennaDriver Reference to the instance of the antenna driver that is supposed to
             * drive the required hardware.
             * @param[in] powerControl Power control
             */
            AntennaMissionState(AntennaDriver& antennaDriver, services::power::IPowerControl& powerControl);

            /**
             * @brief Initializes antenna mission state
             */
            void Initialize();

            /**
             * @brief Returns information whether the antenna deployment is currently being performed.
             *
             * @return True if the at least one antenna is being deployed, false otherwise.
             */
            bool IsDeploymentInProgress() const;

            /**
             * @brief Returns information whether the entire process is finished.
             *
             * @return True is the entire deployment process is finished, false otherwise.
             */
            bool IsFinished() const;

            /**
             * @brief Returns information whether the override of the deployment state returned by the
             * hardware should be ignored and treated as suspicious.
             *
             * @return True if the hardware deployment state should be ignored, false otherwise.
             */
            bool OverrideState() const;

            /**
             * @brief Returns current deployment state step number.
             *
             * @return Zero based step number.
             */
            std::uint8_t StepNumber() const;

            /**
             * @brief Requests advancing the deployment process to the next step.
             * @param[in] timeout Current operation timeout in mission iterations.
             */
            void NextStep(std::int8_t timeout);

            /**
             * @brief Requests retral of the current deployment step.
             *
             * If the current step retry count hits the limit the state of the process is updated with the same way
             * as if the NextStep method would be called.
             * @param[in] limit Step retry limit number.
             * @param[in] timeout Current operation timeout in mission iterations.
             */
            void Retry(std::uint8_t limit, std::int8_t timeout);

            /**
             * @brief Returns current retry counter;
             * @return Value of current retry counter.
             */
            std::int8_t RetryCount() const;

            /**
             * @brief Returns reference to current antenna driver.
             * @return Reference to instance of antenna driver being used.
             */
            AntennaDriver& Driver();

            /**
             * @brief Requests overriding the antenna deployment state reported by the hardware.
             *
             * Calling this method will cause the deployment process to run to the end regardless of the
             * status reported by the underlying hardware.
             */
            void OverrideDeploymentState();

            /**
             * @brief Overrides the current deployment step number.
             *
             * Use this method with caution as this method can be used to immediately stop the deployment process
             * without any post process cleanup.
             *
             * @param[in] stepNumber New step number.
             */
            void OverrideStep(std::uint8_t stepNumber);

            /**
             * @brief Requests updating internal state with provided antenna deployment status.
             * @param[in] status Current antenna deployment status.
             */
            void Update(const AntennaDeploymentStatus& status);

            /**
             * @brief Requests restarting the deployment process.
             */
            void Restart();

            /**
             * @brief Immediately finishes the antenna deployment process.
             */
            void Finish();

            /**
             * @brief Sets current operation timeout in mission iterations.
             * @param cycleCount Number of mission iterations before operation times out.
             */
            void SetTimeout(std::int8_t cycleCount);

            /**
             * @brief Returns current timeout length in mission iterations
             * @return Current timeout length in mission iterations.
             */
            std::int8_t GetTimeout() const;

            /**
             * @brief Mission iteration notification procedure.
             */
            void NextCycle();

            /**
             * @brief Queries the state whether the current operation has timed out.
             * @return True in case of timeout, false otherwise.
             */
            bool TimedOut() const;

            /**
             * @brief Returns total count of the deployment steps.
             * @return Total deployment step count.
             */
            static std::uint8_t StepCount();

            /**
             * @brief Fetches current antenna telemetry
             * @param result Reference to object that will be filled with telemetry
             * @return true if telemetry was fetched correctly, false otherwise
             */
            bool CurrentTelemetry(devices::antenna::AntennaTelemetry& result) const;

            /**
             * @brief Updates stored antenna telementry
             * @return true if telemetry was fetched correctly, false otherwise
             */
            bool UpdateTelemetry();

            /** @brief Power control  */
            services::power::IPowerControl& Power;

          private:
            /** @brief Telemetry synchronization semaphore */
            OSSemaphoreHandle _telemetrySync;

            /** @brief Current antenna telemetry */
            devices::antenna::AntennaTelemetry _currentTelemetry;

            /**
             * @brief Flag indicating whether antenna hardware status should be ignored.
             */
            bool _overrideState;

            /**
             * @brief Flag indicating whether the antennas are being currently deployed.
             */
            bool _inProgress;

            /**
             * @brief Current step number.
             */
            std::uint8_t _stepNumber;

            /**
             * @brief Current step retry count.
             */
            std::uint8_t _retryCount;

            /**
             * @brief Number of mission iteration cycles before current operation times out.
             */
            std::int8_t _cycleCount;

            /**
             * @brief Current antenna driver instance.
             */
            AntennaDriver& _driver;
        };

        inline std::int8_t AntennaMissionState::RetryCount() const
        {
            return this->_retryCount;
        }

        inline bool AntennaMissionState::IsDeploymentInProgress() const
        {
            return this->_inProgress;
        }

        inline bool AntennaMissionState::OverrideState() const
        {
            return this->_overrideState;
        }

        inline std::uint8_t AntennaMissionState::StepNumber() const
        {
            return this->_stepNumber;
        }

        inline void AntennaMissionState::NextStep(std::int8_t timeout)
        {
            ++this->_stepNumber;
            this->_retryCount = 0;
            SetTimeout(timeout);
        }

        inline AntennaDriver& AntennaMissionState::Driver()
        {
            return this->_driver;
        }

        inline void AntennaMissionState::OverrideDeploymentState()
        {
            this->_overrideState = true;
        }

        inline void AntennaMissionState::OverrideStep(std::uint8_t stepNumber)
        {
            this->_stepNumber = stepNumber;
            this->_retryCount = 0;
            SetTimeout(0);
        }

        inline void AntennaMissionState::Restart()
        {
            OverrideStep(0);
        }

        inline void AntennaMissionState::Finish()
        {
            OverrideStep(StepCount());
        }

        inline void AntennaMissionState::SetTimeout(std::int8_t cycleCount)
        {
            this->_cycleCount = cycleCount;
        }

        inline std::int8_t AntennaMissionState::GetTimeout() const
        {
            return this->_cycleCount;
        }

        inline void AntennaMissionState::NextCycle()
        {
            --this->_cycleCount;
        }

        inline bool AntennaMissionState::TimedOut() const
        {
            return this->_cycleCount <= 0;
        }

        /** @}*/
    }
}

#endif /* LIBS_MISSION_INCLUDE_MISSION_ANTENNA_H_ */
