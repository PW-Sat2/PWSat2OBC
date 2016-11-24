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
        class AntennaMissionState final
        {
          public:
            AntennaMissionState(AntennaDriver& antennaDriver);

            bool IsDeploymentInProgress() const;

            bool IsDeploymentPartFinished() const;

            bool IsFinished() const;

            bool OverrideState() const;

            std::uint8_t StepNumber() const;

            void NextStep();

            void Retry(std::uint8_t limit);

            AntennaDriver& Driver();

            void OverrideDeploymentState();

            void OverrideStep(std::uint8_t stepNumber);

            void Update(const AntennaDeploymentStatus& status);

            void Restart();

            static std::uint8_t StepCount();

            static std::uint8_t DeploymentStepCount();

          private:
            bool overrideState;
            bool inProgress;
            uint8_t stepNumber;
            uint8_t retryCount;
            AntennaDriver& driver;
        };

        inline bool AntennaMissionState::IsDeploymentInProgress() const
        {
            return this->inProgress;
        }

        inline bool AntennaMissionState::OverrideState() const
        {
            return this->overrideState;
        }

        inline std::uint8_t AntennaMissionState::StepNumber() const
        {
            return this->stepNumber;
        }

        inline void AntennaMissionState::NextStep()
        {
            ++this->stepNumber;
        }

        inline AntennaDriver& AntennaMissionState::Driver()
        {
            return this->driver;
        }

        inline void AntennaMissionState::OverrideDeploymentState()
        {
            this->overrideState = true;
        }

        inline void AntennaMissionState::OverrideStep(std::uint8_t stepNumber)
        {
            this->stepNumber = stepNumber;
            this->retryCount = 0;
        }

        inline void AntennaMissionState::Restart()
        {
            OverrideStep(0);
        }

        void AntennaInitializeActionDescriptor(AntennaMissionState* stateDescriptor,
            SystemActionDescriptor* missionDescriptor //
            );

        void AntennaInitializeUpdateDescriptor(AntennaMissionState* stateDescriptor,
            SystemStateUpdateDescriptor* descriptor //
            );
    }
}

#endif /* LIBS_MISSION_INCLUDE_MISSION_ANTENNA_H_ */
