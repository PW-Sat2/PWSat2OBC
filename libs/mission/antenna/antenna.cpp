#include <cstdint>
#include <cstring>
#include "antenna/driver.h"
#include "antenna_state.h"
#include "antenna_task.hpp"
#include "gsl/gsl_util"
#include "mission/base.hpp"
#include "system.h"
#include "time/TimePoint.h"

namespace mission
{
    namespace antenna
    {
        /**
         * @addtogroup mission_atenna
         * @{
         */
        /**
         * @brief Type definition of the specific deployment step handler.
         *
         * @param[in] state Reference to global satellite state.
         * @param[in] stateDescriptor Reference to deployment process state.
         * @param[in] driver Reference to current antenna driver instance
         */
        typedef void DeploymentProcedure(const SystemState& state, //
            AntennaMissionState& stateDescriptor,
            AntennaDriver& driver //
            );

        /**
         * @brief This deployment step performs regular deployment operation.
         *
         * @param[in] state Reference to global satellite state.
         * @param[in] stateDescriptor Reference to the deployment process state object.
         * @param[in] driver Reference to current antenna driver instance.
         */
        static void RegularDeploymentStep(const SystemState& state, //
            AntennaMissionState& stateDescriptor,
            AntennaDriver& driver //
            );

        /**
         * @brief This deployment step resets current hardware channel.
         *
         * @param[in] state Reference to global satellite state.
         * @param[in] stateDescriptor Reference to the deployment process state object.
         * @param[in] driver Reference to current antenna driver instance.
         */
        static void ResetDriverStep(const SystemState& state, //
            AntennaMissionState& stateDescriptor,
            AntennaDriver& driver //
            );

        /**
         * @brief This deployment step finalizes antenna deployment process.
         *
         * @param[in] state Reference to global satellite state.
         * @param[in] stateDescriptor Reference to the deployment process state object.
         * @param[in] driver Reference to current antenna driver instance.
         */
        static void FinishDeploymentStep(const SystemState& state, //
            AntennaMissionState& stateDescriptor,
            AntennaDriver& driver //
            );

        /**
         * @brief Deployment step retry limit.
         *
         * This value controls how many times each deployment step will be repeated
         * in case of errors before process can advance further.
         */
        static constexpr std::uint8_t StepRetryLimit = 3;

        /**
         * @brief Hardware operation retry limit.
         *
         * This value controls how many times each hardware operation will be issued
         * in case of errors before declaring it as a failure.
         */
        static constexpr std::uint8_t RetryLimit = 3;

        /**
         * @brief Class that describes single deployment step.
         *
         * The entire process is driven by the array of steps, each described by the object of this type.
         */
        struct AntennaDeploymentStep final
        {
            /**
             * @brief Pointer to the procedure the performs current step.
             */
            DeploymentProcedure* procedure;

            /**
             * @brief Hardware channel that should be used to perform this step.
             */
            AntennaChannel channel;

            /**
             * @brief Identifier of the antenna that should be used/affected by this step.
             */
            AntennaId antennaId;

            /**
             * @brief Antenna deployment process timeout in seconds.
             */
            uint8_t deploymentTimeout;

            /**
             * @brief Flag indicating whether hardware deployment switches should be overriden.
             */
            bool overrideSwitches;
        };

        /**
         * @brief Default timeout value that should be enough in case there are no problems.
         */
        constexpr std::uint8_t DefaultTimeout = 9;

        /**
         * @brief Extended timeout that should take care of the most of the problems.
         */
        constexpr std::uint8_t MediumTimeout = 19;

        /**
         * @brief Long timeouts for heating problems.
         */
        constexpr std::uint8_t LongTimeout = 39;

        /**
         * @brief Last resort timeout in case everything else fails.
         */
        constexpr std::uint8_t EmergencyTimeout = 59;

        /**
         * @brief Array of antenna deployment steps.
         *
         * The entire process is composed of steps that are run in sequence from the beginning. Steps themselves are
         * grouped into series that together form logical variant of the antenna deployment process.
         *
         * Following is the list of the deployment stages that are defined in this array:
         * - Automatic deployment (primary hardware channel)
         * - Automatic deployment (backup hardware channel)
         * - Manual deployment (primary hardware channel)
         * - Manual deployment (backup hardware channel)
         * - Manual deployment with increased deployment timeout (primary hardware channel)
         * - Manual deployment with increased deployment timeout (backup hardware channel)
         * - Manual deployment with overridden deployment switches (primary hardware channel)
         * - Manual deployment with overridden deployment switches (backup hardware channel)
         * - Deployment finalization (primary hardware channel)
         * - Deployment finalization (backup hardware channel)
         */
        static const AntennaDeploymentStep deploymentSteps[] = {
            {ResetDriverStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA_AUTO_ID, 0, false},
            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA_AUTO_ID, DefaultTimeout, false},

