#include "MissionTestHelpers.h"
#include <ostream>
#include "state/struct.h"

void showValue(const SystemState& state, std::ostream& os)
{
    os << "time=" << state.Time.count() << ", "
       << "antennaDeployed=" << state.Antenna.Deployed;
}
