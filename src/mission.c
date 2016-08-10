#include <stdbool.h>
#include <FreeRTOS.h>
#include <task.h>

#include "base/os.h"
#include "logger/logger.h"
#include "mission.h"
#include "mission/sail.h"
#include "mission/state.h"
#include "obc.h"
#include "obc_time.h"
#include "system.h"

static SystemStateUpdateResult UpdateTime(SystemState* state, void* param)
{
    UNREFERENCED_PARAMETER(param);

    state->Time = CurrentTime();

    return SystemStateUpdateOK;
}

static void MissionControlTask(void* param)
{
    MissionState* missionState = (MissionState*)param;

    LOG(LOG_LEVEL_DEBUG, "Starting mission control task");
    SystemState state;
    SystemStateEmpty(&state);

    SystemStateUpdateDescriptor updateDescriptors[] = {missionState->UpdateTime, missionState->Sail.Update};

    SystemStateVerifyDescriptor verifyDescriptors[0];

    SystemActionDescriptor* actionDescriptors[] = {&missionState->Sail.OpenSail};

    while (1)
    {
        LOG(LOG_LEVEL_INFO, "Mission control loop");

        LOG(LOG_LEVEL_INFO, "Updating system state");

        SystemStateUpdateResult updateResult = SystemStateUpdate(&state, updateDescriptors, COUNT_OF(updateDescriptors));

        LOGF(LOG_LEVEL_INFO, "System state update result %d", updateResult);

        LOG(LOG_LEVEL_TRACE, "Verifing system state");

        SystemStateVerifyDescriptorResult verifyResults[COUNT_OF(verifyDescriptors)];

        SystemStateVerifyResult verifyResult = SystemStateVerify(&state, verifyDescriptors, verifyResults, COUNT_OF(verifyDescriptors));

        LOGF(LOG_LEVEL_INFO, "Verify result %d", verifyResult);

        LOG(LOG_LEVEL_INFO, "Determining actions");

        SystemDetermineActions(&state, actionDescriptors, COUNT_OF(actionDescriptors));

        LOG(LOG_LEVEL_INFO, "Executing actions");

        for (size_t i = 0; i < COUNT_OF(actionDescriptors); i++)
        {
            if (actionDescriptors[i]->Runnable)
            {
                LOGF(LOG_LEVEL_INFO, "Running action %s", actionDescriptors[i]->Name);
                actionDescriptors[i]->ActionProc(&state, actionDescriptors[i]->Param);
            }
        }

        System.SleepTask(10000);
    }
}

void InitializeMission(MissionState* missionState, OBC* obc)
{
    SailInitializeUpdateDescriptor(&missionState->Sail.Update, &missionState->SailOpened);
    SailInitializeActionDescriptor(&missionState->Sail.OpenSail, &missionState->SailOpened);

    missionState->UpdateTime.Name = "Update time";
    missionState->UpdateTime.Param = NULL;
    missionState->UpdateTime.UpdateProc = UpdateTime;

    System.CreateTask(MissionControlTask, "MissionControl", 2048, missionState, tskIDLE_PRIORITY + 2, NULL);
}
