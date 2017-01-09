#include <cstring>
#include "struct.h"

SystemState::SystemState() //
    : Time(std::chrono::milliseconds(0)),
      SailOpened(false)
{
    Antenna.Deployed = false;
    memset(&Antenna.DeploymentState, 0, sizeof(Antenna.DeploymentState));
}
