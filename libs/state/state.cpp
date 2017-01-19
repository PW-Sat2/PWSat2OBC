#include <cstring>
#include "struct.h"

using namespace std::chrono_literals;

SystemState::SystemState(adcs::IAdcsCoordinator& adcsAccess) //
    : Time(0ms),
      SailOpened(false),
      adcs(adcsAccess)
{
    Antenna.Deployed = false;
    memset(&Antenna.DeploymentState, 0, sizeof(Antenna.DeploymentState));
}
