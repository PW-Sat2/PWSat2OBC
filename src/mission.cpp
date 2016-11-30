#include "mission.h"
#include <array>
#include "base/os.h"
#include "logger/logger.h"
#include "mission/adcs_mission.h"
#include "mission/sail.h"
#include "obc.h"
#include "state/state.h"
#include "system.h"
#include "time/TimePoint.h"

/**
 * @addtogroup mission
 * @{
 */
MissionState::MissionState(OBC& obc) : antennaMission(obc.antennaDriver)
{
}

/**
 * @brief Object describing which descriptors are available in specific mode
 */
struct ModeDescriptor
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
};

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

static SystemStateUpdateDescriptor NormalModeUpdateDescriptors[3] = {};

static SystemActionDescriptor NormalModeActionDescriptors[2] = {};

static void NormalModeLoop(SystemState* state, MissionState* missionState)
{
    UNREFERENCED_PARAMETER(missionState);
    std::array<SystemActionDescriptor*, count_of(NormalModeActionDescriptors)> runnableActions;

    ModeDescriptor descriptor;
    descriptor.update = NormalModeUpdateDescriptors;
    descriptor.updateCount = COUNT_OF(NormalModeUpdateDescriptors);
    descriptor.verify = NULL;
    descriptor.verifyResult = NULL;
    descriptor.verifyCount = 0;
    descriptor.actions = NormalModeActionDescriptors;
    descriptor.actionCount = COUNT_OF(NormalModeActionDescriptors);
    descriptor.runnableActions = runnableActions.data();

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
        TimeLongDelay(&Main.timeProvider, TimeSpanFromSeconds(10));

        NormalModeLoop(&state, missionState);
    }
}
static void TimeInitializeUpdateDescriptor(SystemStateUpdateDescriptor* descriptor)
{
    descriptor->Name = "Update time";
    descriptor->Param = NULL;
    descriptor->UpdateProc = UpdateTime;
}

void InitializeMission(MissionState* missionState, OBC* obc)
{
    UNREFERENCED_PARAMETER(obc);
    // Update descriptors
    NormalModeUpdateDescriptors[0] = AntennaInitializeUpdateDescriptor(missionState->antennaMission);
    TimeInitializeUpdateDescriptor(&NormalModeUpdateDescriptors[1]);
    SailInitializeUpdateDescriptor(&NormalModeUpdateDescriptors[2], &missionState->SailOpened);

    // Action Descriptors
    NormalModeActionDescriptors[0] = AntennaInitializeActionDescriptor(missionState->antennaMission);
    SailInitializeActionDescriptor(&NormalModeActionDescriptors[1], &missionState->SailOpened);

    //    ADCSInitializeDescriptors(&obc->adcs, &missionState->ADCS);
    System.CreateTask(MissionControlTask, "MissionControl", 2048, missionState, 2, NULL);
}

/** @} */
