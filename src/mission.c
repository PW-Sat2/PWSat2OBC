#include <stdbool.h>
#include <FreeRTOS.h>
#include <task.h>

#include "time/TimePoint.h"
#include "base/os.h"
#include "logger/logger.h"
#include "mission.h"
#include "mission/sail.h"
#include "obc.h"
#include "state/state.h"
#include "system.h"

typedef struct
{
    SystemStateUpdateDescriptor* update;
    size_t updateCount;
    SystemStateVerifyDescriptor* verify;
    SystemStateVerifyDescriptorResult* verifyResult;
    size_t verifyCount;
    SystemActionDescriptor* actions;
    size_t actionCount;
    SystemActionDescriptor** runnableActions;
} ModeDescriptor;

static SystemStateUpdateResult UpdateTime(SystemState* state, void* param)
{
    UNREFERENCED_PARAMETER(param);

    TimeGetCurrentMissionTime(&Main.timeProvider, &state->Time);

    return SystemStateUpdateOK;
}

static void Loop(SystemState* state, ModeDescriptor* mode)
{
    LOG(LOG_LEVEL_INFO, "Mission control loop");

    LOG(LOG_LEVEL_INFO, "Updating system state");

    SystemStateUpdateResult updateResult = SystemStateUpdate(state, mode->update, mode->updateCount);

    LOGF(LOG_LEVEL_INFO, "System state update result %d", updateResult);

    LOG(LOG_LEVEL_TRACE, "Verifing system state");

    SystemStateVerifyResult verifyResult = SystemStateVerify(state, mode->verify, mode->verifyResult, mode->verifyCount);

    LOGF(LOG_LEVEL_INFO, "Verify result %d", verifyResult);

    LOG(LOG_LEVEL_INFO, "Determining actions");

    size_t runnableCount = SystemDetermineActions(state, mode->actions, mode->actionCount, mode->runnableActions);

    LOGF(LOG_LEVEL_INFO, "Executing %d actions", runnableCount);

    SystemDispatchActions(state, mode->runnableActions, runnableCount);
}

static void NormalModeLoop(SystemState* state, MissionState* missionState)
{
    SystemStateUpdateDescriptor updateDescriptors[] = {missionState->UpdateTime, missionState->Sail.Update};

    SystemStateVerifyDescriptor verifyDescriptors[0];

    SystemStateVerifyDescriptorResult verifyResults[COUNT_OF(verifyDescriptors)];

    SystemActionDescriptor actionDescriptors[] = {missionState->Sail.OpenSail};

    SystemActionDescriptor* runnableActions[COUNT_OF(actionDescriptors)];

    ModeDescriptor descriptor = {.update = updateDescriptors,
        .updateCount = COUNT_OF(updateDescriptors),
        .verify = verifyDescriptors,
        .verifyResult = verifyResults,
        .verifyCount = COUNT_OF(verifyDescriptors),
        .actions = actionDescriptors,
        .actionCount = COUNT_OF(actionDescriptors),
        .runnableActions = runnableActions};

    Loop(state, &descriptor);
}

static void MissionControlTask(void* param)
{
    MissionState* missionState = (MissionState*)param;

    LOG(LOG_LEVEL_DEBUG, "Starting mission control task");
    SystemState state;
    SystemStateEmpty(&state);

    while (1)
    {
        NormalModeLoop(&state, missionState);

        System.SleepTask(10000);
    }
}

void InitializeMission(MissionState* missionState, OBC* obc)
{
    UNREFERENCED_PARAMETER(obc);

    SailInitializeUpdateDescriptor(&missionState->Sail.Update, &missionState->SailOpened);
    SailInitializeActionDescriptor(&missionState->Sail.OpenSail, &missionState->SailOpened);

    missionState->UpdateTime.Name = "Update time";
    missionState->UpdateTime.Param = NULL;
    missionState->UpdateTime.UpdateProc = UpdateTime;

    System.CreateTask(MissionControlTask, "MissionControl", 2048, missionState, tskIDLE_PRIORITY + 2, NULL);
}
