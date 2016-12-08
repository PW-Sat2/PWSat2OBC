#include "adcs/adcs.h"
#include "adcs_mission.h"
#include "state/struct.h"
#include "system.h"

static SystemStateUpdateResult ADCSUpdate(SystemState* state, void* param)
{
    ADCSContext* adcs = (ADCSContext*)param;

    state->ADCS.CurrentMode = adcs->CurrentMode;

    return SystemStateUpdateOK;
}

static bool TurnOffCondition(const SystemState* state, void* param)
{
    UNREFERENCED_PARAMETER(state);
    UNREFERENCED_PARAMETER(param);
    return false; // HAS_FLAG(state->RequestedCommand, TerminalCommandADCSTurnOff);
}

static void TurnOff(const SystemState* state, void* param)
{
    UNREFERENCED_PARAMETER(state);
    ADCSContext* adcs = (ADCSContext*)param;
    adcs->Command(adcs, ADCSCommandTurnOff);
}

static bool DetumbleCondition(const SystemState* state, void* param)
{
    UNREFERENCED_PARAMETER(state);
    UNREFERENCED_PARAMETER(param);
    return false; // HAS_FLAG(state->RequestedCommand, TerminalCommandADCSDetumble);
}

static void Detumble(const SystemState* state, void* param)
{
    UNREFERENCED_PARAMETER(state);
    ADCSContext* adcs = (ADCSContext*)param;
    adcs->Command(adcs, ADCSCommandDetumble);
}

static bool SunPointCondition(const SystemState* state, void* param)
{
    UNREFERENCED_PARAMETER(state);
    UNREFERENCED_PARAMETER(param);
    return false; // HAS_FLAG(state->RequestedCommand, TerminalCommandADCSSunPoint);
}

static void SunPoint(const SystemState* state, void* param)
{
    UNREFERENCED_PARAMETER(state);
    ADCSContext* adcs = (ADCSContext*)param;
    adcs->Command(adcs, ADCSCommandSunPoint);
}

void ADCSInitializeDescriptors(ADCSContext* adcs, ADCSDescriptors* descriptors)
{
    descriptors->Update.Name = "ADCS Update";
    descriptors->Update.Param = adcs;
    descriptors->Update.UpdateProc = ADCSUpdate;

    descriptors->TurnOff.Name = "ADCS Turn Off";
    descriptors->TurnOff.Param = adcs;
    descriptors->TurnOff.Condition = TurnOffCondition;
    descriptors->TurnOff.ActionProc = TurnOff;

    descriptors->Detumble.Name = "ADCS Detumble";
    descriptors->Detumble.Param = adcs;
    descriptors->Detumble.Condition = DetumbleCondition;
    descriptors->Detumble.ActionProc = Detumble;

    descriptors->SunPoint.Name = "ADCS SunPoint";
    descriptors->SunPoint.Param = adcs;
    descriptors->SunPoint.Condition = SunPointCondition;
    descriptors->SunPoint.ActionProc = SunPoint;
}
