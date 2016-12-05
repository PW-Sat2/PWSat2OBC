#ifndef LIBS_MISSION_INCLUDE_MISSION_ANTENNA_H_
#define LIBS_MISSION_INCLUDE_MISSION_ANTENNA_H_

#include <cstdint>
#include "antenna/antenna.h"
#include "base/os.h"
#include "state/state.h"

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
             */
            AntennaMissionState(AntennaDriver& antennaDriver);

            /**
             * @brief Returns information whether the antenna deployment is currently being performed.
             *
             * @return True if the at least one antenna is being deployed, false otherwise.
             */
            bool IsDeploymentInProgress() const;

            /**
             * @brief Returns information whether the antenna deployment part of the entire process is finished.
             *
             * @return True is the deployment part of the process is finished, false otherwise.
             */
            bool IsDeploymentPartFinished() const;

            /**
             * @brief Returns information whether the entire process is finished.
             *
             * @return True is the entire deployemnt process is finished, false otherwise.
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
             */
            void NextStep();

            /**
             * @brief Requests retral of the current deployment step.
             *
             * If the current step retry count hits the limit the state of the process is updated with the same way
             * as if the NextStep method would be called.
             * @param[in] limit Step retry limit number.
             */
            void Retry(std::uint8_t limit);

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
             * @brief Returns total count of the deployment steps.
             * @return Total deployment step count.
             */
            static std::uint8_t StepCount();

            /**
             * @brief Returns total count of the steps that are strictly related to the antenna deployment.
             * @return Deployment step count.
             */
            static std::uint8_t DeploymentStepCount();

          private:
            /**
             * @brief Flag indicating whether antenna hardware status should be ignored.
             */
            bool _overrideState;

            /**
             * @brief Flag indicating whether the antennas are being curretnly deployed.
             */
            bool _inProgress;

            /**
             * @brief Current step number.
             */
            uint8_t _stepNumber;

            /**
             * @brief Current step retry count.
             */
            uint8_t _retryCount;

            /**
             * @brief Current antenna driver instance.
             */
            AntennaDriver& _driver;
        };

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

        inline void AntennaMissionState::NextStep()
        {
            ++this->_stepNumber;
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
        }

        inline void AntennaMissionState::Restart()
        {
            OverrideStep(0);
        }

        /**
         * @brief Returns antenna deployment action descriptor.
         *
         * @param[in] stateDescriptor Reference to the antenna deployment process state object.
         * @returns Action descriptor that runs antenna deployment process.
         */
        SystemActionDescriptor GetAntennaDeploymentActionDescriptor(AntennaMissionState& stateDescriptor);

        /**
         * @brief Returns antenna deployment update descriptor.
         *
         * @param[in] stateDescriptor Reference to the antenna deployment process state object.
         * @returns Update descriptor that runs antenna deployment update process.
         */
        SystemStateUpdateDescriptor GetAntennaDeploymentUpdateDescriptor(AntennaMissionState& stateDescriptor);

        /** @}*/
    }
}

#endif /* LIBS_MISSION_INCLUDE_MISSION_ANTENNA_H_ */
