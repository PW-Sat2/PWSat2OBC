#include <cstdint>
#include <cstring>
#include "antenna/driver.h"
#include "antenna_state.h"
#include "gsl/gsl_util"
#include "state/state.h"
#include "system.h"
#include "time/TimePoint.h"

namespace mission
{
    namespace antenna
    {
        typedef void DeploymentProcedure(const SystemState& state, //
            AntennaMissionState& stateDescriptor,
            AntennaDriver& driver //
            );

        static DeploymentProcedure RegularDeploymentStep, ResetDriverStep, FinishDeploymentStep;

        static constexpr std::uint8_t StepRetryLimit = 3;
        static constexpr std::uint8_t RetryLimit = 3;

        struct AntennaDeploymentStep final
        {
            DeploymentProcedure* procedure;
            AntennaChannel channel;
            AntennaId antennaId;
            uint8_t deploymentTimeout;
            bool overrideSwitches;
        };

        static const AntennaDeploymentStep deploymentSteps[] = {
            {ResetDriverStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA_AUTO_ID, 0, false},
            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA_AUTO_ID, 9, false},

            {ResetDriverStep, ANTENNA_BACKUP_CHANNEL, ANTENNA_AUTO_ID, 0, false},
            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA_AUTO_ID, 9, false},

            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA1_ID, 9, false},
            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA2_ID, 9, false},
            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA3_ID, 9, false},
            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA4_ID, 9, false},

            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA1_ID, 9, false},
            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA2_ID, 9, false},
            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, 9, false},
            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA4_ID, 9, false},

            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA1_ID, 19, false},
            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA2_ID, 19, false},
            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA3_ID, 19, false},
            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA4_ID, 19, false},

            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA1_ID, 19, false},
            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA2_ID, 19, false},
            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, 19, false},
            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA4_ID, 19, false},

            {ResetDriverStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA_AUTO_ID, 0, false},
            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA1_ID, 39, true},
            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA2_ID, 39, true},
            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA3_ID, 39, true},
            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA4_ID, 39, true},

            {ResetDriverStep, ANTENNA_BACKUP_CHANNEL, ANTENNA_AUTO_ID, 0, false},
            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA1_ID, 39, true},
            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA2_ID, 39, true},
            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, 39, true},
            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA4_ID, 39, true},

            {ResetDriverStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA_AUTO_ID, 0, false},
            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA1_ID, 59, true},
            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA2_ID, 59, true},
            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA3_ID, 59, true},
            {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA4_ID, 59, true},

            {ResetDriverStep, ANTENNA_BACKUP_CHANNEL, ANTENNA_AUTO_ID, 0, false},
            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA1_ID, 59, true},
            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA2_ID, 59, true},
            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, 59, true},
            {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA4_ID, 59, true},
            {FinishDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA_AUTO_ID, 0, false},
            {FinishDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA_AUTO_ID, 0, false},
        };

        static constexpr uint8_t DeploymentStepLimit = COUNT_OF(deploymentSteps);
        static constexpr uint8_t FinalizationStepIndex = COUNT_OF(deploymentSteps) - 2;

        AntennaMissionState::AntennaMissionState(AntennaDriver& antennaDriver)
            : overrideState(false), //
              inProgress(false),    //
              stepNumber(0),        //
              retryCount(0),        //
              driver(antennaDriver)
        {
        }

        void AntennaMissionState::Retry(std::uint8_t limit)
        {
            if ((this->retryCount + 1) == limit)
            {
                NextStep();
            }
            else
            {
                ++this->retryCount;
            }
        }

        bool AntennaMissionState::IsFinished() const
        {
            return this->stepNumber >= DeploymentStepLimit;
        }

        bool AntennaMissionState::IsDeploymentPartFinished() const
        {
            return this->stepNumber >= FinalizationStepIndex;
        }

        void AntennaMissionState::Update(const AntennaDeploymentStatus& status)
        {
            this->inProgress = status.IsDeploymentActive[0] | //
                status.IsDeploymentActive[1] |                //
                status.IsDeploymentActive[2] |                //
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

        static bool AntennaDeploymentCondition(const SystemState* state, void* param)
        {
            AntennaMissionState* stateDescriptor = (AntennaMissionState*)param;
            const TimeSpan t = TimeSpanFromMinutes(40);
            if (TimeSpanLessThan(state->Time, t))
            {
                return false;
            }

            if (stateDescriptor->IsDeploymentInProgress())
            {
                return false;
            }

            return !stateDescriptor->IsFinished();
        }

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

        static void AntennaDeploymentAction(const SystemState* state, void* param)
        {
            AntennaMissionState* stateDescriptor = (AntennaMissionState*)param;
            const AntennaDeploymentStep& step = deploymentSteps[stateDescriptor->StepNumber()];
            DeploymentProcedure* procedure = step.procedure;
            procedure(*state, *stateDescriptor, stateDescriptor->Driver());
        }

        /**
         * @brief This procedure checks whether there is currently antenna deployment process in progress.
         * @param[in] deploymentState Current driver deployment state
         * @return Operation status.
         */
        static bool AreAllAntennasDeployed(const AntennaDeploymentStatus& deploymentState)
        {
            return deploymentState.DeploymentStatus[0] & //
                deploymentState.DeploymentStatus[1] &    //
                deploymentState.DeploymentStatus[2] &    //
                deploymentState.DeploymentStatus[3];
        }

        SystemStateUpdateResult AntennaDeploymentUpdate(SystemState* state, void* param)
        {
            UNREFERENCED_PARAMETER(state);
            AntennaMissionState* stateDescriptor = (AntennaMissionState*)param;
            if (stateDescriptor->IsFinished())
            {
                return SystemStateUpdateOK;
            }

            AntennaDeploymentStatus deploymentStatus;
            AntennaDriver& driver = stateDescriptor->Driver();
            const OSResult result = driver.GetDeploymentStatus(&driver,
                deploymentSteps[stateDescriptor->StepNumber()].channel,
                &deploymentStatus //
                );

            if (OS_RESULT_FAILED(result))
            {
                return SystemStateUpdateFailure;
            }

            for (int i = 0; i < 4; ++i)
            {
                state->Antenna.DeploymentState[i] = deploymentStatus.DeploymentStatus[i] || state->Antenna.DeploymentState[i];
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

            state->Antenna.Deployed = stateDescriptor->IsFinished();
            return SystemStateUpdateOK;
        }

        void AntennaInitializeActionDescriptor(AntennaMissionState* stateDescriptor,
            SystemActionDescriptor* descriptor //
            )
        {
            descriptor->Name = "Deploy Antenna Action";
            descriptor->Param = stateDescriptor;
            descriptor->Condition = AntennaDeploymentCondition;
            descriptor->ActionProc = AntennaDeploymentAction;
        }

        void AntennaInitializeUpdateDescriptor(AntennaMissionState* stateDescriptor,
            SystemStateUpdateDescriptor* descriptor //
            )
        {
            descriptor->Name = "Deploy Antenna Update";
            descriptor->Param = stateDescriptor;
            descriptor->UpdateProc = AntennaDeploymentUpdate;
        }
    }
}
