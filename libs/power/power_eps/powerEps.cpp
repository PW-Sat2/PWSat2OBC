#include <stdlib.h>

#include "eps/eps.h"
#include "power/power.h"
#include "power_eps.h"
#include "system.h"

static void TriggerSystemPowerCycle(PowerControl* powerControl)
{
    UNREFERENCED_PARAMETER(powerControl);

    EpsTriggerSystemPowerCycle();
}

void EPSPowerControlInitialize(PowerControl* powerControl)
{
    powerControl->Extra = NULL;
    powerControl->TriggerSystemPowerCycle = TriggerSystemPowerCycle;
}
