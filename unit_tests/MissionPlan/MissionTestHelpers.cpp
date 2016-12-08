#include "MissionTestHelpers.h"
#include <ostream>
#include "state/state.h"

void showValue(const SystemState& state, std::ostream& os)
{
    os << "time=" << state.Time.value << ", "
       << "antennaDeployed=" << state.Antenna.Deployed;
}
