#include "mission.h"
#include "base/os.h"
#include "logger/logger.h"
#include "mission/adcs_mission.h"
#include "mission/sail.h"
#include "obc.h"
#include "state/state.h"
#include "system.h"
#include "time/TimePoint.h"

/**
 * @brief Object describing which descriptors are available in specific mode
 */
typedef struct
{
    /** @brief Array of update descriptors */
    SystemStateUpdateDescriptor* update;
    /** @brief Count of update descriptors */
    size_t updateCount;
    /** @brief Array of verify descriptors */
    SystemStateVerifyDescriptor* verify;
    /** @brief Array of verify results (with the same length as @see verify) */
    SystemStateVerifyDescriptorResult* verifyResult;
    /** @brief Count of verify descriptors/results */
    size_t verifyCount;
    /** @brief Array of action descriptors */
    SystemActionDescriptor* actions;
    /** @brief Count of action descriptors */
    size_t actionCount;
    /** @brief Buffer for pointers to action descriptors that can be executed */
    SystemActionDescriptor** runnableActions;
} ModeDescriptor;

static SystemStateUpdateResult UpdateTime(SystemState* state, void* param)
{
    UNREFERENCED_PARAMETER(param);

    TimeGetCurrentTime(&Main.timeProvider, &state->Time);

    return SystemStateUpdateOK;
}

static void Loop(SystemState* state, ModeDescriptor* mode)
{
    LOG(LOG_LEVEL_TRACE, "Updating system state");

    SystemStateUpdateResult updateResult = SystemStateUpdate(state, mode->update, mode->updateCount);

    LOGF(LOG_LEVEL_TRACE, "System state update result %d", updateResult);

    SystemStateVerifyResult verifyResult = SystemStateVerify(state, mode->verify, mode->verifyResult, mode->verifyCount);

    LOGF(LOG_LEVEL_TRACE, "Verify result %d", verifyResult);

    size_t runnableCount = SystemDetermineActions(state, mode->actions, mode->actionCount, mode->runnableActions);

    LOGF(LOG_LEVEL_TRACE, "Executing %d actions", runnableCount);

    SystemDispatchActions(state, mode->runnableActions, runnableCount);
}

static void NormalModeLoop(SystemState* state, MissionState* missionState)
{
    SystemStateUpdateDescriptor updateDescriptors[] = {
        missionState->TerminalCommandUpdate, //
        missionState->UpdateTime,            //
        missionState->Sail.Update,           //
        missionState->ADCS.Update            //
    };

    SystemActionDescriptor actionDescriptors[] = {
        missionState->Sail.OpenSail, //
        missionState->ADCS.TurnOff,  //
        missionState->ADCS.Detumble, //
        missionState->ADCS.SunPoint, //
    };

    SystemActionDescriptor* runnableActions[COUNT_OF(actionDescriptors)];

    ModeDescriptor descriptor;
    descriptor.update = updateDescriptors;
    descriptor.updateCount = COUNT_OF(updateDescriptors);
    descriptor.verify = NULL;
    descriptor.verifyResult = NULL;
    descriptor.verifyCount = 0;
    descriptor.actions = actionDescriptors;
    descriptor.actionCount = COUNT_OF(actionDescriptors);
    descriptor.runnableActions = runnableActions;

    Loop(state, &descriptor);
}

static void MissionControlTask(void* param)
{
    auto missionState = static_cast<MissionState*>(param);

    LOG(LOG_LEVEL_DEBUG, "Starting mission control task");
    SystemState state;
    SystemStateEmpty(&state);

    while (1)
    {
        NormalModeLoop(&state, missionState);

        TimeLongDelay(&Main.timeProvider, TimeSpanFromSeconds(10));
    }
}

void InitializeMission(MissionState* missionState, OBC* obc)
{
    UNREFERENCED_PARAMETER(obc);

    SailInitializeUpdateDescriptor(&missionState->Sail.Update, &missionState->SailOpened);
    SailInitializeActionDescriptor(&missionState->Sail.OpenSail, &missionState->SailOpened);
    ADCSInitializeDescriptors(&obc->adcs, &missionState->ADCS);

    missionState->UpdateTime.Name = "Update time";
    missionState->UpdateTime.Param = NULL;
    missionState->UpdateTime.UpdateProc = UpdateTime;

    System::CreateTask(MissionControlTask, "MissionControl", 2048, missionState, TaskPriority::P2, nullptr);
}
