#include <stdbool.h>
#include <FreeRTOS.h>
#include <task.h>

#include "time/TimePoint.h"
#include "base/os.h"
#include "logger/logger.h"
#include "mission.h"
#include "mission/adcs_mission.h"
#include "mission/sail.h"
#include "obc.h"
#include "state/state.h"
#include "system.h"

static TerminalCommand terminalCommand = TerminalCommandNone;

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

    TimeGetCurrentMissionTime(&Main.timeProvider, &state->Time);

    return SystemStateUpdateOK;
}

static void Loop(SystemState* state, ModeDescriptor* mode)
{
    LOG(LOG_LEVEL_INFO, "Updating system state");

    SystemStateUpdateResult updateResult = SystemStateUpdate(state, mode->update, mode->updateCount);

    LOGF(LOG_LEVEL_INFO, "System state update result %d", updateResult);

    SystemStateVerifyResult verifyResult = SystemStateVerify(state, mode->verify, mode->verifyResult, mode->verifyCount);

    LOGF(LOG_LEVEL_INFO, "Verify result %d", verifyResult);

    size_t runnableCount = SystemDetermineActions(state, mode->actions, mode->actionCount, mode->runnableActions);

    LOGF(LOG_LEVEL_INFO, "Executing %d actions", runnableCount);

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

    ModeDescriptor descriptor = {.update = updateDescriptors,
        .updateCount = COUNT_OF(updateDescriptors),
        .verify = NULL,
        .verifyResult = NULL,
        .verifyCount = 0,
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

static SystemStateUpdateResult UpdateCommandTerminal(SystemState* state, void* param)
{
    UNREFERENCED_PARAMETER(param);

    state->RequestedCommand = terminalCommand;
    terminalCommand = TerminalCommandNone;

    return SystemStateUpdateOK;
}

void SetTerminalCommand(TerminalCommand command)
{
    terminalCommand = command;
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

    missionState->TerminalCommandUpdate.Name = "Terminal command";
    missionState->TerminalCommandUpdate.Param = NULL;
    missionState->TerminalCommandUpdate.UpdateProc = UpdateCommandTerminal;

    System.CreateTask(MissionControlTask, "MissionControl", 2048, missionState, tskIDLE_PRIORITY + 2, NULL);
}