            {ResetDriverStep, ANTENNA_BACKUP_CHANNEL, ANTENNA_AUTO_ID, 0, false},
            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA_AUTO_ID, DefaultTimeout, false},

            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA1_ID, DefaultTimeout, false},
            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA2_ID, DefaultTimeout, false},
            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA3_ID, DefaultTimeout, false},
            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA4_ID, DefaultTimeout, false},

            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA1_ID, DefaultTimeout, false},
            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA2_ID, DefaultTimeout, false},
            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, DefaultTimeout, false},
            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA4_ID, DefaultTimeout, false},

            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA1_ID, MediumTimeout, false},
            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA2_ID, MediumTimeout, false},
            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA3_ID, MediumTimeout, false},
            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA4_ID, MediumTimeout, false},

            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA1_ID, MediumTimeout, false},
            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA2_ID, MediumTimeout, false},
            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, MediumTimeout, false},
            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA4_ID, MediumTimeout, false},

            {ResetDriverStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA_AUTO_ID, 0, false},
            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA1_ID, LongTimeout, true},
            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA2_ID, LongTimeout, true},
            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA3_ID, LongTimeout, true},
            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA4_ID, LongTimeout, true},

            {ResetDriverStep, ANTENNA_BACKUP_CHANNEL, ANTENNA_AUTO_ID, 0, false},
            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA1_ID, LongTimeout, true},
            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA2_ID, LongTimeout, true},
            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, LongTimeout, true},
            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA4_ID, LongTimeout, true},

            {ResetDriverStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA_AUTO_ID, 0, false},
            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA1_ID, EmergencyTimeout, true},
            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA2_ID, EmergencyTimeout, true},
            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA3_ID, EmergencyTimeout, true},
            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA4_ID, EmergencyTimeout, true},

            {ResetDriverStep, ANTENNA_BACKUP_CHANNEL, ANTENNA_AUTO_ID, 0, false},
            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA1_ID, EmergencyTimeout, true},
            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA2_ID, EmergencyTimeout, true},
            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, EmergencyTimeout, true},
            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA4_ID, EmergencyTimeout, true},
            {FinishDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA_AUTO_ID, 0, false},
            {FinishDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA_AUTO_ID, 0, false},
        };

        /**
         * @brief Number of steps in the antenna deployment process.
         */
        static constexpr uint8_t DeploymentStepLimit = count_of(deploymentSteps);

        /**
         * @brief Number of the first step in the deployment finalization phase.
         */
        static constexpr uint8_t FinalizationStepIndex = count_of(deploymentSteps) - 2;

        static_assert(DeploymentStepLimit > FinalizationStepIndex, "There should be at least one finalization step");

        AntennaMissionState::AntennaMissionState(AntennaDriver& antennaDriver)
            : _overrideState(false), //
              _inProgress(false),    //
              _stepNumber(0),        //
              _retryCount(0),        //
              _driver(antennaDriver)
        {
        }

        void AntennaMissionState::Retry(std::uint8_t limit)
        {
            if ((this->_retryCount + 1) == limit)
            {
                NextStep();
            }
            else
            {
                ++this->_retryCount;
            }
        }

        bool AntennaMissionState::IsFinished() const
        {
            return this->_stepNumber >= DeploymentStepLimit;
        }

        bool AntennaMissionState::IsDeploymentPartFinished() const
        {
            return this->_stepNumber >= FinalizationStepIndex;
        }

        void AntennaMissionState::Update(const AntennaDeploymentStatus& status)
        {
            this->_inProgress = status.IsDeploymentActive[0] | //
                status.IsDeploymentActive[1] |                 //
                status.IsDeploymentActive[2] |                 //
                status.IsDeploymentActive[3];
        }

        std::uint8_t AntennaMissionState::StepCount()
        {
            return DeploymentStepLimit;
        }

        std::uint8_t AntennaMissionState::DeploymentStepCount()
        {
            return FinalizationStepIndex;
        }

        /**
         * @brief This procedure is supposed to stop any deployment process that may currently be active on the
         * passed hardware channel.
         *
         * @param[in] driver Reference to current antenna driver instance.
         * @param[in] channel Hardware channel that should be used for current operation.
         * @param[in] retryCount Number of step retry attempts in case of errors.
         * @return True if operation has been successfully completed, false otherwise.
         */
        static bool EndDeployment(AntennaDriver& driver, AntennaChannel channel, std::uint8_t retryCount)
        {
            while (retryCount-- > 0)
            {
                const OSResult status = driver.FinishDeployment(&driver, channel);
                if (OS_RESULT_SUCCEEDED(status))
                {
                    return true;
                }
            }

            return false;
        }

        /**
         * @brief This procedure is supposed to stop any deployment process that may currently be active on the
         * hardware channel used in the previous step.
         *
         * @param[in] state Reference to global satellite state.
         * @param[in] stateDescriptor Reference to the deployment process state object.
         * @param[in] driver Reference to current antenna driver instance.
         */
        static void StopDeployment(const SystemState& state,
            AntennaMissionState& stateDescriptor,
            AntennaDriver& driver //
            )
        {
            UNREFERENCED_PARAMETER(state);
            if (stateDescriptor.StepNumber() == 0)
            {
                return;
            }

            const AntennaDeploymentStep& step = deploymentSteps[stateDescriptor.StepNumber() - 1];
            EndDeployment(driver, step.channel, RetryLimit);
        }

        /**
         * @brief This procedure is supposed to start deployment process described by the current step.
         *
         * @param[in] state Reference to global satellite state.
         * @param[in] stateDescriptor Reference to the deployment process state object.
         * @param[in] driver Reference to current antenna driver instance.
         */
        static void BeginDeployment(const SystemState& state,
            AntennaMissionState& stateDescriptor,
            AntennaDriver& driver //
            )
        {
            UNREFERENCED_PARAMETER(state);
            const AntennaDeploymentStep& step = deploymentSteps[stateDescriptor.StepNumber()];
            std::uint8_t counter = RetryLimit;
            while (counter-- > 0)
            {
                const OSResult result = driver.DeployAntenna(&driver,
                    step.channel,
                    step.antennaId,
                    TimeSpanFromSeconds(step.deploymentTimeout),
                    step.overrideSwitches //
                    );

                if (OS_RESULT_SUCCEEDED(result))
                {
                    stateDescriptor.NextStep();
                    return;
                }
            }

            stateDescriptor.Retry(StepRetryLimit);
        }

        void RegularDeploymentStep(const SystemState& state,
            AntennaMissionState& stateDescriptor,
            AntennaDriver& driver //
            )
        {
            StopDeployment(state, stateDescriptor, driver);
            BeginDeployment(state, stateDescriptor, driver);
        }

        void ResetDriverStep(const SystemState& state,
            AntennaMissionState& stateDescriptor,
            AntennaDriver& driver //
            )
        {
            UNREFERENCED_PARAMETER(state);
            const AntennaDeploymentStep& step = deploymentSteps[stateDescriptor.StepNumber()];

            std::uint8_t counter = RetryLimit;
            while (counter-- > 0)
            {
                const OSResult result = driver.Reset(&driver, step.channel);
                if (OS_RESULT_SUCCEEDED(result))
                {
                    stateDescriptor.NextStep();
                    return;
                }
            }

            stateDescriptor.Retry(StepRetryLimit);
        }

        void FinishDeploymentStep(const SystemState& state,
            AntennaMissionState& stateDescriptor,
            AntennaDriver& driver //
            )
        {
            UNREFERENCED_PARAMETER(state);
            const AntennaDeploymentStep& step = deploymentSteps[stateDescriptor.StepNumber()];
            if (EndDeployment(driver, step.channel, RetryLimit))
            {
                stateDescriptor.NextStep();
            }
            else
            {
                stateDescriptor.Retry(StepRetryLimit);
            }
        }

        /**
         * @brief This procedure is deployment action entry point.
         *
         * This procedure runs the antenna deployment process.
         * @param[in] state Pointer to global satellite state.
         * @param[in] param Pointer to the deployment condition private context. This pointer should point
         * at the object of AntennaMissionState type.
         */
        static void AntennaDeploymentAction(const SystemState& state, void* param)
        {
            AntennaMissionState* stateDescriptor = (AntennaMissionState*)param;
            const AntennaDeploymentStep& step = deploymentSteps[stateDescriptor->StepNumber()];
            DeploymentProcedure* procedure = step.procedure;
            procedure(state, *stateDescriptor, stateDescriptor->Driver());
        }

        /**
         * @brief This procedure checks whether there is currently antenna deployment process in progress.
         * @param[in] deploymentState Current driver deployment state
         * @return True if at least one antenna is being deployed, false otherwise.
         */
        static bool AreAllAntennasDeployed(const AntennaDeploymentStatus& deploymentState)
        {
            return deploymentState.DeploymentStatus[0] & //
                deploymentState.DeploymentStatus[1] &    //
                deploymentState.DeploymentStatus[2] &    //
                deploymentState.DeploymentStatus[3];
        }

        /**
         * @brief Procedure that verifies whether the antenna deployment process should be executed.
         * @param[in] state Pointer to global satellite state.
         * @param[in] param Pointer to the deployment condition private context. This pointer should point
         * at the object of AntennaMissionState type.
         *
         * @return True if the deployment action should be performed, false otherwise.
         */
        static bool AntennaDeploymentCondition(const SystemState& state, void* param)
        {
            AntennaMissionState* stateDescriptor = (AntennaMissionState*)param;
            const TimeSpan t = TimeSpanFromMinutes(40);
            if (TimeSpanLessThan(state.Time, t))
            {
                return false;
            }

            if (stateDescriptor->IsDeploymentInProgress())
            {
                return false;
            }

            return !stateDescriptor->IsFinished();
        }

        /**
         * @brief This procedure is antenna deployment update descriptor entry point.
         *
         * This procedure updates the global satellite state as well as deployemnt process private state.
         * @param[in] state Pointer to global satellite state.
         * @param[in] param Pointer to the deployment condition private context. This pointer should point
         * at the object of AntennaMissionState type.
         * @return Operation status.
         */
        static UpdateResult AntennaDeploymentUpdate(SystemState& state, void* param)
        {
            UNREFERENCED_PARAMETER(state);
            AntennaMissionState* stateDescriptor = (AntennaMissionState*)param;
            if (stateDescriptor->IsFinished())
            {
                return UpdateResult::UpdateOK;
            }

            AntennaDeploymentStatus deploymentStatus;
            AntennaDriver& driver = stateDescriptor->Driver();
            const OSResult result = driver.GetDeploymentStatus(&driver,
                deploymentSteps[stateDescriptor->StepNumber()].channel,
                &deploymentStatus //
                );

            if (OS_RESULT_FAILED(result))
            {
                return UpdateResult::UpdateFailure;
            }

            for (int i = 0; i < 4; ++i)
            {
                state.Antenna.DeploymentState[i] = deploymentStatus.DeploymentStatus[i] || state.Antenna.DeploymentState[i];
            }

            stateDescriptor->Update(deploymentStatus);

            if (                                                //
                !stateDescriptor->IsDeploymentPartFinished() && //
                !stateDescriptor->OverrideState() &&            //
                AreAllAntennasDeployed(deploymentStatus)        //
                )
            {
                stateDescriptor->OverrideStep(FinalizationStepIndex);
            }

            state.Antenna.Deployed = stateDescriptor->IsFinished();
            return UpdateResult::UpdateOK;
        }

        AntennaTask::AntennaTask(AntennaDriver& driver) : state(driver)
        {
        }

        ActionDescriptor<SystemState> AntennaTask::BuildAction()
        {
            ActionDescriptor<SystemState> descriptor;
            descriptor.name = "Deploy Antenna Action";
            descriptor.param = &this->state;
            descriptor.condition = AntennaDeploymentCondition;
            descriptor.actionProc = AntennaDeploymentAction;
            return descriptor;
        }

        UpdateDescriptor<SystemState> AntennaTask::BuildUpdate()
        {
            UpdateDescriptor<SystemState> descriptor;
            descriptor.name = "Deploy Antenna Update";
            descriptor.param = &this->state;
            descriptor.updateProc = AntennaDeploymentUpdate;
            return descriptor;
        }

        /** @}*/
    }
}
